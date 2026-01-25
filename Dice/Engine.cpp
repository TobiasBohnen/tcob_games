// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Engine.hpp"

#include "Die.hpp"
#include "Game.hpp"
#include "Socket.hpp"

using namespace scripting;

engine::engine(init const& init)
    : _init {init}
    , _hudProxy {init.State.HUD, PALETTE[0]}
    , _fgProxy {init.State.Foreground, colors::Transparent}
    , _bgProxy {init.State.Background, PALETTE[0]}
    , _texProxy {init.State.Sprites, colors::Transparent}
{
    _script.open_libraries(library::Table, library::String, library::Math);
    _script.Warning.connect([](script::warning_event const& ev) {
        logger::Warning(ev.Message);
    });

    create_env();
    create_wrappers();

    _init.Events.Collision.connect([this](auto const& ev) {
        if (_gameStatus != game_status::Running) { return; }
        call(_callbacks.OnCollision, ev.A, ev.B);
    });

    _init.Events.SocketDieInserted.connect([this]([[maybe_unused]] socket const* socket) {
        if (_gameStatus == game_status::Waiting) {
            _updateHUD = true;
        }
    });
    _init.Events.SocketDieRemoved.connect([this]([[maybe_unused]] socket const* socket) {
        if (_gameStatus == game_status::Waiting) {
            _updateHUD = true;
        }
    });
    _init.Events.DieMotion.connect([this]([[maybe_unused]] die const* die) {
        if (_gameStatus == game_status::Waiting) {
            _updateHUD = true;
        }
    });

    _init.Events.StartTurn.connect([this]() { start_turn(); });
}

template <typename R>
inline auto engine::call(callback<R> const& func, auto&&... args) -> R
{
    if (func) {
        auto& fn {*func};
        auto  result {fn.protected_call(_table, this, args...)};
        if (result) { return result.value(); }

        logger::Error("error calling function");
    }

    return R {};
}

void engine::run(string const& file)
{
    auto env {**_script.Environment};

    // require
    auto const path {io::get_parent_folder(file)};

    auto func {make_unique_closure(std::function {[this, path, &env](char const* module) {
        if (env.has("package", "loaded", module)) { return env["package"]["loaded"][module].as<table>(); }

        if (auto pkg {_script.run_file<table>(std::format("{}/{}.lua", path, module))}) {
            auto& val {pkg.value()};
            env["package"]["loaded"][module] = val;
            return val;
        }

        _script.view().error("module %s not found", module);
        return _script.create_table();
    }})};
    env["require"] = func.get();
    _funcs.push_back(std::move(func));

    _table = *_script.run_file<table>(file);

    _table.try_get(_callbacks.OnSetup, "on_setup");
    _table.try_get(_callbacks.OnTurnUpdate, "on_turn_update");
    _table.try_get(_callbacks.OnTurnFinish, "on_turn_finish");
    _table.try_get(_callbacks.OnTurnStart, "on_turn_start");
    _table.try_get(_callbacks.OnDrawHUD, "on_draw_hud");
    _table.try_get(_callbacks.OnCollision, "on_collision");

    call(_callbacks.OnSetup);
}

auto engine::update(milliseconds deltaTime) -> bool
{
    update_hud();

    _init.State.CanStart = _gameStatus == game_status::Waiting;

    if (_gameStatus != game_status::Running) { return false; }

    for (auto& channel : _soundChannels) {
        channel.update();
    }

    auto const dt {deltaTime.count()};
    _turnTime += dt;
    game_status const status {static_cast<game_status>(call(_callbacks.OnTurnUpdate, dt, _turnTime))};
    _gameStatus = status;

    switch (status) {
    case Running: return true;
    case Waiting: {
        call(_callbacks.OnTurnFinish);
        _init.Game->reset_sockets();
        _turnTime = 0;
        return false;
    }
    case GameOver:
        _updateHUD = true;
        return false;
    }

    return true;
}

auto engine::start_turn() -> bool
{
    if (_gameStatus != game_status::Waiting) { return false; }

    call(_callbacks.OnTurnStart);
    _gameStatus = game_status::Running;
    return true;
}

