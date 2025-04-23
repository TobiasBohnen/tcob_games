// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Renderer.hpp"

#include "../dungeon/Dungeon.hpp"
#include "../dungeon/Object.hpp"
#include "../monsters/Player.hpp"

namespace Rogue {

auto static term_to_grid(point_i pos, point_i center) -> point_i
{
    return pos - point_i {TERM_MAP_SIZE.Width / 2, TERM_MAP_SIZE.Height / 2} + center;
}

auto static grid_to_term(point_i pos, point_i center) -> point_i
{
    return pos + point_i {TERM_MAP_SIZE.Width / 2, TERM_MAP_SIZE.Height / 2} - center;
}

void renderer::draw(context const& ctx)
{
    ctx.Terminal->clear();

    draw_layout(ctx);

    draw_map(ctx);

    draw_player(ctx);

    draw_log(ctx);
    draw_mfd(ctx);
    draw_detail(ctx);
}

constexpr i32        STATS_HEIGHT {10};
constexpr color_pair SEEN_COLORS {color::Lerp(colors::Black, colors::White, 0.15f), colors::Black};

void renderer::draw_layout(context const& ctx)
{
    auto& term {*ctx.Terminal};

    term.color_set(colors::White, colors::Black);
    // Stats
    term.rectangle({TERM_MAP_SIZE.Width, 0, TERM_SIZE.Width - TERM_MAP_SIZE.Width - 1, STATS_HEIGHT});
    // MFD
    term.rectangle({TERM_MAP_SIZE.Width, STATS_HEIGHT + 1, TERM_SIZE.Width - TERM_MAP_SIZE.Width - 1, TERM_MAP_SIZE.Height - STATS_HEIGHT - 2});
    // Detail
    term.rectangle({TERM_MAP_SIZE.Width, TERM_MAP_SIZE.Height, TERM_SIZE.Width - TERM_MAP_SIZE.Width - 1, TERM_SIZE.Height - TERM_MAP_SIZE.Height - 1});
    // Log
    term.rectangle({0, TERM_MAP_SIZE.Height, TERM_MAP_SIZE.Width - 1, TERM_SIZE.Height - TERM_MAP_SIZE.Height - 1});
}

void renderer::draw_map(context const& ctx)
{
    auto& tiles {ctx.Dungeon->tiles()};
    auto& term {*ctx.Terminal};

    for (i32 y {0}; y < TERM_MAP_SIZE.Height; ++y) {
        for (i32 x {0}; x < TERM_MAP_SIZE.Width; ++x) {
            point_i const termPos {point_i {x, y}};
            point_i const gridPos {term_to_grid(termPos, ctx.Center)};
            if (!tiles.contains(gridPos)) { continue; }

            auto& tile {tiles[gridPos]};
            auto [fg, bg] {lighting(ctx, tile, gridPos)};
            if (gridPos == ctx.Center) {
                term.color_set(colors::White, colors::Black);
                term.add_str(termPos, "+");
            } else if (tile.Seen || tile.InSight) {
                term.color_set(fg, bg);
                term.add_str(termPos, tile.Symbol);
                draw_objects(ctx, bg, tile, gridPos);
            }
        }
    }
}

void renderer::draw_objects(context const& ctx, color bg, tile const& tile, point_i gridPos)
{
    auto& term {*ctx.Terminal};

    auto const& objects {tile.Objects};
    for (auto const& object : objects) {
        point_i const termPos {grid_to_term(gridPos, ctx.Center)};
        if (TERM_MAP_SIZE.contains(termPos)) {
            auto const colors {tile.InSight ? object->colors() : SEEN_COLORS};
            term.color_set(colors.first, colors.second == colors::Transparent ? bg : colors.second);
            term.add_str(termPos, object->symbol());
        }
    }
}

void renderer::draw_log(context const& ctx)
{
    auto& term {*ctx.Terminal};

    isize const maxMessages {TERM_SIZE.Height - TERM_MAP_SIZE.Height - 2};
    auto const  log {ctx.Log->size() >= maxMessages
                         ? std::span<log_message const>(*ctx.Log).subspan(ctx.Log->size() - maxMessages, maxMessages)
                         : std::span<log_message const>(*ctx.Log)};

    i32 y {TERM_MAP_SIZE.Height + 1};

    term.color_set(colors::White, colors::Black);
    for (auto const& message : log) {
        term.add_str({1, y++}, message.first); // TODO: check message length
        if (message.second > 99) {
            term.add_str(std::format(" (yes, again.)", message.second));
        } else if (message.second > 1) {
            term.add_str(std::format(" ({}x)", message.second));
        }
    }
}

void renderer::draw_player(context const& ctx)
{
    auto& term {*ctx.Terminal};

    i32 y {0};

    // symbol
    auto const& player {*ctx.Player};
    auto const  termPos {grid_to_term(player.Position, ctx.Center)};
    if (TERM_MAP_SIZE.contains(termPos)) {
        term.color_set(player.color(), colors::Black);
        term.add_str(termPos, player.symbol());
    }

    // stats
    auto const* stats {ctx.PlayerProfile};
    i32 const   x {TERM_MAP_SIZE.Width + 1};
    term.color_set(colors::White, colors::Black);
    term.add_str({x, y++}, std::format("{} [Lvl {}]", stats->Name, player.current_level()));
    term.add_str({x, y++}, std::format("{}", player.Position));

    y++;

    auto const drawBar {[&](i32 current, i32 max) {
        term.add_str(std::format("{}/{}", current, max));
        i32 const    tickCount {static_cast<i32>(std::ceil(static_cast<f32>(current) / max * 10.f))};
        string const ticks {string(tickCount, '-') + string(10 - tickCount, ' ')};
        term.add_str({static_cast<i32>(TERM_SIZE.Width - ticks.size() - 3), term.get_xy().Y}, std::format("[{}]", ticks));
    }};

    // HP
    term.color_set(colors::Silver, colors::Black);
    term.add_str({x, y++}, "HP: ");
    term.color_set(colors::Red, colors::Black);
    drawBar(stats->HP, player.hp_max());

    // MP
    term.color_set(colors::Silver, colors::Black);
    term.add_str({x, y++}, "MP: ");
    term.color_set(colors::RoyalBlue, colors::Black);
    drawBar(stats->MP, player.mp_max());

    y++;

    // XP
    term.color_set(colors::Silver, colors::Black);
    term.add_str({x, y++}, "XP: ");
    term.color_set(colors::DarkGray, colors::Black);
    i32 const dungeon {player.current_level()};
    i32 const xpLevel {profile::xp_required_for(dungeon)};
    drawBar(stats->XP - xpLevel, profile::xp_required_for(dungeon + 1) - xpLevel);

    y += 2;

    switch (ctx.Mode) {
    case mode::Move: {
        term.color_set(colors::White, colors::Blue);
        term.add_str({x, y++}, "Move");
    } break;
    case mode::Look: {
        term.color_set(colors::Silver, colors::Blue);
        term.add_str({x, y++}, "Look");
    } break;
    case mode::Interact: {
        term.color_set(colors::GhostWhite, colors::Blue);
        term.add_str({x, y++}, "Interact");
    } break;
    }
}

void renderer::draw_mfd(context const& ctx)
{
    auto&       term {*ctx.Terminal};
    auto const* stats {ctx.PlayerProfile};

    i32 const x {TERM_MAP_SIZE.Width + 1};
    i32       y {STATS_HEIGHT + 1};

    switch (ctx.MfdMode) {
    case mfd_mode::Character: {
        term.color_set(colors::White, colors::Black);
        term.add_str({x, y++}, "Character (TAB)");
        y++;

        term.color_set(colors::Silver, colors::Black);
        term.add_str({x, y++}, std::format("Strength:     {:02}", stats->Attributes.Strength));
        term.add_str({x, y++}, std::format("Agility:      {:02}", stats->Attributes.Agility));
        term.add_str({x, y++}, std::format("Dexterity:    {:02}", stats->Attributes.Dexterity));
        term.add_str({x, y++}, std::format("Intelligence: {:02}", stats->Attributes.Intelligence));
        term.add_str({x, y++}, std::format("Vitality:     {:02}", stats->Attributes.Vitality));
    } break;
    case mfd_mode::Inventory: {
        term.color_set(colors::White, colors::Black);
        term.add_str({x, y++}, "Inventory (TAB)");
        y++;

        // Gold
        term.color_set(colors::White, colors::Black);
        term.add_str({x, y++}, "Gold:   ");
        term.color_set(colors::Gold, colors::Black);
        term.add_str(std::format("{:07}", ctx.Player->count_gold()));

    } break;
    case mfd_mode::Magic: {
        term.color_set(colors::White, colors::Black);
        term.add_str({x, y++}, "Magic (TAB)");
        y++;
        auto drawMagicStat {[&](string const& label, color fg, i32 value) {
            term.color_set(colors::Silver, colors::Black);
            term.add_str({x, y++}, label);
            term.color_set(fg, colors::Black);
            term.add_str(std::format("{:02}", value));
        }};

        drawMagicStat("Earth:  ", COLOR_EARTH, stats->Magic.Earth);
        drawMagicStat("Wind:   ", COLOR_WIND, stats->Magic.Wind);
        drawMagicStat("Fire:   ", COLOR_FIRE, stats->Magic.Fire);
        drawMagicStat("Water:  ", COLOR_WATER, stats->Magic.Water);
        drawMagicStat("Life:   ", COLOR_LIFE, stats->Magic.Life);
        drawMagicStat("Energy: ", COLOR_ENERGY, stats->Magic.Energy);
    } break;
    case mfd_mode::Monsters:
        term.color_set(colors::White, colors::Black);
        term.add_str({x, y++}, "Monsters (TAB)");
        y++;
        break;
    }
}

void renderer::draw_detail(context const& ctx)
{
    auto& term {*ctx.Terminal};

    i32 x {TERM_MAP_SIZE.Width + 1};
    i32 y {TERM_MAP_SIZE.Height + 1};

    term.color_set(colors::White, colors::Black);
    x = TERM_MAP_SIZE.Width + 24;
    y = TERM_MAP_SIZE.Height + 1;
    term.add_str({x, y++}, "7 8 9");
    term.add_str({x, y++}, " \\|/ ");
    term.add_str({x, y++}, "4-5-6");
    term.add_str({x, y++}, " /|\\ ");
    term.add_str({x, y++}, "1 2 3");
}

auto renderer::lighting(context const& ctx, tile& tile, point_i gridPos) const -> color_pair
{
    auto& term {*ctx.Terminal};

    auto&       dungeon {*ctx.Dungeon};
    auto const& player {*ctx.Player};

    if (!dungeon.is_line_of_sight(player.Position, gridPos)) {
        tile.InSight = false;
        return tile.Seen ? SEEN_COLORS
                         : std::pair {colors::Black, colors::Black};
    }

    f32 lightFgR {0.f}, lightFgG {0.f}, lightFgB {0.f};
    f32 lightBgR {0.f}, lightBgG {0.f}, lightBgB {0.f};
    f32 totalLightFactor {0.f};

    auto const accumulateLight {[&](color const& lightColor, f32 const factor) -> void {
        lightFgR += (lightColor.R / 255.f) * factor;
        lightFgG += (lightColor.G / 255.f) * factor;
        lightFgB += (lightColor.B / 255.f) * factor;

        lightBgR += (lightColor.R / 255.f) * factor;
        lightBgG += (lightColor.G / 255.f) * factor;
        lightBgB += (lightColor.B / 255.f) * factor;

        totalLightFactor += factor;
    }};

    auto const falloff {[](f64 range, f64 distance) -> f32 {
        if (distance == 0) { return 1.f; }
        if (range == 0) { return 0.f; }

        return static_cast<f32>(std::clamp((range * range) / (distance * distance) * ((range - distance) / range), 0., 1.));
    }};

    auto const& lights {dungeon.lights()};
    for (auto const& light : lights) {
        f64 const distance {euclidean_distance(light->Position, gridPos)};
        f64 const range {light->Range};
        if (distance > range) { continue; }

        if (!dungeon.is_line_of_sight(light->Position, gridPos)) { continue; }

        f32 const factor {light->Falloff ? falloff(range, distance) : 1.0f};
        accumulateLight(light->Color, factor * light->Intensity);
    }

    f64 const distance {euclidean_distance(player.Position, gridPos)};
    f64 const range {ctx.PlayerProfile->VisualRange};
    if (distance <= range) {
        f32 const factor {falloff(range, distance)};
        accumulateLight(player.light_color(), factor);
    }

    if (totalLightFactor > 0.f) {
        tile.Seen    = true;
        tile.InSight = true;

        totalLightFactor = std::max(1.0f, totalLightFactor);
        auto const  apply {[&](u8 c, f32 light) -> u8 { return std::min(255.f, c * (light / totalLightFactor)); }};
        color const fgColor {apply(tile.ForegroundColor.R, lightFgR), apply(tile.ForegroundColor.G, lightFgG), apply(tile.ForegroundColor.B, lightFgB)};
        color const bgColor {apply(tile.BackgroundColor.R, lightBgR), apply(tile.BackgroundColor.G, lightBgG), apply(tile.BackgroundColor.B, lightBgB)};
        return color_pair {fgColor, bgColor};
    }

    tile.InSight = false;
    return std::pair {color::Lerp(colors::Black, colors::White, 0.15f), colors::Black};
}

}
