// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Engine.hpp"

#include "Game.hpp"
#include "objects/Die.hpp"
#include "objects/Slot.hpp"

using namespace scripting;

auto extract_alpha(gfx::image const& img, rect_f const& uv) -> grid<u8>
{
    auto const& info {img.info()};
    rect_f      rect;
    rect.Position.X  = uv.Position.X * info.Size.Width;
    rect.Position.Y  = -uv.Position.Y * info.Size.Height;
    rect.Size.Width  = uv.Size.Width * info.Size.Width;
    rect.Size.Height = -uv.Size.Height * info.Size.Height;

    grid<u8> retValue {size_i {rect.Size}};

    for (i32 y {0}; y < rect.Size.Height; ++y) {
        for (i32 x {0}; x < rect.Size.Width; ++x) {
            retValue[x, y] = img.get_pixel({static_cast<i32>(x + rect.Position.X), static_cast<i32>(y + rect.Position.Y)}).A;
        }
    }

    return retValue;
}

engine::engine(base_game& game, shared_assets& assets)
    : _game {game}
    , _assets {assets}
{
    _game.Collision.connect([&](auto const& ev) {
        call("on_collision", ev.A, ev.B);
    });
}

template <typename R>
inline auto engine::call(string const& name, auto&&... args) -> R
{
    return _table[name].as<function<R>>()(_table, this, args...);
}

void engine::run(string const& file)
{
    create_env(io::get_parent_folder(file));
    create_wrappers();

    _table = *_script.run_file<table>(file);

    create_gfx();
    create_sfx();

    call("on_setup");
}

auto engine::update(milliseconds deltaTime) -> bool
{
    if (call<bool>("can_run")) {
        call("on_run", deltaTime.count());
        return true;
    }

    return false;
}

auto engine::start_turn() -> bool
{
    if (call<bool>("can_start")) {
        _game.get_slots()->lock();
        call("on_start");
        return true;
    }

    return false;
}

