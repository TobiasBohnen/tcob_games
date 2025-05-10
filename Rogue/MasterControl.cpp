// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "MasterControl.hpp"

#include "dungeon/Object.hpp"
#include "dungeon/Tile.hpp"
#include "ui/Renderer.hpp"

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

    random::dice<6> magicDice;
    random::dice<5> attrDice;
    profile         profile;
    profile.Magic.Earth             = magicDice.roll_n_sum(3);
    profile.Magic.Wind              = magicDice.roll_n_sum(3);
    profile.Magic.Fire              = magicDice.roll_n_sum(3);
    profile.Magic.Water             = magicDice.roll_n_sum(3);
    profile.Magic.Life              = magicDice.roll_n_sum(3);
    profile.Magic.Energy            = magicDice.roll_n_sum(3);
    profile.Attributes.Strength     = std::clamp(attrDice.roll_n_sum(2), 1, 10);
    profile.Attributes.Intelligence = std::clamp(attrDice.roll_n_sum(2), 1, 10);
    profile.Attributes.Vitality     = std::clamp(attrDice.roll_n_sum(2), 1, 10);
    profile.Attributes.Agility      = std::clamp(attrDice.roll_n_sum(2), 1, 10);
    profile.Attributes.Dexterity    = std::clamp(attrDice.roll_n_sum(2), 1, 10);
    _player                         = std::make_unique<player>(*this, profile);

    auto&       dungeon {_dungeons.emplace_back()};
    auto const& tiles {_dungeons[0].tiles()};
    for (i32 i {0}; i < tiles.count(); ++i) {
        if (tiles[i].is_passable()) {
            _player->try_move({i % tiles.width(), i / tiles.width()});
            break;
        }
    }

    auto doorObj {std::make_shared<door>(false, false)};
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

    set_view_center(_player->position());

    _player->FinishedAction.connect([this]() { end_turn(); });
    _player->FinishedPath.connect([this]() {
        _mode = mode::Move;
        mark_dirty();
    });
    _player->Log.connect([this](auto const& message) { log(message); });
}

constexpr i32 STATS_HEIGHT {10};

void master_control::draw(renderer& renderer)
{
    if (!_redraw) { return; }
    _redraw = false;

    renderer.begin();
    renderer.set_color(colors::White, colors::Black);
    // Stats
    renderer.draw_box({TERM_MAP_SIZE.Width, 0, TERM_SIZE.Width - TERM_MAP_SIZE.Width - 1, STATS_HEIGHT});
    // MFD
    renderer.draw_box({TERM_MAP_SIZE.Width, STATS_HEIGHT + 1, TERM_SIZE.Width - TERM_MAP_SIZE.Width - 1, TERM_MAP_SIZE.Height - STATS_HEIGHT - 2});
    // Detail
    renderer.draw_box({TERM_MAP_SIZE.Width, TERM_MAP_SIZE.Height, TERM_SIZE.Width - TERM_MAP_SIZE.Width - 1, TERM_SIZE.Height - TERM_MAP_SIZE.Height - 1});
    // Log
    renderer.draw_box({0, TERM_MAP_SIZE.Height, TERM_MAP_SIZE.Width - 1, TERM_SIZE.Height - TERM_MAP_SIZE.Height - 1});

    _dungeons[_currentDungeon].draw(renderer, _viewCenter, *_player);
    _player->draw(renderer, _viewCenter, _mode);

    draw_log(renderer);
    draw_mfd(renderer);
    draw_detail(renderer);
}

void master_control::update(milliseconds deltaTime, action_queue& queue)
{
    if (_newTurn) {
        _player->start_turn();
        _newTurn = false;
    }

    if (!_player->busy()) {
        handle_action_queue(queue);
    }

    _player->update(deltaTime);
}

auto master_control::current_dungeon() -> dungeon&
{
    return _dungeons[_currentDungeon];
}

void master_control::end_turn()
{
    if (_mode == mode::Move) {
        set_view_center(_player->position());
    }
    mark_dirty();
    ++_turn;
    _newTurn = true;
}

void master_control::set_view_center(point_i pos)
{
    if (pos != _viewCenter) {
        _viewCenter = pos;
        mark_dirty();
    }
}

void master_control::set_mfd_mode(mfd_mode mode)
{
    if (mode != _mfdMode) {
        _mfdMode = mode;
        mark_dirty();
    }
}

void master_control::mark_dirty()
{
    _redraw = true;
}

void master_control::log(log_message const& message)
{
    if (message.Message.empty()) { return; }

    if (!_log.empty() && _log.back().first == message.Message) {
        _log.back().second++;
    } else {
        _log.emplace_back(message.Message, 1);
    }
    mark_dirty();
}

auto master_control::rand() -> f32
{
    return _rng(0.0f, 1.0f);
}