void engine::create_env()
{
    auto& global {_script.global_table()};

    // create environment
    table env {_script.create_table()};
    env["table"]              = global["table"];
    env["string"]             = global["string"];
    env["math"]               = global["math"];
    env["math"]["random"]     = nullptr;
    env["math"]["randomseed"] = nullptr;
    env["assert"]             = global["assert"];
    env["pairs"]              = global["pairs"];
    env["ipairs"]             = global["ipairs"];
    env["print"]              = global["print"];
    env["type"]               = global["type"];
    env["tonumber"]           = global["tonumber"];
    env["tostring"]           = global["tostring"];

    for (auto const& color : PALETTE_MAP) {
        env["Palette"][color.first] = color.second;
    }

    env["ScreenSize"] = VIRTUAL_SCREEN_SIZE;

    env["GameStatus"]["Running"]  = static_cast<i32>(game_status::Running);
    env["GameStatus"]["Waiting"]  = static_cast<i32>(game_status::Waiting);
    env["GameStatus"]["GameOver"] = static_cast<i32>(game_status::GameOver);

    env["SocketState"]["Idle"]   = static_cast<i32>(socket_state::Idle);
    env["SocketState"]["Accept"] = static_cast<i32>(socket_state::Accept);
    env["SocketState"]["Reject"] = static_cast<i32>(socket_state::Reject);
    env["SocketState"]["Hover"]  = static_cast<i32>(socket_state::Hover);

    env["Rot"]["R0"]   = 0;
    env["Rot"]["R90"]  = 90;
    env["Rot"]["R180"] = 180;
    env["Rot"]["R270"] = 270;

    _script.Environment = env;
}

void engine::create_wrappers()
{
    create_sprite_wrapper();
    create_socket_wrapper();
    create_engine_wrapper();
    create_tex_wrapper();
}

void engine::create_sprite_wrapper()
{
    auto env {**_script.Environment};

    auto newFunc {make_unique_closure(std::function {
        [this](table const& spriteOwner) -> sprite* {
            sprite_def const def {spriteOwner["spriteInit"].get<sprite_def>().value_or(sprite_def {})};

            texture*   tex {nullptr};
            auto const texID {def.TexID};
            if (texID && _textures.contains(*texID)) { tex = &_textures[*texID]; }

            return _init.Game->add_sprite({.Def = def, .Texture = tex, .Owner = spriteOwner});
        }})};
    env["sprite"]["new"] = newFunc.get();
    _funcs.push_back(std::move(newFunc));

    auto sortFunc {make_unique_closure(std::function {[this]() { _init.Events.YSort(); }})};
    env["sprite"]["y_sort"] = sortFunc.get();
    _funcs.push_back(std::move(sortFunc));

    auto& spriteWrapper {*_script.create_wrapper<sprite>("sprite")};
    spriteWrapper["position"] = property {
        [](sprite* sprite) -> point_f { return sprite->Bounds.Position; },
        [](sprite* sprite, point_f p) { sprite->set_bounds(p, sprite->Bounds.Size); }};
    spriteWrapper["size"] = getter {
        [](sprite* sprite) -> size_f { return sprite->Bounds.Size; }};
    spriteWrapper["center"] = getter {
        [](sprite* sprite) -> point_f { return sprite->Bounds.center(); }};
    spriteWrapper["owner"] = getter {
        [](sprite* sprite) -> table const& { return sprite->owner(); }};
    spriteWrapper["texture"] = property {
        [](sprite* sprite) -> u32 { return sprite->get_texture()->ID; },
        [this](sprite* sprite, u32 texID) {
            if (!_textures.contains(texID)) {
                logger::Error("Invalid texture id: {}", texID);
                return;
            }
            sprite->set_texture(&_textures[texID]);
        }};
    spriteWrapper["remove"] = [this](sprite* sprite) { _init.Game->remove_sprite(sprite); };
}

