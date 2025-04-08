// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Level.hpp"

#include "Layout.hpp"

namespace Rogue {

////////////////////////////////////////////////////////////

level::level()
{
    // _tiles = tunneling {20, 5, 12}.generate(clock::now().time_since_epoch().count(), {120, 120});
    // _tiles = drunkards_walk {.4f, 25000, 0.15f, 0.7f}.generate(clock::now().time_since_epoch().count(), {120, 120});
    _tiles = bsp_tree {6, 15}.generate(clock::now().time_since_epoch().count(), {120, 120});
    // _tiles = cellular_automata {30000, 4, 0.5f, 16, 500, true, 1}.generate(clock::now().time_since_epoch().count(), {120, 120});
    // _tiles = city_walls {8, 16}.generate(clock::now().time_since_epoch().count(), {120, 120});
    for (i32 i {0}; i < _tiles.count(); ++i) {
        if (tile_traits::passable(_tiles[i])) {
            _player.Position = {i % _tiles.width(), i / _tiles.width()};
            break;
        }
    }

    _viewCenter = _player.Position;
}

void level::draw(ui::terminal& term)
{
    if (!_redraw) { return; }
    _redraw = false;

    _renderer.draw(term);
}

void level::update(milliseconds deltaTime, action_queue& queue)
{
    if (_animation) {
        handle_animation(deltaTime);
    } else {
        handle_action_queue(queue);
    }

    _player.update(deltaTime);
}

auto level::is_passable(point_i pos) const -> bool
{
    if (!_tiles.contains(pos)) { return false; }
    return tile_traits::passable(_tiles[pos]);
}

void level::end_turn()
{
    if (_mode == mode::Move) {
        _viewCenter = _player.Position;
    }
    mark_dirty();
}

auto level::find_path(point_i target) const -> std::vector<point_i>
{
    ai::astar_pathfinding path;

    struct grid_path {
        grid<tile> const* Parent;

        auto get_cost(point_i p) const -> u64
        {
            auto const& tile {(*Parent)[p]};
            if (!tile_traits::passable(tile) || !tile.Seen) { return ai::astar_pathfinding::IMPASSABLE_COST; }
            return 1;
        }
    } pathfinding {&_tiles};

    return path.find_path(pathfinding, _tiles.size(), _player.Position, target);
}

void level::mark_dirty()
{
    _redraw = true;
}

void level::set_view_center(point_i pos)
{
    if (pos != _viewCenter) {
        _viewCenter = pos;
        mark_dirty();
    }
}

auto level::get_view_center() -> point_i
{
    return _viewCenter;
}

auto level::get_tiles() -> grid<tile>&
{
    return _tiles;
}

auto level::get_player() -> player&
{
    return _player;
}

auto level::get_log() const -> std::vector<string> const&
{
    return _log;
}

void level::log_message(string const& message)
{
    if (message.empty()) { return; }

    _log.push_back(message);
    mark_dirty();
}

void level::handle_animation(milliseconds deltaTime)
{
    _animationTimer -= deltaTime;
    if (_animationTimer <= 0s) {
        if (_animation()) { _animation = {}; }
        _animationTimer = AnimationDelay;
    }
}

void level::handle_action_queue(action_queue& queue)
{
    while (!queue.empty()) {
        auto const action {queue.front()};
        queue.pop();

        switch (action) {
        case action::LookMode:
            _mode = _mode == mode::Move ? mode::Look : mode::Move;
            if (_mode == mode::Move) { set_view_center(_player.Position); }
            break;
        case action::Execute: do_execute(); break;
        default:
            if (_mode == mode::Look) {
                std::optional<point_i> pov;

                switch (action) {
                case action::MoveLeft: pov = get_target(_viewCenter, direction::Left); break;
                case action::MoveRight: pov = get_target(_viewCenter, direction::Right); break;
                case action::MoveUp: pov = get_target(_viewCenter, direction::Up); break;
                case action::MoveDown: pov = get_target(_viewCenter, direction::Down); break;
                case action::MoveLeftUp: pov = get_target(get_target(_viewCenter, direction::Left), direction::Up); break;
                case action::MoveRightUp: pov = get_target(get_target(_viewCenter, direction::Right), direction::Up); break;
                case action::MoveLeftDown: pov = get_target(get_target(_viewCenter, direction::Left), direction::Down); break;
                case action::MoveRightDown: pov = get_target(get_target(_viewCenter, direction::Right), direction::Down); break;
                default: break;
                }

                if (pov) { set_view_center(*pov); }
            } else {
                std::optional<point_i> moveTarget;
                string                 message {};

                switch (action) {
                case action::MoveLeft:
                    moveTarget = get_target(_player.Position, direction::Left);
                    message    = "moved left";
                    break;
                case action::MoveRight:
                    moveTarget = get_target(_player.Position, direction::Right);
                    message    = "moved right";
                    break;
                case action::MoveUp:
                    moveTarget = get_target(_player.Position, direction::Up);
                    message    = "moved up";
                    break;
                case action::MoveDown:
                    moveTarget = get_target(_player.Position, direction::Down);
                    message    = "moved down";
                    break;
                default: break;
                }

                if (moveTarget) {
                    if (_player.try_move(*moveTarget)) {
                        end_turn();
                    } else {
                        message = "bonk";
                    }
                }
                log_message(message);
            }
            break;
        }
    }
}

void level::do_execute()
{
    if (_mode == mode::Look) {
        if (auto p {find_path(_viewCenter)}; !p.empty()) {
            _animation = ([p, this]() mutable -> bool {
                if (_player.try_move(p.front())) {
                    end_turn();
                } else {
                    return true;
                }
                p.erase(p.begin());
                if (p.empty()) {
                    _mode = mode::Move;
                    return true;
                }
                return false;
            });

            _animationTimer = AnimationDelay;
        }
    }
}

}