void master_control::handle_action_queue(action_queue& queue)
{
    while (!queue.empty()) {
        auto const action {queue.front()};
        queue.pop();

        switch (action) {

        case action::LookMode:
            _mode = _mode == mode::Look ? mode::Move : mode::Look;
            if (_mode != mode::Look) { set_view_center(_player->position()); }
            mark_dirty();
            break;
        case action::UseMode:
            _mode = _mode == mode::Use ? mode::Move : mode::Use;
            mark_dirty();
            break;

        default:
            switch (_mode) {
            case mode::Look:
                if (action == action::Execute) {
                    _player->start_path(current_dungeon().find_path(_player->position(), _viewCenter));
                } else if (auto target {get_target(action, _viewCenter)}) {
                    do_look(*target);
                }
                break;
            case mode::Use:
                if (auto target {get_target(action, _player->position())}) {
                    do_interact(*target, true);
                }
                break;
            case mode::Move:
            default:
                switch (action) {
                case action::Get:              do_get(_player->position()); break;
                case action::Search:           _player->search(); break;
                case action::Center:           _mode = mode::Look; break;
                case action::MFDModeChange:    set_mfd_mode(static_cast<mfd_mode>((static_cast<i32>(_mfdMode) + 1) % MFD_COUNT)); break;
                case action::MFDModeCharacter: set_mfd_mode(mfd_mode::Character); break;
                case action::MFDModeInventory: set_mfd_mode(mfd_mode::Inventory); break;
                case action::MFDModeMonsters:  set_mfd_mode(mfd_mode::Monsters); break;
                default:
                    if (auto target {get_target(action, _player->position())}) {
                        do_move(*target);
                    }
                    break;
                }
            }
            break;
        }
    }
}

void master_control::do_get(point_i target)
{
    auto& tiles {_dungeons[_currentDungeon].tiles()};
    if (!tiles.contains(target)) { return; }

    std::vector<std::shared_ptr<object>> toBeRemove;

    auto& dungeon {_dungeons[_currentDungeon]};
    auto& tile {tiles[target]};
    for (auto& object : tile.Objects) {
        auto objItem {std::dynamic_pointer_cast<item>(object)};
        if (objItem && _player->try_pickup(objItem)) {
            toBeRemove.push_back(object);
        }
    }
    if (!toBeRemove.empty()) { // TODO: choice
        for (auto& object : toBeRemove) {
            dungeon.remove_object(*object);
        }
    } else {
        log({"nothing here"});
    }
}

auto master_control::do_move(point_i target) -> bool
{
    auto& tiles {_dungeons[_currentDungeon].tiles()};
    if (!tiles.contains(target)) { return false; }

    for (auto& object : tiles[target].Objects) {
        log({object->on_enter(*_player)});
    }

    if (!_player->try_move(target)) {
        log({"bonk"});
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
    log_message message {};

    auto& tiles {_dungeons[_currentDungeon].tiles()};

    if (tiles.contains(target)) {
        for (auto& object : tiles[target].Objects) {
            if (!object->can_interact(*_player)) { continue; }
            message = object->interact(*_player);
            break;
        }
    }

    _mode = mode::Move;

    if (message.Message.empty()) {
        if (failMessage) { log({"nothing here"}); }
        return false;
    }

    log(message);
    return true;
}

void master_control::draw_log(renderer& renderer)
{

    isize const maxMessages {TERM_SIZE.Height - TERM_MAP_SIZE.Height - 2};
    auto const  log {_log.size() >= maxMessages
                         ? std::span<log_entry const>(_log).subspan(_log.size() - maxMessages, maxMessages)
                         : std::span<log_entry const>(_log)};

    i32 y {TERM_MAP_SIZE.Height + 1};

    renderer.set_color(colors::White, colors::Black);
    for (auto const& message : log) {
        renderer.draw_cell({1, y++}, message.first); // TODO: check message length
        if (message.second > 99) {
            renderer.draw_cell(std::format(" (yes, again.)", message.second));
        } else if (message.second > 1) {
            renderer.draw_cell(std::format(" ({}x)", message.second));
        }
    }
}

void master_control::draw_mfd(renderer& renderer)
{
    i32 const x {TERM_MAP_SIZE.Width + 1};
    i32       y {STATS_HEIGHT + 1};

    switch (_mfdMode) {
    case mfd_mode::Character: {
        renderer.set_color(colors::White, colors::Black);
        renderer.draw_cell({x, y++}, "Character (TAB)");
        y++;

        _player->draw_attributes(renderer, x, y);
    } break;
    case mfd_mode::Inventory: {
        renderer.set_color(colors::White, colors::Black);
        renderer.draw_cell({x, y++}, "Inventory (TAB)");
        y++;

        _player->draw_inventory(renderer, x, y);
    } break;
    case mfd_mode::Monsters:
        renderer.set_color(colors::White, colors::Black);
        renderer.draw_cell({x, y++}, "Monsters (TAB)");
        y++;

        break;
    }
}

void master_control::draw_detail(renderer& renderer)
{
    i32 x {TERM_MAP_SIZE.Width + 1};
    i32 y {TERM_MAP_SIZE.Height + 1};

    renderer.set_color(colors::White, colors::Black);
    x = TERM_MAP_SIZE.Width + 24;
    y = TERM_MAP_SIZE.Height + 1;
    renderer.draw_cell({x, y++}, "7 8 9");
    renderer.draw_cell({x, y++}, " \\|/ ");
    renderer.draw_cell({x, y++}, "4-5-6");
    renderer.draw_cell({x, y++}, " /|\\ ");
    renderer.draw_cell({x, y++}, "1 2 3");
}

} // namespace Rogue
