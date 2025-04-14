// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Renderer.hpp"

#include "Level.hpp"
#include "MasterControl.hpp"

namespace Rogue {

renderer::renderer(master_control& parent)
    : _parent {parent}
{
}

void renderer::draw(ui::terminal& term)
{
    term.clear();

    auto&         level {_parent.get_level()};
    point_i const viewCenter {_parent.get_view_center()};
    auto&         tiles {level.get_tiles()};

    draw_map(term, viewCenter, tiles);
    draw_player(term, viewCenter);
    draw_log(term);
}

void renderer::draw_map(ui::terminal& term, point_i const& viewCenter, grid<tile>& tiles)
{
    for (i32 y {0}; y < TermMapSize.Height; ++y) {
        for (i32 x {0}; x < TermMapSize.Width; ++x) {
            point_i const termPos {point_i {x, y}};
            point_i const gridPos {term_to_grid(termPos, viewCenter)};
            if (!tiles.contains(gridPos)) { continue; }

            auto& tile {tiles[gridPos]};
            auto [fg, bg] {lighting(tile, gridPos)};
            if (gridPos == viewCenter) {
                term.color_set(colors::White, colors::Black);
                term.add_str(termPos, "+");
            } else if (tile.Seen || tile.InSight) {
                term.color_set(fg, bg);
                term.add_str(termPos, tile.Symbol);
            }
        }
    }
}

void renderer::draw_player(ui::terminal& term, point_i const& viewCenter)
{
    // symbol
    auto const& player {_parent.get_player()};
    auto const  termPos {grid_to_term(player.Position, viewCenter)};
    if (TermMapSize.contains(termPos)) {
        term.color_set(player.Color);
        term.add_str(termPos, "@");
    }

    // stats
    i32 const statsX {TermMapSize.Width};
    term.color_set(colors::White, colors::Black);
    term.add_str({statsX, 0}, std::format("{}", player.get_name()));

    term.color_set(colors::White, colors::Black);
    term.add_str({statsX, 1}, std::format("{}", player.Position));

    auto const drawBar {[&](i32 current, i32 max) {
        i32 const    tickCount {static_cast<i32>(std::ceil(static_cast<f32>(current) / max * 10.f))};
        string const ticks {string(tickCount, '-') + string(10 - tickCount, ' ')};
        term.add_str(std::format("[{}] {:03}/{:03}", ticks, current, max));
    }};

    term.color_set(colors::White, colors::Black);
    term.add_str({statsX, 3}, "Health: ");
    term.color_set(colors::Red, colors::Black);
    drawBar(player.get_hp(), player.get_hp_max());

    term.color_set(colors::White, colors::Black);
    term.add_str({statsX, 4}, "Mana:   ");
    term.color_set(colors::RoyalBlue, colors::Black);
    drawBar(player.get_mp(), player.get_mp_max());
}

void renderer::draw_log(ui::terminal& term)
{
    auto const& log {_parent.get_log()};

    std::span<log_message const> last;
    if (log.size() >= 5) {
        last = std::span<log_message const>(log).subspan(log.size() - 5, 5);
    } else {
        last = std::span<log_message const>(log);
    }
    term.color_set(colors::White, colors::Black);
    i32 y {TermMapSize.Height};
    for (auto const& message : last) {
        term.add_str({0, y++}, message.first);
        if (message.second > 1) {
            term.add_str(std::format(" ({}x)", message.second));
        }
    }
}

auto renderer::lighting(tile& tile, point_i gridPos) const -> std::pair<color, color>
{
    auto&       level {_parent.get_level()};
    auto&       tiles {level.get_tiles()};
    auto const& player {_parent.get_player()}; // TODO: light sources

    color fg;
    color bg;

    f64 const  distance {euclidean_distance(player.Position, gridPos)};
    auto const visualRange {player.get_visual_range()};
    bool const inLOS {distance <= visualRange[0] && is_line_of_sight(player.Position, gridPos, tiles)};
    if (distance > visualRange[0] || !inLOS) {
        if (tile.Seen) {
            fg = color::Lerp(colors::Black, colors::White, 0.15f);
            bg = colors::Black;
        } else {
            bg = colors::Black;
            fg = colors::Black;
        }
        tile.InSight = false;
    } else {
        // TODO: don't assume range.size == 3
        if (distance > visualRange[1]) {
            fg = color::Lerp(colors::Black, tile.ForegroundColor, 0.5f);
            bg = color::Lerp(colors::Black, tile.BackgroundColor, 0.5f);
        } else if (distance > visualRange[2]) {
            fg = color::Lerp(colors::Black, tile.ForegroundColor, 0.75f);
            bg = color::Lerp(colors::Black, tile.BackgroundColor, 0.75f);
        } else {
            fg = tile.ForegroundColor;
            bg = tile.BackgroundColor;
        }
        tile.Seen    = true;
        tile.InSight = true;
    }

    return {fg, bg};
}

}
