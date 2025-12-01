// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Engine.hpp"

#include "Die.hpp"
#include "Game.hpp"
#include "Slot.hpp"

using namespace scripting;

static auto from_base26(string_view s) -> u32
{
    u32 n {0};
    for (char c : s) {
        n = (n * 26) + (c - 'a' + 1);
    }
    return n;
}

static auto get_pixel(string_view s, size_i size) -> std::vector<u8>
{
    std::vector<u8> dots;
    dots.reserve(size.area());

    auto const isRLE {[](string_view s) -> bool {
        return s.size() > 1 && (s[1] >= 'a' && s[1] <= 'z');
    }};

    if (isRLE(s)) {
        usize i {0};
        while (i < s.size()) {
            char digitChar {s[i++]};
            u8   val {0};
            if (digitChar >= '0' && digitChar <= '9') {
                val = digitChar - '0';
            } else if (digitChar >= 'A' && digitChar <= 'F') {
                val = 10 + (digitChar - 'A');
            }

            usize start {i};
            while (i < s.size() && s[i] >= 'a' && s[i] <= 'z') { ++i; }

            u32 const run {from_base26(std::string_view(s.data() + start, i - start))};

            dots.insert(dots.end(), run, val);
        }
    } else {
        for (char c : s) {
            if (c >= '0' && c <= '9') {
                dots.push_back(static_cast<u8>(c - '0'));
            } else if (c >= 'A' && c <= 'F') {
                dots.push_back(static_cast<u8>(10 + (c - 'A')));
            }
        }
    }

    return dots;
}

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
        call(_callbacks.OnDieChanged, ev);
    });

    _init.Events.Start.connect([this]() { start_turn(); });
}

template <typename R>
inline auto engine::call(callback<R> const& func, auto&&... args) -> R
{
    if (func) {
        return (*func)(_table, this, args...);
    }

    return R {};
}

void engine::run(string const& file)
{
    // require
    auto        path {io::get_parent_folder(file)};
    auto const& env {**_script.Environment};
    _require       = make_shared_closure(std::function {[env, path, this](string const& module) {
        if (env.has("package", "loaded", module)) { return env["package"]["loaded"][module].as<table>(); }
        string const pkgFile {std::format("{}/{}.lua", path, module)};
        if (auto pkg {_script.run_file<table>(pkgFile)}) {
            auto& val {pkg.value()};
            env["package"]["loaded"][module] = val;
            return val;
        }
        _script.raise_error(std::format("module {} not found", module));
        return _script.create_table();
    }});
    env["require"] = _require.get();

    _table = *_script.run_file<table>(file);

    _table.try_get(_callbacks.OnCollision, "on_collision");
    _table.try_get(_callbacks.OnDieChanged, "on_die_changed");
    _table.try_get(_callbacks.OnSetup, "on_setup");
    _table.try_get(_callbacks.OnTeardown, "on_teardown");
    _table.try_get(_callbacks.Update, "update");
    _table.try_get(_callbacks.Finish, "finish");
    _table.try_get(_callbacks.CanStart, "can_start");
    _table.try_get(_callbacks.Start, "start");

    call(_callbacks.OnSetup);
}

auto engine::update(milliseconds deltaTime) -> bool
{
    _init.State.CanStart = _gameStatus == game_status::TurnEnded && call(_callbacks.CanStart);

    if (_gameStatus != game_status::Running) { return false; }

    game_status const status {static_cast<game_status>(call(_callbacks.Update, deltaTime.count()))};
    _gameStatus = status;

    switch (status) {
    case Running:   return true;
    case TurnEnded: call(_callbacks.Finish); return false;
    case GameOver:  call(_callbacks.OnTeardown); return false;
    }

    return true;
}

