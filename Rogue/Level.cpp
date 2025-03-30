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

void level::draw(ui::terminal& term, point_i offset)
{
    if (!_redraw) { return; }
    _redraw = false;

    term.clear();
    constexpr static std::array<f32, 3> DistanceFalloff {6, 4, 2}; // TODO: player stat

    for (i32 y {0}; y < term.Size->Height; ++y) {
        for (i32 x {0}; x < term.Size->Width; ++x) {
            point_i const pos {x + offset.X, y + offset.Y};
            auto&         tile {_tiles[pos]};

            f64 const  distance {_player.Position.distance_to({x, y})};
            bool const inLOS {distance <= DistanceFalloff[0] && line_of_sight(_player.Position, pos)};
            if (distance > DistanceFalloff[0] || !inLOS) {
                if (tile.Seen) {
                    term.color_set(color::Lerp(colors::Black, tile.ForegroundColor, 0.25f), color::Lerp(colors::Black, tile.BackgroundColor, 0.25f));
                } else {
                    term.color_set(colors::Black, colors::Black);
                }
                tile.InSight = false;
            } else {
                if (distance > DistanceFalloff[1]) {
                    term.color_set(color::Lerp(colors::Black, tile.ForegroundColor, 0.50f), color::Lerp(colors::Black, tile.BackgroundColor, 0.50f));
                } else if (distance > DistanceFalloff[2]) {
                    term.color_set(color::Lerp(colors::Black, tile.ForegroundColor, 0.75f), color::Lerp(colors::Black, tile.BackgroundColor, 0.75f));
                } else {
                    term.color_set(tile.ForegroundColor, tile.BackgroundColor);
                }
                tile.Seen    = true;
                tile.InSight = true;
            }

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
