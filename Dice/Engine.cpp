// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Engine.hpp"

#include "Die.hpp"
#include "Game.hpp"
#include "Slot.hpp"

using namespace scripting;

auto from_base26(string_view s) -> u32
{
    u32 n {0};
    for (char c : s) {
        n = (n * 26) + (c - 'a' + 1);
    }
    return n;
}
auto get_texture_pattern(string_view s, size_i size) -> std::vector<u8>
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

engine::engine(base_game& game, shared_state& state)
    : _game {game}
    , _sharedState {state}
    , _dmdProxy {state}
{
    _game.Collision.connect([this](auto const& ev) {
        if (!_running) { return; }
        call(_callbacks.OnCollision, ev.A, ev.B);
    });

    _game.SlotDieChanged.connect([this](auto const& ev) {
        if (_running) { return; }
        call(_callbacks.OnSlotDieChanged, ev);
    });
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
    create_env(io::get_parent_folder(file));
    create_wrappers();

    _table = *_script.run_file<table>(file);

    create_gfx();
    create_sfx();

    _table.try_get(_callbacks.OnCollision, "on_collision");
    _table.try_get(_callbacks.OnSlotDieChanged, "on_slot_die_changed");
    _table.try_get(_callbacks.OnSetup, "on_setup");
    _table.try_get(_callbacks.OnRun, "on_run");
    _table.try_get(_callbacks.OnFinish, "on_finish");
    _table.try_get(_callbacks.CanStart, "can_start");
    _table.try_get(_callbacks.OnStart, "on_start");

    call(_callbacks.OnSetup);
}

auto engine::update(milliseconds deltaTime) -> bool
{
    _sharedState.CanStart = !_running && call(_callbacks.CanStart);

    if (!_running) { return false; }

    if (!call(_callbacks.OnRun, deltaTime.count())) {
        call(_callbacks.OnFinish);
        _running = false;
        return false;
    }

    return true;
}

auto engine::start_turn() -> bool
{
    if (_running) { return false; }

    if (call(_callbacks.CanStart)) {
        _game.get_slots()->lock();
        call(_callbacks.OnStart);
        _running = true;
        return true;
    }

    return false;
}

void engine::create_env(string const& path)
{
    auto makeFunc {[this](auto&& func) {
        auto ptr {make_shared_closure(std::function {func})};
        _funcs.push_back(ptr);
        return ptr.get();
    }};

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
    env["palette"]        = palette;

    _script.Environment = env;

    // require
    env["require"] = makeFunc([env, path, this](string const& package) {
        if (env.has("package", "loaded", package)) {
            return env["package"]["loaded"][package].as<table>();
        }

        string pkgFile {package + ".lua"};
        if (!io::exists(pkgFile)) { pkgFile = path + "/" + pkgFile; }

        auto pkg {_script.run_file<table>(pkgFile).value()};
        env["package"]["loaded"][package] = pkg;
        return pkg;
    });
}

auto engine::normal_to_world(point_f pos) const -> point_f
{
    auto const& size {VIRTUAL_SCREEN_SIZE};
    return {pos.X * size.Width, pos.Y * size.Height};
}
auto engine::world_to_normal(point_f pos) const -> point_f
{
    auto const& size {VIRTUAL_SCREEN_SIZE};
    return {pos.X / size.Width, pos.Y / size.Height};
}

void engine::create_wrappers()
{
    create_sprite_wrapper();
    create_slot_wrapper();
    create_engine_wrapper();
    create_dmd_wrapper();
}

void engine::create_sprite_wrapper()
{
    auto& spriteWrapper {*_script.create_wrapper<sprite>("sprite")};
    spriteWrapper["Position"] = property {[this](sprite* sprite) { return world_to_normal(sprite->Shape->Bounds->Position); },
                                          [this](sprite* sprite, point_f p) { sprite->Shape->Bounds = {normal_to_world(p), sprite->Shape->Bounds->Size}; }};
    spriteWrapper["Size"]     = property {
        [this](sprite* sprite) -> size_f {
            auto const size {world_to_normal({sprite->Shape->Bounds->Size.Width, sprite->Shape->Bounds->Size.Height})};
            return {size.X, size.Y};
        },
        [this](sprite* sprite, size_f p) {
            auto const size {normal_to_world({p.Width, p.Height})};
            sprite->Shape->Bounds = {sprite->Shape->Bounds->Position, {size.X, size.Y}};
        }};
    spriteWrapper["Bounds"]   = getter {[this](sprite* sprite) {
        rect_f const  bounds {*sprite->Shape->Bounds};
        point_f const tl {world_to_normal(bounds.top_left())};
        point_f const br {world_to_normal(bounds.bottom_right())};
        return rect_f::FromLTRB(tl.X, tl.Y, br.X, br.Y);
    }};
    spriteWrapper["Rotation"] = property {[](sprite* sprite) { return sprite->Shape->Rotation->Value; },
                                          [](sprite* sprite, f32 p) { sprite->Shape->Rotation = degree_f {p}; }};
    spriteWrapper["Scale"]    = property {[](sprite* sprite) { return *sprite->Shape->Scale; },
                                       [](sprite* sprite, size_f factor) { sprite->Shape->Scale = factor; }};
    spriteWrapper["Owner"]    = getter {[](sprite* sprite) { return sprite->Owner; }};
    spriteWrapper["Texture"]  = property {[](sprite* sprite) { return sprite->TexID; },
                                         [this](sprite* sprite, u32 texID) { set_texture(sprite, texID); }};
}