auto engine::start_turn() -> bool
{
    if (_gameStatus != game_status::TurnEnded) { return false; }

    if (call(_callbacks.CanStart)) {
        _init.Slots->lock();
        call(_callbacks.Start);
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

    _script.Environment = env;
}

void engine::create_wrappers()
{
    create_sprite_wrapper();
    create_slot_wrapper();
    create_engine_wrapper();
    create_dmd_wrapper();
    create_sfx_wrapper();
}

void engine::create_sprite_wrapper()
{
    static auto normal_to_world {[](point_f pos) -> point_f {
        auto const& size {VIRTUAL_SCREEN_SIZE};
        return {pos.X * size.Width, pos.Y * size.Height};
    }};
    static auto world_to_normal {[](point_f pos) -> point_f {
        auto const& size {VIRTUAL_SCREEN_SIZE};
        return {pos.X / size.Width, pos.Y / size.Height};
    }};

    auto& spriteWrapper {*_script.create_wrapper<sprite>("sprite")};
    spriteWrapper["position"] = property {
        [](sprite* sprite) { return world_to_normal(sprite->Shape->Bounds->Position); },
        [](sprite* sprite, point_f p) { sprite->Shape->Bounds = {normal_to_world(p), sprite->Shape->Bounds->Size}; }};
    spriteWrapper["size"] = property {
        [](sprite* sprite) -> size_f {
            auto const size {world_to_normal({sprite->Shape->Bounds->Size.Width, sprite->Shape->Bounds->Size.Height})};
            return {size.X, size.Y};
        },
        [](sprite* sprite, size_f p) {
            auto const size {normal_to_world({p.Width, p.Height})};
            sprite->Shape->Bounds = {sprite->Shape->Bounds->Position, {size.X, size.Y}};
        }};
    spriteWrapper["bounds"] = getter {
        [](sprite* sprite) {
            rect_f const  bounds {*sprite->Shape->Bounds};
            point_f const tl {world_to_normal(bounds.top_left())};
            point_f const br {world_to_normal(bounds.bottom_right())};
            return rect_f::FromLTRB(tl.X, tl.Y, br.X, br.Y);
        }};
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
    slotWrapper["isEmpty"] = getter {
        [](slot* slot) { return slot->is_empty(); }};
    slotWrapper["dieValue"] = getter {
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

void engine::create_engine_wrapper()
{
    auto& engineWrapper {*_script.create_wrapper<engine>("engine")};
    // gfx
    engineWrapper["create_background"] = [](engine* engine, string const& bitmap) {
        auto const bgSize {size_i {VIRTUAL_SCREEN_SIZE}};

        auto* tex {engine->_init.BackgroundTexture};
        tex->resize(bgSize, 1, gfx::texture::format::RGBA8);
        tex->regions()["default"] = {.UVRect = {0, 0, 1, 1}, .Level = 0};

        gfx::image bgImg {gfx::image::CreateEmpty(bgSize, gfx::image::format::RGBA)};

        auto const dots {get_pixel(bitmap, bgSize)};
        isize      i {0};
        for (i32 y {0}; y < bgSize.Height; ++y) {
            for (i32 x {0}; x < bgSize.Width; ++x) {
                bgImg.set_pixel({x, y}, PALETTE[dots[i++]]);
            }
        }
        tex->update_data(bgImg, 0);
        tex->Filtering = gfx::texture::filtering::Linear;
    };
    engineWrapper["create_textures"] = [](engine* engine, std::unordered_map<u32, table> const& texMap) {
        engine->_textures.clear();

        struct tex_def {
            u32                ID {0};
            size_i             Size {size_i::Zero};
            string             BitmapString;
            std::optional<u32> Transparent;
        };

        constexpr i32        PAD {2};
        std::vector<tex_def> texDefs;

        // get canvas size
        size_i texImgSize {0, 0};
        for (auto const& [id, texDefTable] : texMap) {
            tex_def& texDef {texDefs.emplace_back()};
            texDef.ID = id;
            if (!texDefTable.try_get(texDef.Size, "size")) { return; }
            if (!texDefTable.try_get(texDef.BitmapString, "bitmap")) { return; }
            texDefTable.try_get(texDef.Transparent, "transparent");

            texImgSize.Width += texDef.Size.Width + (PAD * 2);
            texImgSize.Height = std::max(texImgSize.Height, static_cast<i32>(texDef.Size.Height + (PAD * 2)));
        }

        texImgSize.Width += PAD;
        texImgSize.Height += PAD;

        // draw textures
        point_i pen {PAD, PAD};

        auto* tex {engine->_init.SpriteTexture};
        tex->resize(texImgSize, 1, gfx::texture::format::RGBA8);

        gfx::image texImg {gfx::image::CreateEmpty(texImgSize, gfx::image::format::RGBA)};
        for (auto const& texDef : texDefs) {
            auto& assTex {engine->_textures[texDef.ID]};
            assTex.Size   = size_f {texDef.Size};
            assTex.Region = std::to_string(texDef.ID);
            assTex.Alpha  = grid<u8> {texDef.Size};

            auto const dots {get_pixel(texDef.BitmapString, texDef.Size)};
            isize      i {0};
            for (i32 y {0}; y < texDef.Size.Height; ++y) {
                for (i32 x {0}; x < texDef.Size.Width; ++x) {
                    auto const  idx {dots[i++]};
                    color const color {texDef.Transparent && *texDef.Transparent == idx ? colors::Transparent : PALETTE[idx]};
                    texImg.set_pixel(pen + point_i {x, y}, color);
                    assTex.Alpha[x, y] = color.A;
                }
            }

            tex->regions()[assTex.Region] =
                {.UVRect = {static_cast<f32>(pen.X) / static_cast<f32>(texImgSize.Width),
                            static_cast<f32>(pen.Y) / static_cast<f32>(texImgSize.Height),
                            static_cast<f32>(texDef.Size.Width) / static_cast<f32>(texImgSize.Width),
                            static_cast<f32>(texDef.Size.Height) / static_cast<f32>(texImgSize.Height)},
                 .Level  = 0};

            pen.X += texDef.Size.Width + (PAD * 2);
        }
        tex->update_data(texImg, 0);
        tex->Filtering = gfx::texture::filtering::Linear;
    };
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
    engineWrapper["dmd"] = getter {
        [](engine* engine) { return &engine->_dmdProxy; }};
    engineWrapper["sfx"] = getter {
        [](engine* engine) { return &engine->_sfxProxy; }};
}

void engine::create_dmd_wrapper()
{
    auto& dmdWrapper {*_script.create_wrapper<dmd_proxy>("dmd")};
    dmdWrapper["clear"] = [](dmd_proxy* dmd) { dmd->clear(); };
    dmdWrapper["blit"]  = [](dmd_proxy* dmd, rect_i const& rect, string const& dotStr) { dmd->blit(rect, dotStr); };
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

////////////////////////////////////////////////////////////

dmd_proxy::dmd_proxy(prop<grid<u8>>& dmd)
    : _dmd {dmd}
{
}
void dmd_proxy::clear()
{
    _dmd = grid<u8> {DMD_SIZE, 0};
}
void dmd_proxy::blit(rect_i const& rect, string const& dotStr)
{
    auto const dots {get_pixel(dotStr, rect.Size)};
    _dmd.mutate([&](auto& dmd) { dmd.blit(rect, dots); });
}

////////////////////////////////////////////////////////////

auto sfx_proxy::pickup_coin(u64 seed) -> audio::sound_wave { return audio::sound_generator {random::prng_split_mix_64 {seed}}.generate_pickup_coin(); }
auto sfx_proxy::laser_shoot(u64 seed) -> audio::sound_wave { return audio::sound_generator {random::prng_split_mix_64 {seed}}.generate_laser_shoot(); }
auto sfx_proxy::explosion(u64 seed) -> audio::sound_wave { return audio::sound_generator {random::prng_split_mix_64 {seed}}.generate_explosion(); }
auto sfx_proxy::powerup(u64 seed) -> audio::sound_wave { return audio::sound_generator {random::prng_split_mix_64 {seed}}.generate_powerup(); }
auto sfx_proxy::hit_hurt(u64 seed) -> audio::sound_wave { return audio::sound_generator {random::prng_split_mix_64 {seed}}.generate_hit_hurt(); }
auto sfx_proxy::jump(u64 seed) -> audio::sound_wave { return audio::sound_generator {random::prng_split_mix_64 {seed}}.generate_jump(); }
auto sfx_proxy::blip_select(u64 seed) -> audio::sound_wave { return audio::sound_generator {random::prng_split_mix_64 {seed}}.generate_blip_select(); }
auto sfx_proxy::random(u64 seed) -> audio::sound_wave { return audio::sound_generator {random::prng_split_mix_64 {seed}}.generate_random(); }