// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Engine.hpp"

#include "Die.hpp"
#include "Game.hpp"
#include "Slot.hpp"

using namespace scripting;

engine::engine(base_game& game, shared_state& state)
    : _game {game}
    , _sharedState {state}
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
    auto const& size {_game.world_size()};
    return {pos.X * size.Width, pos.Y * size.Height};
}
auto engine::world_to_normal(point_f pos) const -> point_f
{
    auto const& size {_game.world_size()};
    return {pos.X / size.Width, pos.Y / size.Height};
}

void engine::create_wrappers()
{
    create_canvas_wrapper();
    create_sprite_wrapper();
    create_slot_wrapper();
    create_engine_wrapper();
}

void engine::create_canvas_wrapper()
{
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

        sprite->IsCollisionEnabled = spriteDef["collisionEnabled"].as<bool>();
        sprite->Owner              = spriteDef;

        sprite->Shape           = engine->_game.add_shape();
        sprite->Shape->Material = engine->_spriteMaterial;

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
        string col;
        slotDef.try_get(col, "color");
        face.Color = color::FromString(col);

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
    engineWrapper["dmd"] = [](engine* engine, rect_i const& rect, std::vector<u8> const& dots) {
        engine->_sharedState.Dots.blit(rect, dots);
    };
}

struct tex_def {
    u32            ID {0};
    size_f         Size;
    function<void> Draw;
};

auto engine::create_gfx() -> bool
{
    static auto extract_alpha {[](gfx::image const& img, rect_f const& uv) -> grid<u8> {
        auto const& info {img.info()};
        rect_f      rect;
        rect.Position.X  = uv.Position.X * info.Size.Width;
        rect.Position.Y  = -uv.Position.Y * info.Size.Height;
        rect.Size.Width  = uv.Size.Width * info.Size.Width;
        rect.Size.Height = -uv.Size.Height * info.Size.Height;

        grid<u8> retValue {size_i {rect.Size}};

        for (i32 y {0}; y < retValue.size().Height; ++y) {
            for (i32 x {0}; x < retValue.size().Width; ++x) {
                retValue[{x, y}] = img.get_pixel({static_cast<i32>(x + rect.Position.X), static_cast<i32>(y + rect.Position.Y)}).A;
            }
        }

        return retValue;
    }};

    auto const bgSize {size_i {_game.world_size()}};

    // draw background
    if (function<void> func; _table.try_get(func, "draw_background")) {
        _canvas.begin_frame(bgSize, 1, 0);
        func(_table, this, &_canvas, bgSize);
        _canvas.end_frame();
        _game.set_background_tex(_canvas.get_texture(0));
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
            if (!texDefTable.try_get(texDef.Size, "size")) { return false; }
            if (!texDefTable.try_get(texDef.Draw, "draw")) { return false; }

            canvasSize.Width += texDef.Size.Width + static_cast<i32>(PAD * 2);
            canvasSize.Height = std::max(canvasSize.Height, static_cast<i32>(texDef.Size.Height + (PAD * 2)));
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

            auto& assTex {_textures[texDef.ID]};
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
        for (auto& texture : _textures) {
            auto const& textureRegion {tex->regions()[texture.second.Region]};
            texture.second.Alpha = extract_alpha(spriteImage, textureRegion.UVRect);
        }

        // setup material
        tex->Filtering                        = gfx::texture::filtering::Linear;
        _spriteMaterial->first_pass().Texture = tex;
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
}
