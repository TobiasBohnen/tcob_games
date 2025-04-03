// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Level.hpp"

namespace Rogue {

////////////////////////////////////////////////////////////

level::level()
{
    _tiles = tunneling {20, 5, 12}.generate({120, 120});
    for (i32 i {0}; i < _tiles.size(); ++i) {
        if (_tiles[i].Passable) {
            _player.Position = {i % _tiles.width(), i / _tiles.width()};
            break;
        }
    }
    _pointOfView = _player.Position;
}

void level::draw(ui::terminal& term)
{
    if (!_redraw) { return; }
    _redraw = false;

    term.clear();
    static constexpr std::array<f32, 3> DistanceFalloff {5, 3}; // TODO: player stat

    for (i32 y {0}; y < term.Size->Height; ++y) {
        for (i32 x {0}; x < term.Size->Width; ++x) {
            point_i const termPos {point_i {x, y}};
            point_i const gridPos {term_to_grid(termPos, _pointOfView)};
            if (!_tiles.contains(gridPos)) { continue; }

            auto& tile {_tiles[gridPos]};

            color bg;
            color fg;

            f64 const  distance {_player.Position.distance_to(gridPos)};
            bool const inLOS {distance <= DistanceFalloff[0] && line_of_sight(_player.Position, gridPos, _tiles)};
            if (distance > DistanceFalloff[0] || !inLOS) {
                if (tile.Seen) {
                    fg = color::Lerp(colors::Black, colors::White, 0.15f);
                    bg = colors::Black;
                } else {
                    bg = colors::Black;
                    fg = colors::Black;
                }
                tile.InSight = false;
            } else {
                if (distance > DistanceFalloff[1]) {
                    fg = color::Lerp(colors::Black, tile.ForegroundColor, 0.5f);
                    bg = color::Lerp(colors::Black, tile.BackgroundColor, 0.5f);
                } else {
                    fg = tile.ForegroundColor;
                    bg = tile.BackgroundColor;
                }
                tile.Seen    = true;
                tile.InSight = true;
            }

            if (tile.Seen || tile.InSight) {
                if (gridPos == _hoveredTile) { std::swap(fg, bg); }
                term.color_set(fg, bg);
                term.add_str(termPos, tile.Floor);
            }
        }
    }

    for (auto const& obj : _objects) {
        point_i const termPos {grid_to_term(obj.Position, _pointOfView)};
        term.color_set(obj.Color);
        term.add_str(termPos, obj.Symbol);
    }
    for (auto const& mon : _monsters) {
        point_i const termPos {grid_to_term(mon.Position, _pointOfView)};
        term.color_set(mon.Color);
        term.add_str(termPos, mon.Symbol);
    }

    term.color_set(_player.Color);
    term.add_str(grid_to_term(_player.Position, _pointOfView), "@");
}

void level::update(milliseconds deltaTime)
{
    if (_queue.empty()) { return; }
    _queueTimer -= deltaTime;
    if (_queueTimer <= 0s) {
        std::queue<std::function<bool()>> queue;
        while (!_queue.empty()) {
            auto& front {_queue.front()};
            if (!front()) { queue.emplace(front); }
            _queue.pop();
        }

        _queue.swap(queue);
        _queueTimer = 25ms;
    }
}

void level::key_down(input::key_code kc)
{
    switch (kc) {
    case input::key_code::UP: move_player(direction::Up); break;
    case input::key_code::DOWN: move_player(direction::Down); break;
    case input::key_code::LEFT: move_player(direction::Left); break;
    case input::key_code::RIGHT: move_player(direction::Right); break;
    default: break;
    }
}

void level::mouse_down(point_i pos)
{
    pos = term_to_grid(pos, _pointOfView);
    if (!_tiles.contains(pos)) { return; }

    // move player
    if (!_queue.empty()) { return; }
    ai::astar_pathfinding path;

    struct grid_path {
        grid<tile>* Parent;

        auto get_cost(point_i p) const -> u64
        {
            auto const& tile {(*Parent)[p]};
            if (!tile.Passable || !tile.Seen) { return ai::astar_pathfinding::IMPASSABLE_COST; }
            return 1;
        }
    } pathfinding {&_tiles};

    auto p {path.find_path(pathfinding, _tiles.extent(), _player.Position, pos)};
    if (p.empty()) { return; }
    _queue.emplace([p, this]() mutable -> bool {
        move_player(p.front());

        p.erase(p.begin());
        return p.empty();
    });
    _queueTimer = 25ms;
}

void level::mouse_hover(point_i pos)
{
    pos = term_to_grid(pos, _pointOfView);
    if (!_tiles.contains(pos)) { pos = {-1, -1}; }

    if (_hoveredTile != pos) {
        _hoveredTile = pos;
        _redraw      = true;
    }
}

void level::move_player(direction dir)
{
    move_player(get_target(_player.Position, dir));
}

void level::move_player(point_i pos)
{
    if (is_passable(pos)) {
        _player.Position = pos;
        end_turn();
    }
}

auto level::is_passable(point_i pos) const -> bool
{
    if (!_tiles.contains(pos)) { return false; }
    return _tiles[pos].Passable;
}

void level::end_turn()
{
    _pointOfView = _player.Position;
    _redraw      = true;
}

}
