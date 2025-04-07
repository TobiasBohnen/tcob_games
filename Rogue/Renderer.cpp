// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Renderer.hpp"

#include "Level.hpp"

namespace Rogue {

renderer::renderer(level& level)
    : _level {level}
{
}

void renderer::draw(ui::terminal& term)
{
    term.clear();

    point_i const viewCenter {_level.get_view_center()};
    auto&         tiles {_level.get_tiles()};

    for (i32 y {0}; y < TermMapSize.Height; ++y) {
        for (i32 x {0}; x < TermMapSize.Width; ++x) {
            point_i const termPos {point_i {x, y}};
            point_i const gridPos {term_to_grid(termPos, viewCenter)};
            if (!tiles.contains(gridPos)) { continue; }

            auto& tile {tiles[gridPos]};
            auto [fg, bg] {lighting(tile, gridPos)};
            if (tile.Seen || tile.InSight) {
                if (gridPos == viewCenter) { std::swap(fg, bg); }
                term.color_set(fg, bg);
                term.add_str(termPos, tile.Symbol);
            }
        }
    }

    auto const& player {_level.get_player()};
    auto const  termPos {grid_to_term(player.Position, viewCenter)};
    if (TermMapSize.contains(termPos)) {
        term.color_set(player.Color);
        term.add_str(termPos, "@");
    }

    term.add_str({60, 0}, std::format("{}", player.Position));
    auto const&             log {_level.get_log()};
    std::span<string const> last;
    if (log.size() >= 5) {
        last = std::span<string const>(log).subspan(log.size() - 5, 5);
    } else {
        last = std::span<string const>(log);
    }
    term.color_set(colors::White, colors::Black);
    i32 y {TermMapSize.Height};
    for (auto const& message : last) {
        term.add_str({0, y++}, message);
    }
}

auto renderer::lighting(tile& tile, point_i gridPos) const -> std::pair<color, color>
{
    auto&       tiles {_level.get_tiles()};
    auto const& player {_level.get_player()}; // TODO: light sources

    color fg;
    color bg;

    f64 const  distance {player.Position.distance_to(gridPos)};
    bool const inLOS {distance <= player.LightFalloff[0] && is_line_of_sight(player.Position, gridPos, tiles)};
    if (distance > player.LightFalloff[0] || !inLOS) {
        if (tile.Seen) {
            fg = color::Lerp(colors::Black, colors::White, 0.15f);
            bg = colors::Black;
        } else {
            bg = colors::Black;
            fg = colors::Black;
        }
        tile.InSight = false;
    } else {
        if (distance > player.LightFalloff[1]) {
            fg = color::Lerp(colors::Black, tile.ForegroundColor, 0.5f);
            bg = color::Lerp(colors::Black, tile.BackgroundColor, 0.5f);
        } else if (distance > player.LightFalloff[2]) {
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
