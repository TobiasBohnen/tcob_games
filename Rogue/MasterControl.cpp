// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "MasterControl.hpp"

#include "dungeon/Object.hpp"
#include "dungeon/Tile.hpp"

namespace Rogue {

auto static get_target(action action, point_i pos) -> std::optional<point_i>
{
    auto static target {[](point_i pos, direction dir) -> point_i {
        switch (dir) {
        case direction::Left:
            return pos + point_i {-1, 0};
        case direction::Right:
            return pos + point_i {1, 0};
        case direction::Up:
            return pos + point_i {0, -1};
        case direction::Down:
            return pos + point_i {0, 1};
        default:
            return pos;
        }
    }};

    switch (action) {
    case action::Center:
        return pos;
    case action::MoveLeft:
        return target(pos, direction::Left);
    case action::MoveRight:
        return target(pos, direction::Right);
    case action::MoveUp:
        return target(pos, direction::Up);
    case action::MoveDown:
        return target(pos, direction::Down);
    case action::MoveLeftUp:
        return target(target(pos, direction::Left), direction::Up);
    case action::MoveRightUp:
        return target(target(pos, direction::Right), direction::Up);
    case action::MoveLeftDown:
        return target(target(pos, direction::Left), direction::Down);
    case action::MoveRightDown:
        return target(target(pos, direction::Right), direction::Down);
    default:
        return std::nullopt;
    }
}

master_control::master_control()
{
    // TODO
    _dungeons.emplace_back();
    auto const& tiles {_dungeons[0].tiles()};
    for (i32 i {0}; i < tiles.count(); ++i) {
        if (tiles[i].is_passable()) {
            _player.Position = {i % tiles.width(), i / tiles.width()};
            break;
        }
    }

    auto doorObj {std::make_shared<door>(false)};
    doorObj->Position = {12, 9};
    current_dungeon().add_object(doorObj);

    auto goldObj0 {std::make_shared<gold>(125)};
    goldObj0->Position = {13, 9};
    current_dungeon().add_object(goldObj0);

    auto goldObj1 {std::make_shared<gold>(256)};
    goldObj1->Position = {14, 11};
    current_dungeon().add_object(goldObj1);

    auto trapObj1 {std::make_shared<trap>(false, trap_type::Spikes)};
    trapObj1->Position = {15, 11};
    current_dungeon().add_object(trapObj1);

    set_view_center(_player.Position);

    _player.EndTurn.connect([this]() { end_turn(); });
    _player.FinishedPath.connect([this]() {
        _mode = mode::Move;
        mark_dirty();
    });
}

void master_control::draw(ui::terminal& term)
{
    if (!_redraw) {
        return;
    }
    _redraw = false;

    _renderer.draw({.Terminal = &term,
                    .Dungeon  = &current_dungeon(),
                    .Player   = &_player,
                    .Log      = &_log,
                    .Mode     = _mode,
                    .MfdMode  = _mfdMode,
                    .Center   = _viewCenter});
}

void master_control::update(milliseconds deltaTime, action_queue& queue)
{
    if (!_player.busy()) {
        handle_action_queue(queue);
    }

    _player.update(deltaTime);
}

auto master_control::current_dungeon() -> dungeon&
{
    return _dungeons[_currentDungeon];
}

void master_control::end_turn()
{
    if (_mode == mode::Move) {
        set_view_center(_player.Position);
    }
    mark_dirty();
    ++_turn;
}

void master_control::set_view_center(point_i pos)
{
    if (pos != _viewCenter) {
        _viewCenter = pos;
        mark_dirty();
    }
}

void master_control::mark_dirty() { _redraw = true; }

void master_control::log(string const& message)
{
    if (message.empty()) {
        return;
    }

    if (!_log.empty() && _log.back().first == message) {
        _log.back().second++;
    } else {
        _log.emplace_back(message, 1);
    }
    mark_dirty();
}

void master_control::handle_action_queue(action_queue& queue)
{
    while (!queue.empty()) {
        auto const action {queue.front()};
        queue.pop();

        switch (action) {
        case action::LookMode:
            _mode = _mode == mode::Move ? mode::Look : mode::Move;
            if (_mode == mode::Move) {
                set_view_center(_player.Position);
            }
            mark_dirty();
            break;
        case action::InteractMode:
            _mode = _mode == mode::Move ? mode::Interact : mode::Move;
            mark_dirty();
            break;
        case action::Execute:
            do_execute();
            break;
        case action::MFDModeChange:
            _mfdMode =
                static_cast<mfd_mode>((static_cast<i32>(_mfdMode) + 1) % MFD_COUNT);
            mark_dirty();
            break;
        case action::PickUp:
            do_pickup();
            break;
        default:
            if (_mode == mode::Look) {
                do_look(action);
            } else if (_mode == mode::Interact) {
                do_interact(get_target(action, _player.Position), true);
            } else {
                if (action == action::Center) {
                    _mode = mode::Look;
                }
                do_move(action);
            }
            break;
        }
    }
}

void master_control::do_execute()
{
    if (_mode == mode::Look) {
        _player.start_path(
            current_dungeon().find_path(_player.Position, _viewCenter));
    }
}

void master_control::do_pickup()
{
    std::vector<std::shared_ptr<object>> toRemove;

    auto& tile {current_dungeon().tiles()[_player.Position]};
    for (auto& object : tile.Objects) {
        auto objItem {std::dynamic_pointer_cast<item>(object)};
        if (objItem && _player.try_pickup(objItem)) {
            toRemove.push_back(object);
        }
    }
    if (!toRemove.empty()) { // TODO: choice
        for (auto& object : toRemove) {
            current_dungeon().remove_object(object);
        }
    } else {
        log("nothing here");
    }
}

auto master_control::do_move(action action) -> bool
{
    std::optional<point_i> moveTarget {get_target(action, _player.Position)};
    if (!moveTarget) {
        return false;
    }

    auto& tiles {current_dungeon().tiles()};
    if (!tiles.contains(*moveTarget)) {
        return false;
    }

    for (auto& object : tiles[*moveTarget].Objects) {
        log(object->on_enter(_player));
    }

    if (!_player.try_move(*moveTarget)) {
        log("bonk");
        return false;
    }

    return true;
}

void master_control::do_look(action action)
{
    if (auto pov {get_target(action, _viewCenter)}) {
        set_view_center(*pov);
    }
}

auto master_control::do_interact(std::optional<point_i> interactTarget,
                                 bool                   failMessage) -> bool
{
    string message {};

    if (!interactTarget) {
        return false;
    }
    auto& tiles {current_dungeon().tiles()};

    if (tiles.contains(*interactTarget)) {
        for (auto& object : tiles[*interactTarget].Objects) {
            if (!object->can_interact(_player)) {
                continue;
            }
            message = object->interact(_player);
            break;
        }
    }

    _mode = mode::Move;

    if (message.empty()) {
        if (failMessage) {
            log("nothing here");
        }
        return false;
    }

    log(message);
    return true;
}

} // namespace Rogue