void engine::create_socket_wrapper()
{
    auto env {**_script.Environment};

    auto func {make_unique_closure(std::function {
        [this](table const& socketInit) -> socket* { return _init.Game->add_socket(socketInit.get<socket_face>().value_or(socket_face {})); }})};
    env["socket"]["new"] = func.get();
    _funcs.push_back(std::move(func));
    static auto const convert_sockets {[](std::unordered_map<std::variant<i32, string>, socket*> const& socketMap) {
        std::vector<socket*> sockets;
        sockets.reserve(socketMap.size());
        for (auto const& key : socketMap) { sockets.push_back(key.second); }
        return sockets;
    }};
    env["socket"]["get_hand"] = +[](std::unordered_map<std::variant<i32, string>, socket*> const& sockets) -> hand {
        return get_hand(convert_sockets(sockets));
    };
    env["socket"]["get_value"] = +[](std::unordered_map<std::variant<i32, string>, socket*> const& sockets) -> std::pair<u32, u32> {
        auto socketVec {convert_sockets(sockets)};

        auto const sum {get_sum(socketVec)};

        i32 hand {0};
        switch (get_hand(socketVec).Value) {
        case value_category::None:         hand = 1; break;
        case value_category::OnePair:      hand = 2; break;
        case value_category::TwoPair:      hand = 3; break;
        case value_category::ThreeOfAKind: hand = 4; break;
        case value_category::Straight:     hand = 5; break;
        case value_category::FullHouse:    hand = 6; break;
        case value_category::FourOfAKind:  hand = 8; break;
        case value_category::FiveOfAKind:  hand = 10; break;
        }

        return {sum, hand};
    };

    auto& socketWrapper {*_script.create_wrapper<socket>("socket")};
    socketWrapper["is_empty"] = getter {
        [](socket* socket) -> bool { return socket->is_empty(); }};
    socketWrapper["state"] = getter {
        [](socket* socket) -> u8 { return static_cast<u8>(socket->state()); }};
    socketWrapper["die_value"] = getter {
        [](socket* socket) -> u8 { return socket->is_empty() ? 0 : socket->current_die()->current_face().Value; }};
    socketWrapper["die_color"] = getter {
        [](socket* socket) -> std::optional<u8> { return socket->is_empty() ? std::nullopt : std::optional<u8> {socket->current_die()->current_face().Color}; }};
    socketWrapper["position"] = property {
        [](socket* socket) -> point_i { return socket->HUDPosition; },
        [this](socket* socket, point_f pos) {
            socket->HUDPosition = point_i {pos};
            auto const&   rect {_init.State.HUDBounds};
            point_f const spos {rect.left() + (rect.width() * (pos.X / static_cast<f32>(HUD_SIZE.Width))),
                                rect.top() + (rect.height() * (pos.Y / static_cast<f32>(HUD_SIZE.Height)))};
            socket->move_to(spos);
        }};
    socketWrapper["remove"] = [this](socket* socket) { _init.Game->remove_socket(socket); };
}

void engine::create_engine_wrapper()
{
    auto& engineWrapper {*_script.create_wrapper<engine>("engine")};
    // gfx
    engineWrapper["define_texture"] = [](engine* engine, u32 id, rect_f const& uv) { engine->define_texture(id, rect_i {uv}); };

    // sfx
    engineWrapper["define_sound"] = [](engine* engine, u32 id, audio::sound_wave soundWave) {
        engine->_soundBank.emplace(id, audio::sound_generator {}.create_buffer(soundWave));
    };
    engineWrapper["play_sound"] = [](engine* engine, u32 soundID, std::optional<u8> channelID, std::optional<bool> now) {
        if (channelID >= engine->_soundChannels.size()) {
            logger::Error("Invalid channel id: {}", *channelID);
            return;
        }

        auto     sound {std::make_unique<audio::sound>(engine->_soundBank[soundID])};
        u8 const channel {channelID.value_or(0)};
        if (now.value_or(true)) {
            engine->_soundChannels[channel].play_now(std::move(sound));
        } else {
            engine->_soundChannels[channel].play_queued(std::move(sound));
        }
    };

    // functions
    engineWrapper["rnd"]  = [](engine* engine, f32 min, f32 max) { return engine->_init.State.Rng(min, max); };
    engineWrapper["irnd"] = [](engine* engine, i32 min, i32 max) { return engine->_init.State.Rng(min, max); };
    engineWrapper["log"]  = [](engine*, string const& str) { logger::Info(str); };

    engineWrapper["give_score"] = [](engine* engine, i32 score) { engine->_init.State.Score += score; };
    engineWrapper["update_hud"] = [](engine* engine) { engine->_updateHUD = true; };

    // properties
    engineWrapper["fg"]  = getter {[](engine* engine) -> tex_proxy* { return &engine->_fgProxy; }};
    engineWrapper["bg"]  = getter {[](engine* engine) -> tex_proxy* { return &engine->_bgProxy; }};
    engineWrapper["spr"] = getter {[](engine* engine) -> tex_proxy* { return &engine->_texProxy; }};
    engineWrapper["ssd"] = property {
        [](engine* engine) -> string { return *engine->_init.State.SSD; },
        [](engine* engine, string const& val) { engine->_init.State.SSD = val; }};
    engineWrapper["is_game_over"] = getter {[](engine* engine) -> bool { return engine->_gameStatus == game_status::GameOver; }};
}

