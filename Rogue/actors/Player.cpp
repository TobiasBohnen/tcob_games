// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Player.hpp"

#include <utility>

#include "../MasterControl.hpp"
#include "../dungeon/Dungeon.hpp"
#include "../dungeon/Object.hpp"
#include "../ui/Renderer.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

player::player(master_control& parent, profile profile)
    : actor {parent, std::move(profile)}
{
}

void player::start_turn()
{
    do_search();
}

void player::update(milliseconds deltaTime)
{
    if (_animationTimer <= 0s) { return; }

    _animationTimer -= deltaTime;
    if (_animationTimer <= 0s) {
        if (!_path.empty()) {
            if (!try_move(_path.front())) {
                _path.clear();
                FinishedPath();
            } else {
                _path.erase(_path.begin());
            }
            if (_path.empty()) {
                FinishedPath();
            } else {
                _animationTimer = ANIMATION_DELAY;
            }
        }
    }
}

auto player::busy() const -> bool
{
    return !_path.empty();
}

void player::start_path(std::vector<point_i> const& path)
{
    _path           = path;
    _animationTimer = ANIMATION_DELAY;
}

auto player::try_move(point_i pos) -> bool
{
    if (parent().current_dungeon().is_passable(pos)) {
        _position = pos;
        FinishedAction();
        return true;
    }

    return false;
}

void player::search()
{
    do_search();
    FinishedAction();
}

void player::do_search()
{
    auto& tiles {parent().current_dungeon().tiles()};
    for (auto& tile : tiles) {
        if (!tile.InSight) { continue; }
        for (auto& obj : tile.Objects) {
            Log(obj->on_search(*this));
        }
    }
}

auto player::try_pickup(std::shared_ptr<item> const& item) -> bool
{
    if (!item->can_pickup(*this)) { return false; }

    auto const message {item->pickup(*this)};

    bool const stackable {item->can_stack()};
    bool       added {false};
    auto&      stats {get_profile()};

    if (stackable) {
        for (auto& [inv_item, count] : stats.Inventory) {
            if (inv_item->type() == item->type() && inv_item->name() == item->name()) {
                count += item->amount();
                added = true;
                break;
            }
        }
    }

    if (!added) {
        stats.Inventory.emplace_back(item, item->amount());
    }

    if (!message.Message.empty()) {
        Log(message);
    }

    FinishedAction();
    return true;
}

auto player::symbol() const -> string
{
    return "@";
}

auto player::color() const -> tcob::color
{
    f32 const ratio {static_cast<f32>(get_profile().HP) / static_cast<f32>(hp_max())};
    return color::Lerp(colors::Red, colors::White, ratio);
}

auto player::light_color() const -> tcob::color
{
    return colors::White;
}

auto player::light_range() const -> f32
{
    return get_profile().VisualRange;
}

auto player::position() const -> point_i
{
    return _position;
}

auto player::count_gold() const -> i32
{
    i32 retValue {0};
    for (auto const& [item, count] : get_profile().Inventory) {
        if (item->type() == item_type::Gold) {
            retValue += count;
        }
    }
    return retValue;
}

void player::draw(renderer& renderer, point_i center, mode mode)
{
    i32 y {0};

    // symbol
    auto const termPos {grid_to_term(_position, center)};
    if (TERM_MAP_SIZE.contains(termPos)) {
        renderer.set_color(color(), colors::Black);
        renderer.draw_cell(termPos, symbol());
    }

    // stats
    auto const& stats {get_profile()};
    i32 const   x {TERM_MAP_SIZE.Width + 1};
    renderer.set_color(colors::White, colors::Black);
    renderer.draw_cell({x, y++}, std::format("{}", stats.Name));
    renderer.draw_cell({x, y++}, std::format("{}", _position));

    y++;

    auto const drawBar {[&](i32 current, i32 max) {
        renderer.draw_cell(std::format("{}/{}", current, max));
        i32 const    tickCount {static_cast<i32>(std::ceil(static_cast<f32>(current) / max * 10.f))};
        string const ticks {string(tickCount, '-') + string(10 - tickCount, ' ')};
        renderer.draw_cell({static_cast<i32>(TERM_SIZE.Width - ticks.size() - 3), renderer.current_cell().Y}, std::format("[{}]", ticks));
    }};

    // HP
    renderer.set_color(colors::Silver, colors::Black);
    renderer.draw_cell({x, y++}, "HP: ");
    renderer.set_color(colors::Red, colors::Black);
    drawBar(stats.HP, hp_max());

    // MP
    renderer.set_color(colors::Silver, colors::Black);
    renderer.draw_cell({x, y++}, "MP: ");
    renderer.set_color(colors::RoyalBlue, colors::Black);
    drawBar(stats.MP, mp_max());

    y += 2;

    switch (mode) {
    case mode::Move: {
        renderer.set_color(colors::White, colors::Blue);
        renderer.draw_cell({x, y++}, "Move");
    } break;
    case mode::Look: {
        renderer.set_color(colors::Silver, colors::Blue);
        renderer.draw_cell({x, y++}, "Look");
    } break;
    case mode::Use: {
        renderer.set_color(colors::GhostWhite, colors::Blue);
        renderer.draw_cell({x, y++}, "Use");
    } break;
    }
}

void player::draw_inventory(renderer& renderer, i32 x, i32 y)
{
    // Gold
    renderer.set_color(colors::White, colors::Black);
    renderer.draw_cell({x, y++}, "Gold:   ");
    renderer.set_color(colors::Gold, colors::Black);
    renderer.draw_cell(std::format("{:07}", count_gold()));
}

void player::draw_attributes(renderer& renderer, i32 x, i32 y)
{
    auto const& stats {get_profile()};

    renderer.set_color(colors::Silver, colors::Black);
    renderer.draw_cell({x, y++}, std::format("Strength:     {:02}", stats.Attributes.Strength));
    renderer.draw_cell({x, y++}, std::format("Agility:      {:02}", stats.Attributes.Agility));
    renderer.draw_cell({x, y++}, std::format("Dexterity:    {:02}", stats.Attributes.Dexterity));
    renderer.draw_cell({x, y++}, std::format("Intelligence: {:02}", stats.Attributes.Intelligence));
    renderer.draw_cell({x, y++}, std::format("Vitality:     {:02}", stats.Attributes.Vitality));

    y += 2;

    auto drawMagicStat {[&](string const& label, tcob::color fg, i32 value) {
        renderer.set_color(colors::Silver, colors::Black);
        renderer.draw_cell({x, y++}, label);
        renderer.set_color(fg, colors::Black);
        renderer.draw_cell(std::format("{:02}", value));
    }};

    drawMagicStat("Earth:  ", COLOR_EARTH, stats.Magic.Earth);
    drawMagicStat("Wind:   ", COLOR_WIND, stats.Magic.Wind);
    drawMagicStat("Fire:   ", COLOR_FIRE, stats.Magic.Fire);
    drawMagicStat("Water:  ", COLOR_WATER, stats.Magic.Water);
    drawMagicStat("Life:   ", COLOR_LIFE, stats.Magic.Life);
    drawMagicStat("Energy: ", COLOR_ENERGY, stats.Magic.Energy);
}

}