void engine::create_slot_wrapper()
{
    auto& slotWrapper {*_script.create_wrapper<slot>("slot")};
    slotWrapper["Owner"]    = getter {[](slot* slot) { return slot->Owner; }};
    slotWrapper["IsEmpty"]  = getter {[](slot* slot) { return slot->is_empty(); }};
    slotWrapper["DieValue"] = getter {[](slot* slot) -> u8 { return slot->is_empty() ? 0 : slot->current_die()->current_face().Value; }};
}

void engine::create_engine_wrapper()
{
    auto& engineWrapper {*_script.create_wrapper<engine>("engine")};
    engineWrapper["random"]        = [](engine* engine, f32 min, f32 max) { return engine->_sharedState.Rng(min, max); };
    engineWrapper["random_int"]    = [](engine* engine, i32 min, i32 max) { return engine->_sharedState.Rng(min, max); };
    engineWrapper["log"]           = [](string const& str) { logger::Info(str); };
    engineWrapper["create_sprite"] = [](engine* engine, table const& spriteDef) {
        auto* sprite {engine->_game.add_sprite()};

        spriteDef.try_get(sprite->IsCollidable, "collidable");
        spriteDef.try_get(sprite->IsWrappable, "wrappable");
        sprite->Owner = spriteDef;

        sprite->Shape           = engine->_game.add_shape();
        sprite->Shape->Material = engine->_sharedState.SpriteMaterial;

        auto const texID {spriteDef["texture"].as<u32>()};
        engine->set_texture(sprite, texID);
        sprite->Shape->Bounds = rect_f {point_f::Zero, sprite->Texture->Size};

        return sprite;
    };
    engineWrapper["remove_sprite"] = [](engine* engine, sprite* sprite) {
        engine->_game.remove_shape(sprite->Shape);
        if (sprite->WrapCopy) { engine->_game.remove_shape(sprite->WrapCopy); }
        engine->_game.remove_sprite(sprite);
    };
    engineWrapper["create_slot"] = [this](engine* engine, table const& slotDef) -> slot* {
        slot_face face;
        slotDef.try_get(face.Op, "op");
        slotDef.try_get(face.Value, "value");
        slotDef.try_get(face.Color, "color");

        auto* slot {engine->_game.add_slot(face)};
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
            for (auto const& value : values) {
                vec.emplace_back(value, color);
            }
        }
        if (vec.empty()) { return; }
        for (i32 i {0}; i < count; ++i) {
            engine->_game.add_die(vec);
        }
    };
    engineWrapper["roll_dice"]   = [](engine* engine) { engine->_game.roll(); };
    engineWrapper["reset_slots"] = [](engine* engine, table const& slotsTable) {
        std::vector<slot*> slots;
        for (auto const& key : slotsTable.get_keys<string>()) {
            slots.push_back(slotsTable[key].as<slot*>());
        }

        auto* s {engine->_game.get_slots()};
        s->unlock();
        s->reset(slots);
    };
    engineWrapper["give_score"] = [](engine* engine, i32 score) {
        engine->_sharedState.Score += score;
    };
    engineWrapper["DMD"] = getter {[](engine* engine) { return &engine->_dmdProxy; }};
}

void engine::create_dmd_wrapper()
{
    auto& dmdWrapper {*_script.create_wrapper<dmd_proxy>("dmd")};
    dmdWrapper["clear"] = [](dmd_proxy* dmd) {
        dmd->clear();
    };
    dmdWrapper["blit"] = [](dmd_proxy* dmd, rect_i const& rect, string const& dotStr) {
        dmd->blit(rect, dotStr);
    };
}

struct tex_def {
    u32                ID {0};
    size_i             Size {size_i::Zero};
    string             Pattern;
    std::optional<u32> Transparent;
};