void engine::create_tex_wrapper()
{
    auto& texWrapper {*_script.create_wrapper<tex_proxy>("tex")};
    texWrapper["bounds"] = getter {[](tex_proxy* tex) -> rect_i { return tex->bounds(); }};
    texWrapper["size"]   = getter {[](tex_proxy* tex) -> size_i { return tex->bounds().Size; }};

    texWrapper["clear"] = [](tex_proxy* tex, std::optional<rect_f> const& rect) {
        std::optional<rect_i> r {std::nullopt};
        if (rect) { r = rect_i {*rect}; }
        tex->clear(r);
    };

    texWrapper["pixel"]  = [](tex_proxy* tex, point_f pos, u8 color) { tex->pixel(point_i {pos}, color); };
    texWrapper["line"]   = [](tex_proxy* tex, point_f start, point_f end, u8 color) { tex->line(point_i {start}, point_i {end}, color); };
    texWrapper["circle"] = [](tex_proxy* tex, point_f center, f32 radius, u8 color, bool fill) { tex->circle(point_i {center}, static_cast<i32>(radius), color, fill); };
    texWrapper["rect"]   = [](tex_proxy* tex, rect_f const& rect, u8 color, bool fill) { tex->rect(rect_i {rect}, color, fill); };

    texWrapper["blit"] = [](tex_proxy* tex, rect_f const& rect, string const& dotStr, std::optional<blit_settings> settings) {
        tex->blit(rect_i {rect}, dotStr, settings ? *settings : blit_settings {});
    };
    texWrapper["print"] = [](tex_proxy* tex, point_f pos, string_view text, u8 color, std::optional<font_type> fontType) {
        tex->print(point_i {pos}, text, color, fontType ? *fontType : font_type::Font5x5);
    };
    texWrapper["socket"] = [](tex_proxy* tex, socket* socket) { tex->socket(socket); };
}

void engine::define_texture(u32 id, rect_i const& uv)
{
    auto& tex {_textures[id]};
    tex.ID     = id;
    tex.Size   = size_f {uv.Size};
    tex.Region = std::to_string(id);
    tex.Alpha  = grid<u8> {uv.Size};

    _init.Game->get_sprite_texture()->regions()[tex.Region] =
        {.UVRect = {static_cast<f32>(uv.left()) / static_cast<f32>(SPRITE_TEXTURE_SIZE.Width),
                    static_cast<f32>(uv.top()) / static_cast<f32>(SPRITE_TEXTURE_SIZE.Height),
                    static_cast<f32>(uv.width()) / static_cast<f32>(SPRITE_TEXTURE_SIZE.Width),
                    static_cast<f32>(uv.height()) / static_cast<f32>(SPRITE_TEXTURE_SIZE.Height)},
         .Level  = 0};

    for (i32 y {0}; y < uv.height(); ++y) {
        for (i32 x {0}; x < uv.width(); ++x) {
            color const col {_init.State.Sprites->get_pixel(point_i {x, y} + uv.top_left())};
            tex.Alpha[x, y] = col.A;
        }
    }
}

void engine::update_hud()
{
    if (_updateHUD) {
        call(_callbacks.OnDrawHUD, &_hudProxy);
        _updateHUD = false;
    }
}
