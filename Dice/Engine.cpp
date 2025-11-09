// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Engine.hpp"

#include "Game.hpp"
#include "objects/Die.hpp"
#include "objects/Slot.hpp"

using namespace scripting;

engine::engine(base_game& game, script_assets& assets)
    : _game {game}
    , _assets {assets}
{
}

template <typename R = void>
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

void engine::update(milliseconds deltaTime)
{
    if (call<bool>("can_process_turn")) {
        call("on_process_turn", deltaTime.count());
    }
}

auto engine::end_turn() -> bool
{
    if (call<bool>("can_end_turn")) {
        call("on_end_turn");
        return true;
    }

    return false;
}

void engine::create_env(string const& path)
{
    auto make_func {[&](auto&& func) {
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
    env["require"] = make_func([env, path, this](string const& package) {
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
    auto convert_point {[this](point_f pos) -> point_f {
        return {_game.bounds().left() + (pos.X * _game.bounds().Size.Width),
                pos.Y * _game.bounds().Size.Height};
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
    spriteWrapper["position"] = [convert_point](sprite* sprite, point_f p) {
        sprite->Shape->Bounds = {convert_point(p), sprite->Shape->Bounds->Size};
    };
    spriteWrapper["rotation"] = [](sprite* sprite, f32 p) {
        sprite->Shape->Rotation = degree_f {p};
    };

    auto& slotsWrapper {*_script.create_wrapper<slots>("slots")};
    slotsWrapper["slot_is_empty"]  = [](slots* slots, usize idx) { return slots->get_slot(idx)->empty(); };                             // TODO: error check
    slotsWrapper["slot_die_value"] = [](slots* slots, usize idx) { return slots->get_slot(idx)->current_die()->current_face().Value; }; // TODO: error check
    slotsWrapper["is_complete"]    = [](slots* slots) { return slots->is_complete(); };
    slotsWrapper["lock"]           = [](slots* slots) { slots->lock(); };
    slotsWrapper["unlock"]         = [](slots* slots) { slots->unlock(); };
    slotsWrapper["are_locked"]     = [](slots* slots) { slots->are_locked(); };

    auto& engineWrapper {*_script.create_wrapper<engine>("engine")};
    engineWrapper["random"]        = [](engine* engine, f32 min, f32 max) { return engine->_game.random(min, max); };
    engineWrapper["create_sprite"] = [convert_point](engine* engine, point_f center, u32 tex) {
        auto  ptr {std::make_unique<sprite>()};
        auto* sprite {ptr.get()};
        engine->_assets.Sprites.push_back(std::move(ptr));

        auto const& texture {engine->_assets.Textures[tex]}; // TODO: error check
        auto const  spriteCenter {convert_point(center)};
        sprite->Shape         = engine->_game.create_shape();
        sprite->Shape->Bounds = rect_f {
            point_f {spriteCenter.X - (texture.Size.Width / 2), spriteCenter.Y - (texture.Size.Width / 2)},
            size_f {texture.Size}};
        sprite->Shape->Material      = engine->_assets.SpriteMaterial;
        sprite->Shape->TextureRegion = texture.Region;
        return sprite;
    };
    engineWrapper["create_slots"] = [](engine* engine, table const& slots) { };
    engineWrapper["create_dice"]  = [](engine* engine, table const& dice) { };
    engineWrapper["roll_dice"]    = [](engine* engine) { engine->_game.roll(); };
}

auto engine::create_gfx() -> bool
{
    table gfxTable;
    if (!_table.try_get(gfxTable, "Gfx")) { return false; }

    if (function<void> func; gfxTable.try_get(func, "draw_background")) { // background
        auto const canvasSize {size_i {_game.bounds().Size}};
        _canvas.begin_frame(canvasSize, 1, 0);
        func(this, &_canvas, canvasSize);
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

    if (function<std::unordered_map<u32, table>> func; gfxTable.try_get(func, "get_textures")) { // textures
        auto const texMap {func(gfxTable, this)};
        struct tex_def {
            u32            ID {0};
            size_i         Size;
            function<void> Draw;
        };
        std::vector<tex_def> texDefs;

        size_i canvasSize {0, 0};
        for (auto const& [id, texDefTable] : texMap) {
            tex_def& texDef {texDefs.emplace_back()};
            texDef.ID = id;
            if (!texDefTable.try_get(texDef.Size, "Size")) { return false; }
            if (!texDefTable.try_get(texDef.Draw, "Draw")) { return false; }

            canvasSize.Width += texDef.Size.Width + static_cast<i32>(PAD * 2);
            canvasSize.Height = std::max(canvasSize.Height, texDef.Size.Height + static_cast<i32>(PAD * 2));
        }

        canvasSize.Width += static_cast<i32>(PAD);
        canvasSize.Height += static_cast<i32>(PAD);

        point_f pen {PAD, PAD};

        _canvas.begin_frame(canvasSize, 1, 1);
        auto tex {_canvas.get_texture(1)};

        for (auto const& texDef : texDefs) {
            _canvas.save();
            _canvas.translate(pen);
            texDef.Draw(&_canvas);

            _assets.Textures[texDef.ID].Size   = texDef.Size;
            _assets.Textures[texDef.ID].Region = std::to_string(texDef.ID);
            tex->regions()[_assets.Textures[texDef.ID].Region] =
                {.UVRect = {
                     {pen.X / canvasSize.Width, -pen.Y / canvasSize.Height},
                     {static_cast<f32>(texDef.Size.Width) / canvasSize.Width,
                      -static_cast<f32>(texDef.Size.Height) / canvasSize.Height}},
                 .Level = 0};

            pen.X += texDef.Size.Width + (PAD * 2);

            _canvas.restore();
        }
        _canvas.end_frame();

        _assets.SpriteMaterial->first_pass().Texture = tex;
    } else {
        return false;
    }

    return true;
}

auto engine::create_sfx() -> bool
{
    if (table sfxTable; _table.try_get(sfxTable, "Sfx")) {
    }

    return true;
}
