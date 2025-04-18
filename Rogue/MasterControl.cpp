// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "MasterControl.hpp"

#include "level/Tile.hpp"
#include "objects/Object.hpp"

namespace Rogue {

master_control::master_control()
{
    // TODO
    _levels.emplace_back();
    auto const& tiles {_levels[0].tiles()};
    for (i32 i {0}; i < tiles.count(); ++i) {
        if (tiles[i].is_passable()) {
            _player.Position = {i % tiles.width(), i / tiles.width()};
            break;
        }
    }

    auto doorObj {std::make_shared<door>()};
    doorObj->Position = {12, 9};
    add_object(doorObj);

    auto goldObj {std::make_shared<gold>(125)};
    goldObj->Position = {13, 9};
    add_object(goldObj);

    set_view_center(_player.Position);
}

void master_control::draw(ui::terminal& term)
{
    if (!_redraw) { return; }
    _redraw = false;

    _renderer.draw({.Terminal = &term, .Level = &current_level(), .Player = &_player, .Log = &_log, .Center = _viewCenter});
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

auto master_control::current_level() -> level&
{
    return _levels[_currentLevel];
}

void master_control::end_turn()
{
    if (_mode == mode::Move) { set_view_center(_player.Position); }
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
        case action::InteractMode:
            log("direction?");
            _mode = mode::Interact;
            break;
        case action::Execute: do_execute(); break;
        case action::PickUp: {
            std::vector<std::shared_ptr<object>> toRemove;
            auto&                                tile {current_level().tiles()[_player.Position]};
            for (auto& object : tile.Objects) {
                if (object->can_pickup(_player)) {
                    log(object->pickup(_player));
                    toRemove.push_back(object);
                }
            }
            if (!toRemove.empty()) {
                end_turn();
                for (auto& object : toRemove) {
                    remove_object(object);
                }
            }
        } break;
        default:
            if (_mode == mode::Look) {
                do_look(action);
            } else if (_mode == mode::Interact) {
                do_interact(action);
            } else {
                do_move(action);
            }
            break;
        }
    }
}

void master_control::do_execute()
{
    if (_mode == mode::Look) {
        if (auto p {current_level().find_path(_player.Position, _viewCenter)}; !p.empty()) {
            _animation = ([p, this]() mutable -> bool {
                if (_player.try_move(p.front(), current_level())) {
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

void master_control::do_move(action action)
{
    std::optional<point_i> moveTarget {get_target(action, _player.Position)};
    if (!moveTarget) { return; }

    auto& level {current_level()};
    if (!level.is_passable(*moveTarget)) { //  check interact
        auto& tiles {level.tiles()};
        if (tiles.contains(*moveTarget)) {
            for (auto& object : tiles[*moveTarget].Objects) {
                if (!object->can_interact(_player)) { continue; }
                log(object->interact(_player));
                end_turn();
                break;
            }
        }
        return;
    }

    if (_player.try_move(*moveTarget, current_level())) {
        end_turn();
    } else {
        log("bonk");
    }
}

void master_control::do_look(action action)
{
    if (auto pov {get_target(action, _viewCenter)}) { set_view_center(*pov); }
}

void master_control::do_interact(action action)
{
    std::optional<point_i> interactTarget {get_target(action, _player.Position)};
    string                 message {};

    if (interactTarget) {
        auto& tiles {current_level().tiles()};

        if (tiles.contains(*interactTarget)) {
            for (auto& object : tiles[*interactTarget].Objects) {
                if (!object->can_interact(_player)) { continue; }
                message = object->interact(_player);
                break;
            }
        }
    }

    _mode = mode::Move;
    if (message.empty()) {
        message = "nothing here";
    } else {
        end_turn();
    }

    log(message);
}

void master_control::add_object(std::shared_ptr<object> const& object)
{
    auto& level {current_level()};
    level.objects().push_back(object);
    level.tiles()[object->Position].Objects.push_back(object);
    mark_dirty();
}

void master_control::remove_object(std::shared_ptr<object> const& object)
{
    auto& level {current_level()};
    helper::erase_first(level.objects(), [object](auto const& val) { return val == object; });
    helper::erase_first(level.tiles()[object->Position].Objects, [object](auto const& val) { return val == object; });
}

auto master_control::get_target(action action, point_i pos) const -> std::optional<point_i>
{
    auto static target {[](point_i pos, direction dir) -> point_i {
        switch (dir) {
        case direction::Left: return pos + point_i {-1, 0};
        case direction::Right: return pos + point_i {1, 0};
        case direction::Up: return pos + point_i {0, -1};
        case direction::Down: return pos + point_i {0, 1};
        default: return pos;
        }
    }};

    switch (action) {
    case action::MoveLeft: return target(pos, direction::Left); break;
    case action::MoveRight: return target(pos, direction::Right); break;
    case action::MoveUp: return target(pos, direction::Up); break;
    case action::MoveDown: return target(pos, direction::Down); break;
    case action::MoveLeftUp: return target(target(pos, direction::Left), direction::Up); break;
    case action::MoveRightUp: return target(target(pos, direction::Right), direction::Up); break;
    case action::MoveLeftDown: return target(target(pos, direction::Left), direction::Down); break;
    case action::MoveRightDown: return target(target(pos, direction::Right), direction::Down); break;
    default: return std::nullopt;
    }
}

}
