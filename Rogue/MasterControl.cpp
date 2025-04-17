// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "MasterControl.hpp"

#include "level/Tile.hpp"

namespace Rogue {

master_control::master_control()
{
    // TODO
    auto const& tiles {_level.get_tiles()};
    for (i32 i {0}; i < tiles.count(); ++i) {
        if (tile_traits::passable(tiles[i])) {
            _player.Position = {i % tiles.width(), i / tiles.width()};
            break;
        }
    }

    set_view_center(_player.Position);
}

void master_control::draw(ui::terminal& term)
{
    if (!_redraw) { return; }
    _redraw = false;

    _renderer.draw(term, _viewCenter);
}

void master_control::update(milliseconds deltaTime, action_queue& queue)
{
    if (_animation) {
        handle_animation(deltaTime);
    } else {
        handle_action_queue(queue);
    }

    _player.update(deltaTime);
}

auto master_control::get_level() -> level&
{
    return _level;
}

void master_control::end_turn()
{
    if (_mode == mode::Move) {
        set_view_center(_player.Position);
    }
    mark_dirty();
}

void master_control::set_view_center(point_i pos)
{
    if (pos != _viewCenter) {
        _viewCenter = pos;
        mark_dirty();
    }
}

void master_control::mark_dirty()
{
    _redraw = true;
}

auto master_control::get_player() -> player&
{
    return _player;
}

auto master_control::get_log() const -> std::vector<log_message> const&
{
    return _log;
}

void master_control::log(string const& message)
{
    if (message.empty()) { return; }

    if (!_log.empty() && _log.back().first == message) {
        _log.back().second++;
    } else {
        _log.emplace_back(message, 1);
    }
    mark_dirty();
}

void master_control::handle_animation(milliseconds deltaTime)
{
    _animationTimer -= deltaTime;
    if (_animationTimer <= 0s) {
        if (_animation()) { _animation = {}; }
        _animationTimer = AnimationDelay;
    }
}

void master_control::handle_action_queue(action_queue& queue)
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
                case action::MoveLeftUp:
                    moveTarget = get_target(get_target(_player.Position, direction::Left), direction::Up);
                    message    = "moved left up";
                    break;
                case action::MoveRightUp:
                    moveTarget = get_target(get_target(_player.Position, direction::Right), direction::Up);
                    message    = "moved right up";
                    break;
                case action::MoveLeftDown:
                    moveTarget = get_target(get_target(_player.Position, direction::Left), direction::Down);
                    message    = "moved left down";
                    break;
                case action::MoveRightDown:
                    moveTarget = get_target(get_target(_player.Position, direction::Right), direction::Down);
                    message    = "moved right down";
                    break;
                default: break;
                }

                if (moveTarget) {
                    if (_player.try_move(*moveTarget, _level)) {
                        end_turn();
                    } else {
                        message = "bonk";
                    }
                }
                log(message);
            }
            break;
        }
    }
}

void master_control::do_execute()
{
    if (_mode == mode::Look) {
        if (auto p {_level.find_path(_player.Position, _viewCenter)}; !p.empty()) {
            _animation = ([p, this]() mutable -> bool {
                if (_player.try_move(p.front(), _level)) {
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
