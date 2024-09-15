// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Player.hpp"

#include "assets/GameAssets.hpp"

namespace stn {

/////////////////////////////////////////////////////

player::player(tilemap& map, canvas& canvas, assets& assets)
    : _map {map}
    , _canvas {canvas}
    , _assets {assets}
    , _tiles {_assets.get_texture("player"), _assets.get_tiles_def("player")}
{
    _position = {3, 3};
    set_direction(direction::Down, 0);
}

void player::update(milliseconds deltaTime)
{
    if (_move) { _move->update(deltaTime); }
}

void player::draw(gfx::canvas& canvas)
{
    point_f const offset {_map.get_offset()};
    point_f const p {(_position.X - offset.X) * TILE_SIZE_F.Width, (_position.Y - offset.Y) * TILE_SIZE_F.Height};

    canvas.set_fill_style(colors::White);
    canvas.draw_image(_tiles.Texture, _tiles.Set[_tile].as<tile>().Texture, {p, TILE_SIZE_F});

    canvas.set_fill_style(COLOR0);
    canvas.begin_path();
    canvas.rect({{0, 0}, {CANVAS_SIZE_F.Width, TILE_SIZE_F.Height}});
    canvas.fill();

    canvas.set_fill_style(colors::White);
    for (i32 i {0}; i < _health; ++i) {
        canvas.draw_image(_tiles.Texture, _tiles.Set["health"].as<tile>().Texture, {{TILE_SIZE_F.Width / 2 * i, 0.f}, TILE_SIZE_F / 2});
    }
    for (i32 i {0}; i < _sanity; ++i) {
        canvas.draw_image(_tiles.Texture, _tiles.Set["sanity"].as<tile>().Texture, {{TILE_SIZE_F.Width / 2 * i, TILE_SIZE_F.Height / 2}, TILE_SIZE_F / 2});
    }
}

void player::move(direction dir, bool dirOnly)
{
    if (_move && _move->get_status() == playback_status::Running) { return; }

    if (!dirOnly) {
        point_i move;
        switch (dir) {
        case direction::Left: move = point_i {-1, 0}; break;
        case direction::Right: move = point_i {1, 0}; break;
        case direction::Up: move = point_i {0, -1}; break;
        case direction::Down: move = point_i {0, 1}; break;
        default: break;
        }

        point_f const start {_position};
        point_f const end {_position + move};
        if (!_map.check_solid(point_i {end})) {
            _move = make_unique_tween<smootherstep_tween<point_f>>(500ms, start, end);
            _move->Value.Changed.connect([&](auto&& val) {
                _canvas.request_draw();
                std::array<i32, 5> const phases {0, 1, 0, 2, 0};
                set_direction(_direction, phases[static_cast<i32>(_move->get_progress() * 4)]);

                _position = val;
                _map.set_offset(_position);
            });
            _move->Finished.connect([&] { set_direction(_direction, 0); });
            _move->Interval = 10ms;
            _move->start();
        }
    }

    set_direction(dir, 0);
}

void player::set_direction(direction dir, i32 phase)
{
    point_i move;
    switch (dir) {
    case direction::Left: _tile = "player-left" + std::to_string(phase); break;
    case direction::Right: _tile = "player-right" + std::to_string(phase); break;
    case direction::Up: _tile = "player-up" + std::to_string(phase); break;
    case direction::Down: _tile = "player-down" + std::to_string(phase); break;
    default: break;
    }

    _direction = dir;
    _canvas.request_draw();
}

}
