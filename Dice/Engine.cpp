// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Engine.hpp"

#include "Die.hpp"
#include "Game.hpp"
#include "Slot.hpp"

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

    _init.Events.SlotDieChanged.connect([this](auto const& ev) {
        if (_gameStatus != game_status::TurnEnded) { return; }
        call(_callbacks.OnDieChange, ev);
    });
    _init.Events.DieMotion.connect([this]() {
        if (_gameStatus != game_status::TurnEnded) { return; }
        call(_callbacks.OnDieMotion);
    });

    _init.Events.Start.connect([this]() { start_turn(); });
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
}

auto engine::update(milliseconds deltaTime) -> bool
{
    _init.State.CanStart = _gameStatus == game_status::TurnEnded && call(_callbacks.CanStartTurn);

    if (_gameStatus != game_status::Running) { return false; }

    game_status const status {static_cast<game_status>(call(_callbacks.OnTurnUpdate, deltaTime.count()))};
    _gameStatus = status;

    switch (status) {
    case Running:   return true;
    case TurnEnded: call(_callbacks.OnTurnFinish); return false;
    case GameOver:  call(_callbacks.OnTeardown); return false;
    }

    return true;
}

auto engine::start_turn() -> bool
{
    if (_gameStatus != game_status::TurnEnded) { return false; }

    if (call(_callbacks.CanStartTurn)) {
        _init.Slots->lock();
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
    palette["Black"]      = PALETTE[0];
    palette["Gray"]       = PALETTE[1];
    palette["White"]      = PALETTE[2];
    palette["Red"]        = PALETTE[3];
    palette["Pink"]       = PALETTE[4];
    palette["DarkBrown"]  = PALETTE[5];
    palette["Brown"]      = PALETTE[6];
    palette["Orange"]     = PALETTE[7];
    palette["Yellow"]     = PALETTE[8];
    palette["DarkGreen"]  = PALETTE[9];
    palette["Green"]      = PALETTE[10];
    palette["LightGreen"] = PALETTE[11];
    palette["BlueGray"]   = PALETTE[12];
    palette["DarkBlue"]   = PALETTE[13];
    palette["Blue"]       = PALETTE[14];
    palette["LightBlue"]  = PALETTE[15];
    env["Palette"]        = palette;

    env["ScreenSize"] = VIRTUAL_SCREEN_SIZE;
    env["DMDSize"]    = DMD_SIZE;

    table gameStatus {_script.create_table()};
    gameStatus["Running"]   = static_cast<i32>(game_status::Running);
    gameStatus["TurnEnded"] = static_cast<i32>(game_status::TurnEnded);
    gameStatus["GameOver"]  = static_cast<i32>(game_status::GameOver);
    env["GameStatus"]       = gameStatus;

    table slotState {_script.create_table()};
    slotState["Idle"]   = static_cast<i32>(slot_state::Idle);
    slotState["Accept"] = static_cast<i32>(slot_state::Accept);
    slotState["Reject"] = static_cast<i32>(slot_state::Reject);
    slotState["Hover"]  = static_cast<i32>(slot_state::Hover);
    env["SlotState"]    = slotState;

    _script.Environment = env;
}

void engine::create_wrappers()
{
    create_sprite_wrapper();
    create_slot_wrapper();
    create_die_wrapper();
    create_engine_wrapper();
    create_dmd_wrapper();
    create_sfx_wrapper();
}

void engine::create_sprite_wrapper()
{
    auto& spriteWrapper {*_script.create_wrapper<sprite>("sprite")};
    spriteWrapper["position"] = property {
        [](sprite* sprite) { return sprite->Shape->Bounds->Position; },
        [](sprite* sprite, point_f p) { sprite->Shape->Bounds = {p, sprite->Shape->Bounds->Size}; }};
    spriteWrapper["size"] = property {
        [](sprite* sprite) -> size_f { return sprite->Shape->Bounds->Size; },
        [](sprite* sprite, size_f s) { sprite->Shape->Bounds = {sprite->Shape->Bounds->Position, s}; }};
    spriteWrapper["bounds"] = getter {
        [](sprite* sprite) { return *sprite->Shape->Bounds; }};
    spriteWrapper["rotation"] = property {
        [](sprite* sprite) { return sprite->Shape->Rotation->Value; },
        [](sprite* sprite, f32 p) { sprite->Shape->Rotation = degree_f {p}; }};
    spriteWrapper["scale"] = property {
        [](sprite* sprite) { return *sprite->Shape->Scale; },
        [](sprite* sprite, size_f factor) { sprite->Shape->Scale = factor; }};
    spriteWrapper["owner"] = getter {
        [](sprite* sprite) { return sprite->Owner; }};
    spriteWrapper["texture"] = property {
        [](sprite* sprite) { return sprite->TexID; },
        [this](sprite* sprite, u32 texID) { set_texture(sprite, texID); }};
}

void engine::create_slot_wrapper()
{
    auto& slotWrapper {*_script.create_wrapper<slot>("slot")};
    slotWrapper["owner"] = getter {
        [](slot* slot) { return slot->Owner; }};
    slotWrapper["is_empty"] = getter {
        [](slot* slot) { return slot->is_empty(); }};
    slotWrapper["state"] = getter {
        [this](slot* slot) { return static_cast<u8>(slot->state()); }};
    slotWrapper["die_value"] = getter {
        [](slot* slot) -> u8 { return slot->is_empty() ? 0 : slot->current_die()->current_face().Value; }};
    slotWrapper["position"] = property {
        [this](slot* slot) -> point_i {
            auto const&   rect {_init.State.DMDBounds};
            point_f const pos {slot->bounds().Position};
            return {static_cast<i32>(std::round((pos.X - rect.left()) / (rect.width() / DMD_SIZE.Width))),
                    static_cast<i32>(std::round((pos.Y - rect.top()) / (rect.height() / DMD_SIZE.Height)))};
        },
        [this](slot* slot, point_i pos) {
            auto const& rect {_init.State.DMDBounds};
            slot->move_to({rect.left() + (pos.X * (rect.width() / DMD_SIZE.Width)),
                           rect.top() + (pos.Y * (rect.height() / DMD_SIZE.Height))});
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
        engine->_sounds.clear();
        for (auto const& [id, soundWave] : soundMap) {
            engine->_sounds.emplace(id, std::make_unique<audio::sound>(audio::sound_generator {}.create_buffer(soundWave)));
        }
    };

    // functions
    engineWrapper["random"]        = [](engine* engine, f32 min, f32 max) { return engine->_init.State.Rng(min, max); };
    engineWrapper["random_int"]    = [](engine* engine, i32 min, i32 max) { return engine->_init.State.Rng(min, max); };
    engineWrapper["log"]           = [](string const& str) { logger::Info(str); };
    engineWrapper["create_sprite"] = [](engine* engine, table const& spriteDef) {
        auto* sprite {engine->_init.Game->add_sprite()};
        sprite->Owner = spriteDef;
        spriteDef.try_get(sprite->IsCollidable, "collidable");
        spriteDef.try_get(sprite->IsWrappable, "wrappable");

        auto const texID {spriteDef["texture"].as<u32>()};
        engine->set_texture(sprite, texID);
        sprite->Shape->Bounds = rect_f {point_f::Zero, sprite->Texture->Size};

        return sprite;
    };
    engineWrapper["remove_sprite"] = [](engine* engine, sprite* sprite) { engine->_init.Game->remove_sprite(sprite); };
    engineWrapper["create_slot"]   = [](engine* engine, table const& slotDef) -> slot* {
        slot_face face;
        slotDef.try_get(face.Op, "op");
        slotDef.try_get(face.Value, "value");
        slotDef.try_get(face.Color, "color");

        auto* slot {engine->_init.Slots->add_slot(face)};
        slot->Owner = slotDef;

        return slot;
    };
    engineWrapper["create_dice"] = [](engine* engine, i32 count, table const& faces) {
        std::vector<die_face> vec;
        for (i32 i {1}; i <= faces.raw_length(); ++i) {
            table            face {faces.get<table>(i).value()}; // TODO: error check
            std::vector<i32> values;
            face.try_get(values, "values");
            color color;
            face.try_get(color, "color");
            for (auto const& value : values) { vec.emplace_back(value, color); }
        }
        if (vec.empty()) { return; }
        auto& init {engine->_init};
        for (i32 i {0}; i < count; ++i) {
            init.Dice->add_die(init.Game->get_random_die_position(), init.State.Rng, vec[0], vec);
        }
    };
    engineWrapper["roll_dice"]   = [](engine* engine) { engine->_init.Dice->roll(); };
    engineWrapper["reset_slots"] = [](engine* engine, table const& slotsTable) {
        std::vector<slot*> slots;
        for (auto const& key : slotsTable.get_keys<string>()) { slots.push_back(slotsTable[key].as<slot*>()); }

        auto* s {engine->_init.Slots};
        s->unlock();
        s->reset(slots);
    };
    engineWrapper["get_hand"] = [](engine* engine, table const& slotsTable) {
        std::vector<slot*> slots;
        for (auto const& key : slotsTable.get_keys<string>()) { slots.push_back(slotsTable[key].as<slot*>()); }

        return engine->_init.Slots->get_hand(slots);
    };
    engineWrapper["give_score"] = [](engine* engine, i32 score) { engine->_init.State.Score += score; };
    engineWrapper["play_sound"] = [](engine* engine, u32 id) { engine->_sounds[id]->play(); };

    // properties
    engineWrapper["dmd"]        = getter {[](engine* engine) { return &engine->_dmdProxy; }};
    engineWrapper["sfx"]        = getter {[](engine* engine) { return &engine->_sfxProxy; }};
    engineWrapper["background"] = property {
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
    engineWrapper["ssd_value"] = property {
        [](engine* engine) { return *engine->_init.State.SSDValue; },
        [](engine* engine, string const& val) { engine->_init.State.SSDValue = val; }};
}

void engine::create_dmd_wrapper()
{
    auto& dmdWrapper {*_script.create_wrapper<dmd_proxy>("dmd")};
    dmdWrapper["clear"] = [](dmd_proxy* dmd) { dmd->clear(); };
    dmdWrapper["blit"]  = [](dmd_proxy* dmd, rect_i const& rect, string const& dotStr) { dmd->blit(rect, dotStr); };
    dmdWrapper["print"] = [](dmd_proxy* dmd, point_i pos, string_view text, std::variant<u8, color> col) {
        std::visit(overloaded {
                       [&](u8 col) { dmd->print(pos, text, PALETTE[col]); },
                       [&](color col) { dmd->print(pos, text, col); }},
                   col);
    };
}

void engine::create_sfx_wrapper()
{
    auto& sfxWrapper {*_script.create_wrapper<sfx_proxy>("sfx")};
    sfxWrapper["pickup_coin"] = [](sfx_proxy* sfx, u64 seed) { return sfx->pickup_coin(seed); };
    sfxWrapper["laser_shoot"] = [](sfx_proxy* sfx, u64 seed) { return sfx->laser_shoot(seed); };
    sfxWrapper["explosion"]   = [](sfx_proxy* sfx, u64 seed) { return sfx->explosion(seed); };
    sfxWrapper["powerup"]     = [](sfx_proxy* sfx, u64 seed) { return sfx->powerup(seed); };
    sfxWrapper["hit_hurt"]    = [](sfx_proxy* sfx, u64 seed) { return sfx->hit_hurt(seed); };
    sfxWrapper["jump"]        = [](sfx_proxy* sfx, u64 seed) { return sfx->jump(seed); };
    sfxWrapper["blip_select"] = [](sfx_proxy* sfx, u64 seed) { return sfx->blip_select(seed); };
    sfxWrapper["random"]      = [](sfx_proxy* sfx, u64 seed) { return sfx->random(seed); };
}

void engine::set_texture(sprite* sprite, u32 texID)
{
    sprite->TexID = texID;

    auto& texture {_textures[sprite->TexID]}; // TODO: error check
    sprite->Texture              = &texture;
    sprite->Shape->TextureRegion = texture.Region;
    if (sprite->WrapCopy) { sprite->WrapCopy->TextureRegion = texture.Region; }
}

void engine::create_backgrounds(std::unordered_map<u32, bg_def> const& bgMap)
{
    _backgrounds.clear();

    auto const bgSize {size_i {VIRTUAL_SCREEN_SIZE}};

    auto* tex {_init.BackgroundTexture};
    tex->resize(bgSize, bgMap.size(), gfx::texture::format::RGBA8);
    tex->regions()["default"] = {.UVRect = {0, 0, 1, 1}, .Level = 0};

    gfx::image bgImg {gfx::image::CreateEmpty(bgSize, gfx::image::format::RGBA)};
    u32        level {0};
    for (auto const& [id, bgDef] : bgMap) {
        _backgrounds[id] = std::to_string(id);

        auto const dots {get_pixel(bgDef.Bitmap, bgSize)};
        for (i32 y {0}; y < bgSize.Height; ++y) {
            for (i32 x {0}; x < bgSize.Width; ++x) {
                bgImg.set_pixel({x, y}, PALETTE[dots[x + (y * bgSize.Width)]]);
            }
        }

        tex->regions()[_backgrounds[id]] = {.UVRect = {0, 0, 1, 1}, .Level = level};
        tex->update_data(bgImg, level++);
    }

    tex->Filtering = gfx::texture::filtering::Linear;
}

void engine::create_textures(std::unordered_map<u32, tex_def>& texMap)
{
    _textures.clear();

    constexpr i32        PAD {2};
    std::vector<tex_def> texDefs;

    // get canvas size
    size_i texImgSize {0, 0};
    for (auto& [id, texDef] : texMap) {
        texDef.ID = id;
        texDefs.push_back(texDef);
        texImgSize.Width += texDef.Size.Width + (PAD * 2);
        texImgSize.Height = std::max(texImgSize.Height, static_cast<i32>(texDef.Size.Height + (PAD * 2)));
    }

    texImgSize.Width += PAD;
    texImgSize.Height += PAD;

    // draw textures
    point_i pen {PAD, PAD};

    auto* tex {_init.SpriteTexture};
    tex->resize(texImgSize, 1, gfx::texture::format::RGBA8);

    gfx::image texImg {gfx::image::CreateEmpty(texImgSize, gfx::image::format::RGBA)};
    for (auto const& texDef : texDefs) {
        auto& assTex {_textures[texDef.ID]};
        assTex.Size   = size_f {texDef.Size};
        assTex.Region = std::to_string(texDef.ID);
        assTex.Alpha  = grid<u8> {texDef.Size};

        auto const dots {get_pixel(texDef.Bitmap, texDef.Size)};
        for (i32 y {0}; y < texDef.Size.Height; ++y) {
            for (i32 x {0}; x < texDef.Size.Width; ++x) {
                auto const  idx {dots[x + (y * texDef.Size.Width)]};
                color const color {texDef.Transparent == idx ? colors::Transparent : PALETTE[idx]};
                texImg.set_pixel(pen + point_i {x, y}, color);
                assTex.Alpha[x, y] = color.A;
            }
        }

        tex->regions()[assTex.Region] =
            {.UVRect = {static_cast<f32>(pen.X) / static_cast<f32>(texImgSize.Width), static_cast<f32>(pen.Y) / static_cast<f32>(texImgSize.Height),
                        static_cast<f32>(texDef.Size.Width) / static_cast<f32>(texImgSize.Width), static_cast<f32>(texDef.Size.Height) / static_cast<f32>(texImgSize.Height)},
             .Level  = 0};

        pen.X += texDef.Size.Width + (PAD * 2);
    }

    tex->update_data(texImg, 0);
    tex->Filtering = gfx::texture::filtering::NearestNeighbor;
}