auto engine::create_gfx() -> bool
{
    // draw background
    if (function<string> func; _table.try_get(func, "get_background")) {
        auto const bgSize {size_i {VIRTUAL_SCREEN_SIZE}};

        auto& tex {_sharedState.BackgroundTexture};
        tex->resize(bgSize, 1, gfx::texture::format::RGBA8);
        tex->Filtering                                        = gfx::texture::filtering::Linear;
        tex->regions()["default"]                             = {.UVRect = {0, 0, 1, 1}, .Level = 0};
        _sharedState.BackgroundMaterial->first_pass().Texture = tex;

        gfx::image bgImg {gfx::image::CreateEmpty(bgSize, gfx::image::format::RGBA)};

        auto const pattern {func(_table, this, bgSize)};
        auto const dots {get_texture_pattern(pattern, bgSize)};
        isize      i {0};
        for (i32 y {0}; y < bgSize.Height; ++y) {
            for (i32 x {0}; x < bgSize.Width; ++x) {
                bgImg.set_pixel({x, y}, PALETTE[dots[i++]]);
            }
        }
        tex->update_data(bgImg, 0);
    } else {
        return false;
    }

    constexpr i32 PAD {2};

    // textures
    if (function<std::unordered_map<u32, table>> func; _table.try_get(func, "get_textures")) {
        auto const           texMap {func(_table, this)};
        std::vector<tex_def> texDefs;

        // get canvas size
        size_i texImgSize {0, 0};
        for (auto const& [id, texDefTable] : texMap) {
            tex_def& texDef {texDefs.emplace_back()};
            texDef.ID = id;
            if (!texDefTable.try_get(texDef.Size, "size")) { return false; }
            if (!texDefTable.try_get(texDef.Pattern, "pattern")) { return false; }
            texDefTable.try_get(texDef.Transparent, "transparent");

            texImgSize.Width += texDef.Size.Width + (PAD * 2);
            texImgSize.Height = std::max(texImgSize.Height, static_cast<i32>(texDef.Size.Height + (PAD * 2)));
        }

        texImgSize.Width += PAD;
        texImgSize.Height += PAD;

        // draw textures
        point_i pen {PAD, PAD};

        auto& tex {_sharedState.SpriteTexture};
        tex->resize(texImgSize, 1, gfx::texture::format::RGBA8);
        tex->Filtering                                    = gfx::texture::filtering::Linear;
        _sharedState.SpriteMaterial->first_pass().Texture = tex;

        gfx::image texImg {gfx::image::CreateEmpty(texImgSize, gfx::image::format::RGBA)};
        for (auto const& texDef : texDefs) {
            auto const dots {get_texture_pattern(texDef.Pattern, texDef.Size)};
            isize      i {0};
            for (i32 y {0}; y < texDef.Size.Height; ++y) {
                for (i32 x {0}; x < texDef.Size.Width; ++x) {
                    auto const idx {dots[i++]};
                    if (texDef.Transparent && *texDef.Transparent == idx) {
                        texImg.set_pixel(pen + point_i {x, y}, colors::Transparent);
                    } else {
                        texImg.set_pixel(pen + point_i {x, y}, PALETTE[idx]);
                    }
                }
            }

            auto& assTex {_textures[texDef.ID]};
            assTex.Size   = size_f {texDef.Size};
            assTex.Region = std::to_string(texDef.ID);

            tex->regions()[assTex.Region] =
                {.UVRect = {{static_cast<f32>(pen.X) / texImgSize.Width,
                             static_cast<f32>(pen.Y) / texImgSize.Height},
                            {static_cast<f32>(texDef.Size.Width) / texImgSize.Width,
                             static_cast<f32>(texDef.Size.Height) / texImgSize.Height}},
                 .Level  = 0};

            pen.X += texDef.Size.Width + (PAD * 2);
        }
        tex->update_data(texImg, 0);

        // get alpha
        static auto const extractAlpha {[](gfx::image const& img, rect_f const& uv) -> grid<u8> {
            auto const& info {img.info()};
            rect_i      rect;
            rect.Position.X  = static_cast<i32>(uv.Position.X * info.Size.Width);
            rect.Position.Y  = static_cast<i32>(uv.Position.Y * info.Size.Height);
            rect.Size.Width  = static_cast<i32>(uv.Size.Width * info.Size.Width);
            rect.Size.Height = static_cast<i32>(uv.Size.Height * info.Size.Height);

            grid<u8> retValue {rect.Size};

            for (i32 y {0}; y < rect.height(); ++y) {
                for (i32 x {0}; x < rect.width(); ++x) {
                    retValue[x, y] = img.get_pixel(rect.Position + point_i {x, y}).A;
                }
            }

            return retValue;
        }};

        for (auto& texture : _textures) {
            auto const& textureRegion {tex->regions()[texture.second.Region]};
            texture.second.Alpha = extractAlpha(texImg, textureRegion.UVRect);
        }
    } else {
        return false;
    }

    return true;
}

auto engine::create_sfx() -> bool
{
    return true;
}

void engine::set_texture(sprite* sprite, u32 texID)
{
    sprite->TexID = texID;

    auto const& texture {_textures[sprite->TexID]}; // TODO: error check
    sprite->Texture              = &texture;
    sprite->Shape->TextureRegion = texture.Region;
    if (sprite->WrapCopy) { sprite->WrapCopy->TextureRegion = texture.Region; }
}

////////////////////////////////////////////////////////////

dmd_proxy::dmd_proxy(shared_state& state)
    : _sharedState {state}
{
}
void dmd_proxy::clear()
{
    _sharedState.DMD = grid<u8> {{DMD_WIDTH, DMD_HEIGHT}, 0};
}
void dmd_proxy::blit(rect_i const& rect, string const& dotStr)
{
    auto const dots {get_texture_pattern(dotStr, rect.Size)};
    _sharedState.DMD.mutate([&](auto& dmd) { dmd.blit(rect, dots); });
}
