// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Level.hpp"

namespace Rogue {

////////////////////////////////////////////////////////////

level::level()
{
    _tiles = tunneling {12, 5, 12}.generate({80, 24});
    for (i32 i {0}; i < _tiles.size(); ++i) {
        if (_tiles[i].Passable) {
            _player.Position = {i % _tiles.width(), i / _tiles.width()};
            break;
        }
    }
}

void level::draw(ui::terminal& term)
{
    if (!_redraw) { return; }
    _redraw = false;

    term.clear();
    static constexpr std::array<f32, 3> DistanceFalloff {6, 4, 2}; // TODO: player stat

    for (i32 y {0}; y < term.Size->Height; ++y) {
        for (i32 x {0}; x < term.Size->Width; ++x) {
            point_i const pos {x + _termOffset.X, y + _termOffset.Y};
            auto&         tile {_tiles[pos]};

            color bg;
            color fg;

            f64 const  distance {_player.Position.distance_to({x, y})};
            bool const inLOS {distance <= DistanceFalloff[0] && line_of_sight(_player.Position, pos)};
            if (distance > DistanceFalloff[0] || !inLOS) {
                if (tile.Seen) {
                    fg = color::Lerp(colors::Black, tile.ForegroundColor, 0.15f);
                    bg = color::Lerp(colors::Black, tile.BackgroundColor, 0.15f);
                } else {
                    bg = colors::Black;
                    fg = colors::Black;
                }
                tile.InSight = false;
            } else {
                if (distance > DistanceFalloff[1]) {
                    fg = color::Lerp(colors::Black, tile.ForegroundColor, 0.50f);
                    bg = color::Lerp(colors::Black, tile.BackgroundColor, 0.50f);
                } else if (distance > DistanceFalloff[2]) {
                    fg = color::Lerp(colors::Black, tile.ForegroundColor, 0.75f);
                    bg = color::Lerp(colors::Black, tile.BackgroundColor, 0.75f);
                } else {
                    fg = tile.ForegroundColor;
                    bg = tile.BackgroundColor;
                }
                tile.Seen    = true;
                tile.InSight = true;
            }

            if (tile.Seen || tile.InSight) {
                if (point_i {x, y} == _hoveredTile) { std::swap(fg, bg); }
                term.color_set(fg, bg);
                term.add_str(pos, tile.Floor);
            }
        }
    }

    for (auto const& obj : _objects) {
        point_i const pos {obj.Position + _termOffset};
        term.color_set(obj.Color);
        term.add_str(pos, obj.Symbol);
    }
    for (auto const& mon : _monsters) {
        point_i const pos {mon.Position + _termOffset};
        term.color_set(mon.Color);
        term.add_str(pos, mon.Symbol);
    }

    term.color_set(_player.Color);
    term.add_str(_player.Position + _termOffset, "@");
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
    pos += _termOffset;
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
    _queue.emplace([=, this]() mutable -> bool {
        _player.move_to(p.front());
        end_turn();
        p.erase(p.begin());
        return p.empty();
    });
    _queueTimer = 25ms;
}

void level::mouse_hover(point_i pos)
{
    pos += _termOffset;
    if (!_tiles.contains(pos)) { pos = {-1, -1}; }

    if (_hoveredTile != pos) {
        _hoveredTile = pos;
        _redraw      = true;
    }
}

void level::end_turn()
{
    _redraw = true;
}

auto level::line_of_sight(point_i start, point_i end) const -> bool
{
    auto [x0, y0] {start};
    auto const [x1, y1] {end};
    i32 const dx {std::abs(x1 - x0)};
    i32 const dy {std::abs(y1 - y0)};
    i32 const sx {(x0 < x1) ? 1 : -1};
    i32 const sy {(y0 < y1) ? 1 : -1};
    i32       err {dx - dy};

    for (;;) {
        if (x0 == x1 && y0 == y1) { break; }

        i32 const e2 {2 * err};
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }

        point_i const pos {x0, y0};
        if (pos != end && !_tiles[pos].Passable) { return false; }
    }
    return true;
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