void engine::create_env(string const& path)
{
    auto makeFunc {[&](auto&& func) {
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

void engine::create_wrappers()
{
    auto const normalToWorld {[this](point_f pos) -> point_f {
        auto const& bounds {*_assets.Background->Bounds};
        return {bounds.left() + (pos.X * bounds.width()), pos.Y * bounds.height()};
    }};
    auto const worldToNormal {[this](point_f pos) -> point_f {
        auto const& bounds {*_assets.Background->Bounds};
        return {(pos.X - bounds.left()) / bounds.width(), pos.Y / bounds.height()};
    }};

    // create wrappers
    auto& canvasWrapper {*_script.create_wrapper<gfx::canvas>("canvas")};
    canvasWrapper["begin_path"]   = [](gfx::canvas* canvas) { canvas->begin_path(); };
    canvasWrapper["clear"]        = [](gfx::canvas* canvas, string const& color) { canvas->clear(color::FromString(color)); };
    canvasWrapper["path_2d"]      = [](gfx::canvas* canvas, string const& path) { canvas->path_2d(*gfx::path2d::Parse(path)); };
    canvasWrapper["rect"]         = [](gfx::canvas* canvas, rect_f const& rect) { canvas->rect(rect); };
    canvasWrapper["stroke_color"] = [](gfx::canvas* canvas, string const& color) { canvas->set_stroke_style(color::FromString(color)); };
    canvasWrapper["stroke_width"] = [](gfx::canvas* canvas, f32 w) { canvas->set_stroke_width(w); };
    canvasWrapper["stroke"]       = [](gfx::canvas* canvas) { canvas->stroke(); };
    canvasWrapper["fill_color"]   = [](gfx::canvas* canvas, string const& color) { canvas->set_fill_style(color::FromString(color)); };
    canvasWrapper["fill"]         = [](gfx::canvas* canvas, std::optional<bool> enforeWinding) { canvas->fill(enforeWinding ? *enforeWinding : true); };

    auto& spriteWrapper {*_script.create_wrapper<sprite>("sprite")};
    spriteWrapper["Position"] = property {[worldToNormal](sprite* sprite) { return worldToNormal(sprite->Shape->Bounds->Position); },
                                          [normalToWorld](sprite* sprite, point_f p) { sprite->Shape->Bounds = {normalToWorld(p), sprite->Shape->Bounds->Size}; }};
    spriteWrapper["Rotation"] = property {[](sprite* sprite) { return sprite->Shape->Rotation->Value; },
                                          [](sprite* sprite, f32 p) { sprite->Shape->Rotation = degree_f {p}; }};
    spriteWrapper["Type"]     = getter {[](sprite* sprite) { return sprite->Type; }};
    spriteWrapper["Index"]    = getter {[](sprite* sprite) { return sprite->Index; }};

    auto& engineWrapper {*_script.create_wrapper<engine>("engine")};
    engineWrapper["random"]        = [](engine* engine, f32 min, f32 max) { return engine->_assets.Rng(min, max); };
    engineWrapper["randomInt"]     = [](engine* engine, i32 min, i32 max) { return engine->_assets.Rng(min, max); };
    engineWrapper["create_sprite"] = [normalToWorld](engine* engine, usize idx, table const& def) {
        auto  ptr {std::make_unique<sprite>()};
        auto* sprite {ptr.get()};
        engine->_assets.Sprites.push_back(std::move(ptr));

        sprite->Type               = def["type"].as<string>();
        sprite->Index              = idx;
        sprite->TexID              = def["texture"].as<u32>();
        sprite->IsCollisionEnabled = def["collisionEnabled"].as<bool>();

        auto const& texture {engine->_assets.Textures[sprite->TexID]};            // TODO: error check
        auto const  spritePos {normalToWorld({def["x"].as<f32>(), def["y"].as<f32>()})};
        sprite->Shape                = engine->_game.create_shape();
        sprite->Shape->Bounds        = rect_f {spritePos, size_f {texture.Size}}; // TODO: scaling
        sprite->Shape->Material      = engine->_assets.SpriteMaterial;
        sprite->Shape->TextureRegion = texture.Region;

        return sprite;
    };
    engineWrapper["create_slot"] = [normalToWorld](engine* engine, point_f pos, table const& slot) {
        slot_face face;
        slot.try_get(face.Op, "op");
        slot.try_get(face.Value, "value");
        string col;
        slot.try_get(col, "color");
        face.Color = color::FromString(col);

        engine->_game.add_slot(normalToWorld(pos), face);
    };
    engineWrapper["create_dice"] = [](engine* engine, i32 count, table const& faces) {
        std::vector<die_face> vec;
        for (i32 i {1}; i <= faces.raw_length(); ++i) {
            table            face {faces.get<table>(i).value()}; // TODO: error check
            std::vector<i32> values;
            face.try_get(values, "values");
            string col;
            face.try_get(col, "color");
            color const color {color::FromString(col)};
            for (auto const& value : values) {
                vec.emplace_back(value, color);
            }
        }
        if (vec.empty()) { return; }
        for (i32 i {0}; i < count; ++i) {
            engine->_game.add_die(vec);
        }
    };
    engineWrapper["roll_dice"]        = [](engine* engine) { engine->_game.get_dice()->roll(); };
    engineWrapper["release_dice"]     = [](engine* engine, std::vector<i32> const& slotIdx) { engine->_game.release_dice(slotIdx); };
    engineWrapper["set_dice_area"]    = [](engine* engine, rect_f const& rect) { engine->_assets.DiceArea = rect; };
    engineWrapper["is_slot_empty"]    = [](engine* engine, usize idx) { return engine->_game.get_slots()->get_slot(idx - 1)->is_empty(); };                          // TODO: error check
    engineWrapper["slot_die_value"]   = [](engine* engine, usize idx) { return engine->_game.get_slots()->get_slot(idx - 1)->current_die()->current_face().Value; }; // TODO: error check
    engineWrapper["are_slots_filled"] = [](engine* engine) { return engine->_game.get_slots()->are_filled(); };
    engineWrapper["are_slots_locked"] = [](engine* engine) { return engine->_game.get_slots()->are_locked(); };
}

struct tex_def {
    u32            ID {0};
    size_i         Size;
    function<void> Draw;
};

auto engine::create_gfx() -> bool
{
    auto const bgSize {size_i {_assets.Background->Bounds->Size}};

    // draw background
    if (function<void> func; _table.try_get(func, "draw_background")) {
        _canvas.begin_frame(bgSize, 1, 0);
        func(_table, this, &_canvas, bgSize);
        _canvas.end_frame();

        auto tex {_canvas.get_texture(0)};
        _assets.BackgroundMaterial->first_pass().Texture = tex;
        tex->regions()["default"]                        = {.UVRect = {gfx::render_texture::UVRect()},
                                                            .Level  = 0};
        _assets.Background->Material                     = _assets.BackgroundMaterial;
    } else {
        return false;
    }

    constexpr f32 PAD {2};

    // textures
    if (function<std::unordered_map<u32, table>> func; _table.try_get(func, "get_textures")) {
        auto const           texMap {func(_table, this)};
        std::vector<tex_def> texDefs;

        // get canvas size
        size_i canvasSize {0, 0};
        for (auto const& [id, texDefTable] : texMap) {
            tex_def& texDef {texDefs.emplace_back()};
            texDef.ID = id;
            size_f size;
            if (!texDefTable.try_get(size, "size")) { return false; }
            texDef.Size = size_i {size * bgSize};
            if (!texDefTable.try_get(texDef.Draw, "draw")) { return false; }

            canvasSize.Width += texDef.Size.Width + static_cast<i32>(PAD * 2);
            canvasSize.Height = std::max(canvasSize.Height, texDef.Size.Height + static_cast<i32>(PAD * 2));
        }

        canvasSize.Width += static_cast<i32>(PAD);
        canvasSize.Height += static_cast<i32>(PAD);

        // draw textures
        point_f pen {PAD, PAD};

        _canvas.begin_frame(canvasSize, 1, 1);
        auto tex {_canvas.get_texture(1)};

        for (auto const& texDef : texDefs) {
            _canvas.save();
            _canvas.translate(pen);
            texDef.Draw(&_canvas);

            auto& assTex {_assets.Textures[texDef.ID]};
            assTex.Size   = texDef.Size;
            assTex.Region = std::to_string(texDef.ID);
            tex->regions()[assTex.Region] =
                {.UVRect = {
                     {pen.X / canvasSize.Width, -pen.Y / canvasSize.Height},
                     {static_cast<f32>(texDef.Size.Width) / canvasSize.Width,
                      -static_cast<f32>(texDef.Size.Height) / canvasSize.Height}},
                 .Level = 0};

            pen.X += texDef.Size.Width + (PAD * 2);

            _canvas.restore();
        }
        _canvas.end_frame();

        // get alpha
        auto spriteImage {tex->copy_to_image(0)};
        spriteImage.flip_vertically();
        for (auto& texture : _assets.Textures) {
            auto const& textureRegion {tex->regions()[texture.second.Region]};
            texture.second.Alpha = extract_alpha(spriteImage, textureRegion.UVRect);
        }

        // setup material
        tex->Filtering                               = gfx::texture::filtering::Linear;
        _assets.SpriteMaterial->first_pass().Texture = tex;
    } else {
        return false;
    }

    return true;
}

auto engine::create_sfx() -> bool
{
    return true;
}
