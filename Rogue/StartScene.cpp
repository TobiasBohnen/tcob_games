// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

#include <iomanip>

namespace Rogue {

start_scene::start_scene(game& game)
    : scene(game)
{
    window().ClearColor = colors::Black;
}

start_scene::~start_scene() = default;

void start_scene::on_start()
{
    auto& resMgr {library()};
    auto& resGrp {resMgr.create_or_get_group("rogue")};
    resGrp.mount("./rogue.zip");
    resMgr.load_all_groups();

    auto& win {window()};
    auto  windowSize {win.Size()};

    _mainForm = std::make_shared<main_menu>(resGrp, rect_i {point_i::Zero, windowSize});

    root_node()->create_child()->Entity = _mainForm;

    locate_service<gfx::render_system>().stats().reset();
}

void start_scene::on_draw_to(gfx::render_target&)
{
    _level.draw(*_mainForm->Terminal, _mapOffset);
}

void start_scene::on_update(milliseconds)
{
}

void start_scene::on_fixed_update(milliseconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    auto const& stats {locate_service<gfx::render_system>().stats()};
    stream << "avg FPS:" << stats.average_FPS();
    stream << " best FPS:" << stats.best_FPS();
    stream << " worst FPS:" << stats.worst_FPS();

    window().Title = "Roguefort |" + stream.str();
}

void start_scene::on_key_down(input::keyboard::event const& ev)
{
    switch (ev.ScanCode) {
    case input::scan_code::BACKSPACE:
        parent().pop_current_scene();
        break;
    case input::scan_code::UP: _level.move_player(direction::Up); break;
    case input::scan_code::DOWN: _level.move_player(direction::Down); break;
    case input::scan_code::LEFT: _level.move_player(direction::Left); break;
    case input::scan_code::RIGHT: _level.move_player(direction::Right); break;
    default:
        break;
    }
}

////////////////////////////////////////////////////////////

void level::draw(terminal& term, point_i offset)
{
    if (!_redraw) { return; }
    _redraw = false;

    term.clear();

    for (i32 y {0}; y < term.Size->Height; ++y) {
        for (i32 x {0}; x < term.Size->Width; ++x) {
            point_i const pos {x + offset.X, y + offset.Y};
            auto const&   tile {_tiles[pos]};
            term.color_set(tile.Color);
            term.add_str(pos, tile.Floor);
        }
    }

    for (auto const& obj : _objects) {
        point_i const pos {obj.Position + offset};
        term.color_set(obj.Color);
        term.add_str(pos, obj.Symbol);
    }
    for (auto const& mon : _monsters) {
        point_i const pos {mon.Position + offset};
        term.color_set(mon.Color);
        term.add_str(pos, mon.Symbol);
    }

    term.color_set(_player.Color);
    term.add_str(_player.Position + offset, "@");
}

void level::end_turn()
{
    _redraw = true;
}

level::level()
{

    string testLevel =
        "######################                  "
        "#....................#                  "
        "#....................##############     "
        "#.................................#     "
        "#....................##############     "
        "#....................#                  "
        "######################                  ";
    for (i32 y {0}; y < 7; ++y) {
        for (i32 x {0}; x < 40; ++x) {
            _tiles[{x, y}].Floor    = testLevel[x + y * 40];
            _tiles[{x, y}].Passable = testLevel[x + y * 40] == '.';
        }
    }

    _player.Position = {4, 4};
    _monsters.push_back({.Symbol = "T", .Color = colors::Red, .Position = {2, 2}});
    _objects.push_back({.Symbol = "$", .Color = colors::Gold, .Position = {8, 5}});
}

void level::move_player(direction dir)
{
    auto const target(_player.get_target(dir));
    if (can_move_to(target)) {
        _player.move_to(target);
        end_turn();
    }
}

auto level::can_move_to(point_i pos) const -> bool
{
    if (!_tiles.contains(pos)) { return false; }
    return _tiles[pos].Passable;
}

////////////////////////////////////////////////////////////

auto player::get_target(direction dir) const -> point_i
{
    switch (dir) {
    case direction::Left: return Position + point_i {-1, 0};
    case direction::Right: return Position + point_i {1, 0};
    case direction::Up: return Position + point_i {0, -1};
    case direction::Down: return Position + point_i {0, 1};
    default: break;
    }

    return Position;
}

void player::move_to(point_i pos)
{
    Position = pos;
}

}
