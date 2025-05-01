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
        case direction::Left:  return pos + point_i {-1, 0};
        case direction::Right: return pos + point_i {1, 0};
        case direction::Up:    return pos + point_i {0, -1};
        case direction::Down:  return pos + point_i {0, 1};
        default:               return pos;
        }
    }};

    switch (action) {
    case action::Center:    return pos;
    case action::Left:      return target(pos, direction::Left);
    case action::Right:     return target(pos, direction::Right);
    case action::Up:        return target(pos, direction::Up);
    case action::Down:      return target(pos, direction::Down);
    case action::LeftUp:    return target(target(pos, direction::Left), direction::Up);
    case action::RightUp:   return target(target(pos, direction::Right), direction::Up);
    case action::LeftDown:  return target(target(pos, direction::Left), direction::Down);
    case action::RightDown: return target(target(pos, direction::Right), direction::Down);
    default:                return std::nullopt;
    }
}

master_control::master_control()
{
    // TODO
    auto&       dungeon {_dungeons.emplace_back()};
    auto const& tiles {_dungeons[0].tiles()};
    for (i32 i {0}; i < tiles.count(); ++i) {
        if (tiles[i].is_passable()) {
            _player.Position = {i % tiles.width(), i / tiles.width()};
            break;
        }
    }

    auto doorObj {std::make_shared<door>(false)};
    doorObj->Position = {12, 9};
    dungeon.add_object(doorObj);

    auto goldObj0 {std::make_shared<gold>(125)};
    goldObj0->Position = {13, 9};
    dungeon.add_object(goldObj0);

    auto goldObj1 {std::make_shared<gold>(256)};
    goldObj1->Position = {14, 11};
    dungeon.add_object(goldObj1);

    auto trapObj1 {std::make_shared<trap>(false, trap_type::Spikes)};
    trapObj1->Position = {15, 11};
    dungeon.add_object(trapObj1);

    set_view_center(_player.Position);

    _player.EndTurn.connect([this]() { end_turn(); });
    _player.FinishedPath.connect([this]() {
        _mode = mode::Move;
        mark_dirty();
    });
}

void master_control::draw(ui::terminal& term)
{
    if (!_redraw) { return; }
    _redraw = false;

    _renderer.draw({.Terminal = &term,
                    .Dungeon  = &_dungeons[_currentDungeon],
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

auto master_control::current_dungeon() const -> dungeon const&
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
        default:
            if (_mode == mode::Look) {
                if (auto target {get_target(action, _viewCenter)}) {
                    do_look(*target);
                }
            } else if (_mode == mode::Interact) {
                if (auto target {get_target(action, _player.Position)}) {
                    do_interact(*target, true);
                }
            } else {
                switch (action) {
                case action::PickUp:
                    do_pickup();
                    break;
                case action::Center:
                    _mode = mode::Look;
                    break;
                case action::MFDModeChange:
                    _mfdMode = static_cast<mfd_mode>((static_cast<i32>(_mfdMode) + 1) % MFD_COUNT);
                    mark_dirty();
                    break;
                case action::MFDModeCharacter:
                    _mfdMode = mfd_mode::Character;
                    mark_dirty();
                    break;
                case action::MFDModeInventory:
                    _mfdMode = mfd_mode::Inventory;
                    mark_dirty();
                    break;
                case action::MFDModeMagic:
                    _mfdMode = mfd_mode::Magic;
                    mark_dirty();
                    break;
                case action::MFDModeMonsters:
                    _mfdMode = mfd_mode::Monsters;
                    mark_dirty();
                    break;
                default:
                    if (auto target {get_target(action, _player.Position)}) {
                        do_move(*target);
                    }
                }
            }
            break;
        }
    }
}

void master_control::do_execute()
{
    if (_mode == mode::Look) {
        _player.start_path(current_dungeon().find_path(_player.Position, _viewCenter));
    }
}

void master_control::do_pickup()
{
    std::vector<std::shared_ptr<object>> toRemove;

    auto& dungeon {_dungeons[_currentDungeon]};
    auto& tile {dungeon.tiles()[_player.Position]};
    for (auto& object : tile.Objects) {
        auto objItem {std::dynamic_pointer_cast<item>(object)};
        if (objItem && _player.try_pickup(objItem)) {
            toRemove.push_back(object);
        }
    }
    if (!toRemove.empty()) { // TODO: choice
        for (auto& object : toRemove) {
            dungeon.remove_object(object);
        }
    } else {
        log("nothing here");
    }
}

auto master_control::do_move(point_i target) -> bool
{
    auto& tiles {_dungeons[_currentDungeon].tiles()};
    if (!tiles.contains(target)) { return false; }

    for (auto& object : tiles[target].Objects) {
        log(object->on_enter(_player));
    }

    if (!_player.try_move(target)) {
        log("bonk");
        return false;
    }
    return true;
}

void master_control::do_look(point_i target)
{
    set_view_center(target);
}

auto master_control::do_interact(point_i target, bool failMessage) -> bool
{
    string message {};

    auto& tiles {_dungeons[_currentDungeon].tiles()};

    if (tiles.contains(target)) {
        for (auto& object : tiles[target].Objects) {
            if (!object->can_interact(_player)) { continue; }
            message = object->interact(_player);
            break;
        }
    }

    _mode = mode::Move;

    if (message.empty()) {
        if (failMessage) { log("nothing here"); }
        return false;
    }

    log(message);
    return true;
}

} // namespace Rogue
