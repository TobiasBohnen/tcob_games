// Copyright (c) 2025 Tobias Bohnen
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
    , _dmdProxy {init.State.DMD}
{
    create_env();
    create_wrappers();

    _init.Events.Collision.connect([this](auto const& ev) {
        if (_gameStatus != game_status::Running) { return; }
        call(_callbacks.OnCollision, ev.A, ev.B);
    });

    _init.Events.SocketDieChanged.connect([this](auto const& ev) {
        if (_gameStatus != game_status::TurnEnded) { return; }
        call(_callbacks.OnDieChange, ev);
    });
    _init.Events.DieMotion.connect([this]() {
        if (_gameStatus != game_status::TurnEnded) { return; }
        call(_callbacks.OnDieMotion);
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
    _table.try_get(_callbacks.OnDieChange, "on_die_change");
    _table.try_get(_callbacks.OnDieMotion, "on_die_motion");
    _table.try_get(_callbacks.OnSetup, "on_setup");
    _table.try_get(_callbacks.OnTeardown, "on_teardown");
    _table.try_get(_callbacks.OnTurnUpdate, "on_turn_update");
    _table.try_get(_callbacks.OnTurnFinish, "on_turn_finish");
    _table.try_get(_callbacks.CanStartTurn, "can_start_turn");
    _table.try_get(_callbacks.OnTurnStart, "on_turn_start");

    call(_callbacks.OnSetup);
    _init.Game->roll();

    _init.State.CanStart = call(_callbacks.CanStartTurn);
}

auto engine::update(milliseconds deltaTime) -> bool
{
    _init.State.CanStart = _gameStatus == game_status::TurnEnded && call(_callbacks.CanStartTurn);

    if (_gameStatus != game_status::Running) { return false; }

    _turnTime += deltaTime.count();

    game_status const status {static_cast<game_status>(call(_callbacks.OnTurnUpdate, deltaTime.count(), _turnTime))};
    _gameStatus = status;

    switch (status) {
    case Running:   return true;
    case TurnEnded: {
        _init.Sockets->reset();
        call(_callbacks.OnTurnFinish);
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

    if (call(_callbacks.CanStartTurn)) {
        _init.Sockets->lock();
        call(_callbacks.OnTurnStart);
        _gameStatus = game_status::Running;
        return true;
    }

    return false;
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

    env["ScreenSize"] = VIRTUAL_SCREEN_SIZE;
    env["DMDSize"]    = DMD_SIZE;

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

    _script.Environment = env;
    _script.Warning.connect([](scripting::script::warning_event const& ev) {
        logger::Warning(ev.Message);
    });
}

void engine::create_wrappers()
{
    create_sprite_wrapper();
    create_socket_wrapper();
    create_die_wrapper();
    create_engine_wrapper();
    create_dmd_wrapper();
}

void engine::create_sprite_wrapper()
{
    auto& spriteWrapper {*_script.create_wrapper<sprite>("sprite")};
    spriteWrapper["position"] = property {
        [](sprite* sprite) { return sprite->Bounds.Position; },
        [](sprite* sprite, point_f p) { sprite->set_bounds(p, sprite->Bounds.Size); }};
    spriteWrapper["size"] = property {
        [](sprite* sprite) -> size_f { return sprite->Bounds.Size; },
        [](sprite* sprite, size_f s) { sprite->set_bounds(sprite->Bounds.Position, s); }};
    spriteWrapper["bounds"] = getter {
        [](sprite* sprite) { return sprite->Bounds; }};
    spriteWrapper["owner"] = getter {
        [](sprite* sprite) { return sprite->owner(); }};
    spriteWrapper["texture"] = property {
        [](sprite* sprite) { return sprite->get_texture()->ID; },
        [this](sprite* sprite, u32 texID) { sprite->set_texture(&_textures[texID]); }}; // TODO: error check
}

void engine::create_socket_wrapper()
{
    auto& socketWrapper {*_script.create_wrapper<socket>("socket")};
    socketWrapper["is_empty"] = getter {
        [](socket* socket) { return socket->is_empty(); }};
    socketWrapper["state"] = getter {
        [](socket* socket) { return static_cast<u8>(socket->state()); }};
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

void engine::create_die_wrapper()
{
    auto& dieWrapper {*_script.create_wrapper<die>("die")};
    dieWrapper["value"] = getter {
        [](die* die) -> u8 { return die->current_face().Value; }};
}

void engine::create_engine_wrapper()
{
    auto& engineWrapper {*_script.create_wrapper<engine>("engine")};
    // gfx
    engineWrapper["create_backgrounds"] = [](engine* engine, std::unordered_map<u32, bg_def> const& bgMap) { engine->create_backgrounds(bgMap); };
    engineWrapper["create_textures"]    = [](engine* engine, std::unordered_map<u32, tex_def>& texMap) { engine->create_textures(texMap); };

    // sfx
    engineWrapper["create_sounds"] = [](engine* engine, std::unordered_map<u32, audio::sound_wave> const& soundMap) {
        engine->_soundBank.clear();
        for (auto const& [id, soundWave] : soundMap) {
            engine->_soundBank.emplace(id, audio::sound_generator {}.create_buffer(soundWave));
        }
    };

    // functions
    engineWrapper["rnd"]           = [](engine* engine, f32 min, f32 max) { return engine->_init.State.Rng(min, max); };
    engineWrapper["irnd"]          = [](engine* engine, i32 min, i32 max) { return engine->_init.State.Rng(min, max); };
    engineWrapper["log"]           = [](string const& str) { logger::Info(str); };
    engineWrapper["create_sprite"] = [](engine* engine, table const& spriteOwner) {
        sprite_def const def {spriteOwner["spriteInit"].get<sprite_def>().value_or({})};
        auto*            sprite {engine->_init.Game->add_sprite({.IsCollidable = def.IsCollidable,
                                                                 .IsWrappable  = def.IsWrappable,
                                                                 .Owner        = spriteOwner})};
        sprite->set_texture(&engine->_textures[def.Texture]); // TODO: error check
        sprite->set_bounds(def.Position, sprite->get_texture()->Size);
        return sprite;
    };
    engineWrapper["remove_sprite"] = [](engine* engine, sprite* sprite) { engine->_init.Game->remove_sprite(sprite); };
    engineWrapper["create_socket"] = [](engine* engine, table const& socketInit) -> socket* {
        socket_face const face {socketInit.get<socket_face>().value_or({})};
        return engine->_init.Sockets->add_socket(face);
    };
    engineWrapper["remove_socket"] = [](engine* engine, socket* socket) {
        engine->_init.Sockets->remove_socket(socket);
    };
    engineWrapper["get_hand"] = [](engine*, table const& socketsTable) {
        std::vector<socket*> sockets;
        for (auto const& key : socketsTable.get_keys<string>()) { sockets.push_back(socketsTable[key].as<socket*>()); }

        return get_hand(sockets);
    };
    engineWrapper["give_score"] = [](engine* engine, i32 score) { engine->_init.State.Score += score; };
    engineWrapper["play_sound"] = [](engine* engine, u32 id) {
        auto& sound {engine->_sounds[engine->_soundIdx++]};
        sound = std::make_unique<audio::sound>(engine->_soundBank[id]);
        sound->play();
        engine->_soundIdx = engine->_soundIdx % engine->_sounds.size();
    };

    // properties
    engineWrapper["dmd"] = getter {[](engine* engine) { return &engine->_dmdProxy; }};
    engineWrapper["bg"]  = property {
        [](engine* engine) -> u32 {
            for (auto const& [k, v] : engine->_backgrounds) {
                if (v == engine->_init.State.Background) { return k; }
            }
            return 0;
        },
        [](engine* engine, i32 idx) {
            if (!engine->_backgrounds.contains(idx)) { engine->_script.view().error("missing background"); }
            engine->_init.State.Background = engine->_backgrounds[idx];
        }};
    engineWrapper["ssd"] = property {
        [](engine* engine) { return *engine->_init.State.SSDValue; },
        [](engine* engine, string const& val) { engine->_init.State.SSDValue = val; }};
}

void engine::create_dmd_wrapper()
{
    auto& dmdWrapper {*_script.create_wrapper<dmd_proxy>("dmd")};
    dmdWrapper["clear"] = [](dmd_proxy* dmd) { dmd->clear(); };

    dmdWrapper["line"]   = [](dmd_proxy* dmd, point_i start, point_i end, u8 color) { dmd->line(start, end, color); };
    dmdWrapper["circle"] = [](dmd_proxy* dmd, point_i center, i32 radius, u8 color, bool fill) { dmd->circle(center, radius, color, fill); };
    dmdWrapper["rect"]   = [](dmd_proxy* dmd, rect_i const& rect, u8 color, bool fill) { dmd->rect(rect, color, fill); };

    dmdWrapper["blit"]  = [](dmd_proxy* dmd, rect_i const& rect, string const& dotStr) { dmd->blit(rect, dotStr); };
    dmdWrapper["print"] = [](dmd_proxy* dmd, point_i pos, string_view text, u8 col) { dmd->print(pos, text, col); };
}

void engine::create_backgrounds(std::unordered_map<u32, bg_def> const& bgMap)
{
    auto const bgSize {size_i {VIRTUAL_SCREEN_SIZE}};

    auto* tex {_init.BackgroundTexture};
    tex->resize(bgSize, MAX_BACKGROUNDS, gfx::texture::format::RGBA8);
    tex->regions()["default"] = {.UVRect = {0, 0, 1, 1}, .Level = 0};

    gfx::image bgImg {gfx::image::CreateEmpty(bgSize, gfx::image::format::RGBA)};
    for (auto const& [id, bgDef] : bgMap) {
        if (_backgroundLevel >= MAX_BACKGROUNDS) {
            _backgroundLevel = _backgroundLevel % MAX_BACKGROUNDS;
            logger::Warning("bg overflow");
        }
        _backgrounds[id] = std::to_string(id);

        auto const dots {decode_texture_pixels(bgDef.Bitmap, bgSize)};
        for (i32 y {0}; y < bgSize.Height; ++y) {
            for (i32 x {0}; x < bgSize.Width; ++x) {
                bgImg.set_pixel({x, y}, PALETTE[dots[x + (y * bgSize.Width)]]);
            }
        }

        tex->regions()[_backgrounds[id]] = {.UVRect = {0, 0, 1, 1}, .Level = _backgroundLevel};
        tex->update_data(bgImg, _backgroundLevel++);
    }

    tex->Filtering = gfx::texture::filtering::Linear;
}

void engine::create_textures(std::unordered_map<u32, tex_def>& texMap)
{
    i32 rowHeight {0};

    auto* tex {_init.SpriteTexture};
    tex->resize(TEXTURE_SIZE, 1, gfx::texture::format::RGBA8);

    for (auto& [id, texDef] : texMap) {
        if (_texturePen.X + texDef.Size.Width + TEX_PAD > TEXTURE_SIZE.Width) {
            _texturePen.X = TEX_PAD;
            _texturePen.Y += rowHeight + (TEX_PAD * 2);
            rowHeight = 0;
        }
        if (_texturePen.Y + texDef.Size.Height + TEX_PAD > TEXTURE_SIZE.Height) {
            _texturePen.Y = TEX_PAD;
            rowHeight     = 0;
            logger::Warning("tex overflow");
        }

        auto& assTex {_textures[id]};
        assTex.ID     = id;
        assTex.Size   = size_f {texDef.Size};
        assTex.Region = std::to_string(id);
        assTex.Alpha  = grid<u8> {texDef.Size};

        u32 const  rotation {texDef.Rotation.value_or(0)};
        bool const flip_h {texDef.FlipH.value_or(false)};
        bool const flip_v {texDef.FlipV.value_or(false)};

        auto const dots {decode_texture_pixels(texDef.Bitmap, texDef.Size)};
        auto const w {texDef.Size.Width};
        auto const h {texDef.Size.Height};

        auto const map_xy {[&](i32 x, i32 y) -> point_i {
            i32 const r {w - 1};
            i32 const b {h - 1};
            i32 const fx {flip_h ? r - x : x};
            i32 const fy {flip_v ? h - 1 - y : y};
            switch (rotation) {
            case 90:  return {b - fy, fx};
            case 180: return {r - fx, b - fy};
            case 270: return {fy, r - fx};
            default:  return {fx, fy};
            }
        }};
        for (i32 y {0}; y < h; ++y) {
            for (i32 x {0}; x < w; ++x) {
                auto const  idx {dots[x + (y * w)]};
                color const col {texDef.Transparent == idx ? colors::Transparent : PALETTE[idx]};
                auto const  dst {map_xy(x, y)};
                _textureImage.set_pixel(_texturePen + dst, col);
                assTex.Alpha[dst.X, dst.Y] = col.A;
            }
        }

        tex->regions()[assTex.Region] =
            {.UVRect = {static_cast<f32>(_texturePen.X) / static_cast<f32>(TEXTURE_SIZE.Width),
                        static_cast<f32>(_texturePen.Y) / static_cast<f32>(TEXTURE_SIZE.Height),
                        static_cast<f32>(texDef.Size.Width) / static_cast<f32>(TEXTURE_SIZE.Width),
                        static_cast<f32>(texDef.Size.Height) / static_cast<f32>(TEXTURE_SIZE.Height)},
             .Level  = 0};

        rowHeight = std::max(rowHeight, texDef.Size.Height);
        _texturePen.X += texDef.Size.Width + (TEX_PAD * 2);
    }

    tex->update_data(_textureImage, 0);
    tex->Filtering = gfx::texture::filtering::NearestNeighbor;
}
