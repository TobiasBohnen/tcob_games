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
    , _dmdProxy {init.State.DMD, PALETTE[0]}
    , _fgProxy {init.State.Foreground, colors::Transparent}
    , _bgProxy {init.State.Background, PALETTE[0]}
    , _texProxy {init.State.Sprites, colors::Transparent}
{
    create_env();
    create_wrappers();

    _init.Events.Collision.connect([this](auto const& ev) {
        if (_gameStatus != game_status::Running) { return; }
        call(_callbacks.OnCollision, ev.A, ev.B);
    });

    _init.Events.SocketDieChanged.connect([this](auto const& ev) {
        if (_gameStatus != game_status::TurnEnded) { return; }
        call(_callbacks.OnDrawDMD, ev);
    });
    _init.Events.DieMotion.connect([this]() {
        if (_gameStatus != game_status::TurnEnded) { return; }
        call(_callbacks.OnDrawDMD);
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
    // require
    auto const path {io::get_parent_folder(file)};

    _require = make_shared_closure(std::function {[this, path](char const* module) {
        auto const& env {**_script.Environment};
        if (env.has("package", "loaded", module)) { return env["package"]["loaded"][module].as<table>(); }

        if (auto pkg {_script.run_file<table>(std::format("{}/{}.lua", path, module))}) {
            auto& val {pkg.value()};
            env["package"]["loaded"][module] = val;
            return val;
        }

        _script.view().error("module %s not found", module);
        return _script.create_table();
    }});

    (**_script.Environment)["require"] = _require.get();

    _table = *_script.run_file<table>(file);

    _table.try_get(_callbacks.OnCollision, "on_collision");
    _table.try_get(_callbacks.OnSetup, "on_setup");
    _table.try_get(_callbacks.OnTeardown, "on_teardown");
    _table.try_get(_callbacks.OnTurnUpdate, "on_turn_update");
    _table.try_get(_callbacks.OnTurnFinish, "on_turn_finish");
    _table.try_get(_callbacks.OnTurnStart, "on_turn_start");
    _table.try_get(_callbacks.OnDrawDMD, "on_draw_dmd");

    call(_callbacks.OnSetup);
    call(_callbacks.OnDrawDMD);
}

auto engine::update(milliseconds deltaTime) -> bool
{
    _init.State.CanStart = _gameStatus == game_status::TurnEnded;

    if (_gameStatus != game_status::Running) { return false; }

    for (auto& channel : _soundChannels) {
        channel.update();
    }

    game_status const status {static_cast<game_status>(call(_callbacks.OnTurnUpdate, deltaTime.count(), _turnTime))};
    _turnTime += deltaTime.count();
    _gameStatus = status;

    switch (status) {
    case Running:   return true;
    case TurnEnded: {
        call(_callbacks.OnTurnFinish);
        _init.Game->reset_sockets();
        call(_callbacks.OnDrawDMD);
        _turnTime = 0;
        return false;
    }
    case GameOver:
        call(_callbacks.OnTeardown);
        return false;
    }

    return true;
}

auto engine::start_turn() -> bool
{
    if (_gameStatus != game_status::TurnEnded) { return false; }

    call(_callbacks.OnTurnStart);
    call(_callbacks.OnDrawDMD);
    _gameStatus = game_status::Running;
    return true;
}

void engine::create_env()
{
    _script.open_libraries(library::Table, library::String, library::Math);
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

    table palette {_script.create_table()};
    for (auto const& color : PALETTE_MAP) {
        palette[color.first] = color.second;
    }
    env["Palette"] = palette;

    table gameStatus {_script.create_table()};
    gameStatus["Running"]   = static_cast<i32>(game_status::Running);
    gameStatus["TurnEnded"] = static_cast<i32>(game_status::TurnEnded);
    gameStatus["GameOver"]  = static_cast<i32>(game_status::GameOver);
    env["GameStatus"]       = gameStatus;

    table socketState {_script.create_table()};
    socketState["Idle"]   = static_cast<i32>(socket_state::Idle);
    socketState["Accept"] = static_cast<i32>(socket_state::Accept);
    socketState["Reject"] = static_cast<i32>(socket_state::Reject);
    socketState["Hover"]  = static_cast<i32>(socket_state::Hover);
    env["SocketState"]    = socketState;

    table rotation {_script.create_table()};
    rotation["R0"]   = 0;
    rotation["R90"]  = 90;
    rotation["R180"] = 180;
    rotation["R270"] = 270;
    env["Rot"]       = rotation;

    static auto const convert_sockets {[](std::unordered_map<std::variant<i32, string>, socket*> const& socketMap) {
        std::vector<socket*> sockets;
        sockets.reserve(socketMap.size());
        for (auto const& key : socketMap) { sockets.push_back(key.second); }
        return sockets;
    }};
    env["get_hand"] = +[](std::unordered_map<std::variant<i32, string>, socket*> const& sockets) -> hand {
        return get_hand(convert_sockets(sockets));
    };
    env["get_sum"] = +[](std::unordered_map<std::variant<i32, string>, socket*> const& sockets) -> u32 {
        return get_sum(convert_sockets(sockets));
    };
    env["get_value"] = +[](std::unordered_map<std::variant<i32, string>, socket*> const& sockets, i32 baseHandValue) -> u32 {
        auto retValue {get_sum(convert_sockets(sockets))};

        auto const hand {get_hand(convert_sockets(sockets))};
        switch (hand.Value) {
        case value_category::None:         break;
        case value_category::OnePair:      retValue += baseHandValue * 2; break;
        case value_category::TwoPair:      retValue += baseHandValue * 3; break;
        case value_category::ThreeOfAKind: retValue += baseHandValue * 4; break;
        case value_category::Straight:     retValue += baseHandValue * 5; break;
        case value_category::FullHouse:    retValue += baseHandValue * 6; break;
        case value_category::FourOfAKind:  retValue += baseHandValue * 8; break;
        case value_category::FiveOfAKind:  retValue += baseHandValue * 10; break;
        }

        return retValue;
    };

    _script.Environment = env;
    _script.Warning.connect([](scripting::script::warning_event const& ev) {
        logger::Warning(ev.Message);
    });
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
        [this](sprite* sprite, u32 texID) { sprite->set_texture(&_textures[texID]); }}; // TODO: error check
}

void engine::create_socket_wrapper()
{
    auto& socketWrapper {*_script.create_wrapper<socket>("socket")};
    socketWrapper["is_empty"] = getter {
        [](socket* socket) -> bool { return socket->is_empty(); }};
    socketWrapper["state"] = getter {
        [](socket* socket) -> u8 { return static_cast<u8>(socket->state()); }};
    socketWrapper["die_value"] = getter {
        [](socket* socket) -> u8 { return socket->is_empty() ? 0 : socket->current_die()->current_face().Value; }};
    socketWrapper["position"] = property {
        [this](socket* socket) -> point_i {
            auto const&   rect {_init.State.DMDBounds};
            point_f const pos {socket->bounds().Position};
            return {static_cast<i32>(std::round((pos.X - rect.left()) / (rect.width() / DMD_SIZE.Width))),
                    static_cast<i32>(std::round((pos.Y - rect.top()) / (rect.height() / DMD_SIZE.Height)))};
        },
        [this](socket* socket, point_i pos) {
            auto const& rect {_init.State.DMDBounds};
            socket->move_to({rect.left() + (static_cast<f32>(pos.X) * (rect.width() / static_cast<f32>(DMD_SIZE.Width))),
                             rect.top() + (static_cast<f32>(pos.Y) * (rect.height() / static_cast<f32>(DMD_SIZE.Height)))});
        }};
}

void engine::create_engine_wrapper()
{
    auto& engineWrapper {*_script.create_wrapper<engine>("engine")};
    // gfx
    engineWrapper["create_texture"] = [](engine* engine, u32 id, rect_i const& uv) { engine->create_texture(id, uv); };

    // sfx
    engineWrapper["create_sounds"] = [](engine* engine, std::unordered_map<u32, audio::sound_wave> const& soundMap) {
        for (auto const& [id, soundWave] : soundMap) {
            engine->_soundBank.emplace(id, audio::sound_generator {}.create_buffer(soundWave));
        }
    };

    // functions
    engineWrapper["rnd"]           = [](engine* engine, f32 min, f32 max) { return engine->_init.State.Rng(min, max); };
    engineWrapper["irnd"]          = [](engine* engine, i32 min, i32 max) { return engine->_init.State.Rng(min, max); };
    engineWrapper["log"]           = [](string const& str) { logger::Info(str); };
    engineWrapper["create_sprite"] = [](engine* engine, table const& spriteOwner) {
        sprite_def const def {spriteOwner["spriteInit"].get<sprite_def>().value_or(sprite_def {})};

        texture*   tex {nullptr};
        auto const texID {def.TexID};
        if (texID && engine->_textures.contains(*texID)) {
            tex = &engine->_textures[*texID];
        }

        return engine->_init.Game->add_sprite({.Def = def, .Texture = tex, .Owner = spriteOwner});
    };
    engineWrapper["remove_sprite"] = [](engine* engine, sprite* sprite) { engine->_init.Game->remove_sprite(sprite); };
    engineWrapper["create_socket"] = [](engine* engine, table const& socketInit) -> socket* {
        socket_face const face {socketInit.get<socket_face>().value_or(socket_face {})};
        return engine->_init.Game->add_socket(face);
    };
    engineWrapper["remove_socket"] = [](engine* engine, socket* socket) {
        engine->_init.Game->remove_socket(socket);
    };
    engineWrapper["give_score"] = [](engine* engine, i32 score) { engine->_init.State.Score += score; };
    engineWrapper["play_sound"] = [](engine* engine, u32 soundID, std::optional<u8> channelID, std::optional<bool> now) {
        if (channelID >= engine->_soundChannels.size()) {
            logger::Warning("Invalid channel id: {}", *channelID);
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

    // properties
    engineWrapper["screenSize"] = getter {[]() -> size_i { return size_i {VIRTUAL_SCREEN_SIZE}; }};
    engineWrapper["dmd"]        = getter {[](engine* engine) -> tex_proxy* { return &engine->_dmdProxy; }};
    engineWrapper["fg"]         = getter {[](engine* engine) -> tex_proxy* { return &engine->_fgProxy; }};
    engineWrapper["bg"]         = getter {[](engine* engine) -> tex_proxy* { return &engine->_bgProxy; }};
    engineWrapper["spr"]        = getter {[](engine* engine) -> tex_proxy* { return &engine->_texProxy; }};
    engineWrapper["ssd"]        = property {
        [](engine* engine) -> string { return *engine->_init.State.SSD; },
        [](engine* engine, string const& val) { engine->_init.State.SSD = val; }};
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
    texWrapper["circle"] = [](tex_proxy* tex, point_f center, i32 radius, u8 color, bool fill) { tex->circle(point_i {center}, radius, color, fill); };
    texWrapper["rect"]   = [](tex_proxy* tex, rect_f const& rect, u8 color, bool fill) { tex->rect(rect_i {rect}, color, fill); };

    texWrapper["blit"] = [](tex_proxy* tex, rect_f const& rect, string const& dotStr, std::optional<blit_settings> settings) {
        tex->blit(rect_i {rect}, dotStr, settings ? *settings : blit_settings {});
    };
    texWrapper["print"] = [](tex_proxy* tex, point_i pos, string_view text, u8 col, std::optional<font_type> fontType) {
        tex->print(pos, text, col, fontType ? *fontType : font_type::Font5x5);
    };
    texWrapper["socket"] = [this](tex_proxy* tex, socket* socket) { tex->socket(socket, _init.State.DMDBounds); };
}

void engine::create_texture(u32 id, rect_i const& uv)
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
