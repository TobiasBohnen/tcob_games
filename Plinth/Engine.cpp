// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Engine.hpp"

#include "Player.hpp"
using namespace scripting;

engine::engine()
{
    _script.open_libraries(library::Table, library::String, library::Math);
    _script.open_addons();
    _script.Warning.connect([](script::warning_event const& ev) {
        logger::Warning(ev.Message);
    });

    create_env();
    create_wrappers();
}

engine::~engine()
{
    _script.remove_hook();
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
}

auto engine::generate_map(player const& player) -> map_t
{
    static auto read_surfaces {[](auto& cellTable, auto& cell) {
        cellTable.try_get(cell.FloorTexture, "FloorTexture");
        cellTable.try_get(cell.CeilingTexture, "CeilingTexture");
        cellTable.try_get(cell.Light, "Light");
    }};

    auto mapFunc {*_table.get<function<table>>("generate_map")};
    auto mapTable {mapFunc(&player)};

    map_t retValue;
    for (i32 i {1}; i <= mapTable.raw_length(); ++i) {
        cell  cell;
        table cellTable;
        if (!mapTable.try_get(cellTable, i)) { continue; }
        if (string type; cellTable.try_get(type, "type")) {
            if (type == "floor_cell") {
                floor_cell f {};
                read_surfaces(cellTable, f);
                cell = f;
            } else if (type == "normal_wall") {
                normal_wall w {};
                cellTable.try_get(w.Texture, "Texture");
                cell = w;
            } else if (type == "door_wall") {
                door_wall d {};
                cellTable.try_get(d.Texture, "Texture");
                cellTable.try_get(d.FrameTexture, "FrameTexture");
                cellTable.try_get(d.Orientation, "Orientation");
                read_surfaces(cellTable, d);
                cell = d;
            } else if (type == "push_wall") {
                push_wall p {};
                cellTable.try_get(p.Texture, "Texture");
                cellTable.try_get(p.PushDirection, "PushDirection");
                read_surfaces(cellTable, p);
                cell = p;
            } else if (type == "obstacle") {
                obstacle o {};
                read_surfaces(cellTable, o);
                if (auto shapes {cellTable.get<table>("Shapes")}) {
                    for (i32 j {1}; j <= shapes->raw_length(); ++j) {
                        table shapeTable;
                        if (!shapes->try_get(shapeTable, j)) { continue; }

                        obstacle_shape shape {};

                        shapeTable.try_get(shape.IsRound, "IsRound");
                        shapeTable.try_get(shape.Texture, "Texture");
                        shapeTable.try_get(shape.LocalBounds, "LocalBounds");
                        o.Shapes.push_back(shape);
                    }
                }
                cell = o;
            } else if (type == "diagonal_wall") {
                diagonal_wall dg {};
                cellTable.try_get(dg.Texture, "Texture");
                cellTable.try_get(dg.Orientation, "Orientation");
                read_surfaces(cellTable, dg);
                cell = dg;
            }
        }

        retValue[i - 1] = cell;
    }
    return retValue;
}

auto engine::update(milliseconds deltaTime) -> bool
{

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
    env["math"]["randomseed"] = nullptr;
    env["assert"]             = global["assert"];
    env["pairs"]              = global["pairs"];
    env["ipairs"]             = global["ipairs"];
    env["print"]              = global["print"];
    env["type"]               = global["type"];
    env["tonumber"]           = global["tonumber"];
    env["tostring"]           = global["tostring"];

    auto func {make_unique_closure(std::function {[this](i32 m, std::optional<i32> n) {
        if (n) { return _rng(m, *n); }
        return _rng(1, m);
    }})};
    env["math"]["random"] = func.get();
    _funcs.push_back(std::move(func));

    env["MAP_WIDTH"]  = MAP_WIDTH;
    env["MAP_HEIGHT"] = MAP_HEIGHT;

    _script.Environment = env;
}

void engine::create_wrappers()
{
}
