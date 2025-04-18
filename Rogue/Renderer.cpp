// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Renderer.hpp"

#include "level/Level.hpp"
#include "monsters/Player.hpp"
#include "objects/Object.hpp"

namespace Rogue {

void renderer::draw(render_context const& ctx)
{
    ctx.Terminal->clear();

    draw_map(ctx);
    draw_player(ctx);
    draw_log(ctx);
}

void renderer::draw_map(render_context const& ctx)
{
    auto& tiles {ctx.Level->tiles()};
    auto& term {*ctx.Terminal};

    for (i32 y {0}; y < TermMapSize.Height; ++y) {
        for (i32 x {0}; x < TermMapSize.Width; ++x) {
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
                draw_objects(ctx, tile, gridPos);
            }
        }
    }
}

void renderer::draw_objects(render_context const& ctx, tile const& tile, point_i gridPos)
{
    auto& term {*ctx.Terminal};

    auto const& objects {tile.Objects};
    for (auto const& object : objects) {
        point_i const termPos {grid_to_term(gridPos, ctx.Center)};
        if (TermMapSize.contains(termPos)) {
            auto const colors {tile.InSight ? object->colors() : seen_colors()};
            term.color_set(colors.first, colors.second);
            term.add_str(termPos, object->symbol());
        }
    }
}

void renderer::draw_player(render_context const& ctx)
{
    auto& term {*ctx.Terminal};

    // symbol
    auto const& player {*ctx.Player};
    auto const  termPos {grid_to_term(player.Position, ctx.Center)};
    if (TermMapSize.contains(termPos)) {
        term.color_set(player.color(), colors::Black);
        term.add_str(termPos, player.symbol());
    }

    // stats
    auto const& stats {player.stats()};
    i32 const   statsX {TermMapSize.Width};
    term.color_set(colors::White, colors::Black);
    term.add_str({statsX, 0}, std::format("{}", stats.Name));

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
    drawBar(stats.HP, stats.HPMax);

    term.color_set(colors::White, colors::Black);
    term.add_str({statsX, 4}, "Mana:   ");
    term.color_set(colors::RoyalBlue, colors::Black);
    drawBar(stats.MP, stats.MPMax);
}

void renderer::draw_log(render_context const& ctx)
{
    auto&       term {*ctx.Terminal};
    auto const& log {*ctx.Log};

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

auto renderer::lighting(render_context const& ctx, tile& tile, point_i gridPos) const -> color_pair
{
    auto& term {*ctx.Terminal};

    auto&       level {*ctx.Level};
    auto const& player {*ctx.Player};

    if (!level.is_line_of_sight(player.Position, gridPos)) {
        tile.InSight = false;
        return tile.Seen
            ? seen_colors()
            : std::pair {colors::Black, colors::Black};
    }

    // Variables to accumulate light contributions separately for foreground and background.
    f32 lightFgR {0.f}, lightFgG {0.f}, lightFgB {0.f};
    f32 lightBgR {0.f}, lightBgG {0.f}, lightBgB {0.f};
    f32 totalLightFactor {0.f};

    // Lambda to accumulate a given light's color contribution scaled by its factor.
    auto const accumulateLight {[&](color const& lightColor, f32 const factor) -> void {
        lightFgR += (lightColor.R / 255.f) * factor;
        lightFgG += (lightColor.G / 255.f) * factor;
        lightFgB += (lightColor.B / 255.f) * factor;

        lightBgR += (lightColor.R / 255.f) * factor;
        lightBgG += (lightColor.G / 255.f) * factor;
        lightBgB += (lightColor.B / 255.f) * factor;

        totalLightFactor += factor;
    }};

    // Process environmental light sources.
    auto const& lights {level.lights()};
    for (auto const& light : lights) {
        f64 const distance {euclidean_distance(light->Position, gridPos)};
        f64 const range {light->Range};
        if (distance > range) { continue; }

        if (!level.is_line_of_sight(light->Position, gridPos)) { continue; }

        f32 const factor {light->Falloff && distance != 0
                              ? static_cast<f32>(std::clamp((range * range) / (distance * distance) * ((range - distance) / range), 0., 1.))
                              : 1.0f};
        accumulateLight(light->Color, factor * light->Intensity);
    }

    // Process player light.
    f64 const distance {euclidean_distance(player.Position, gridPos)};
    f64 const range {player.stats().VisualRange};
    if (distance <= range) {
        f32 const factor {distance != 0
                              ? static_cast<f32>(std::clamp((range * range) / (distance * distance) * ((range - distance) / range), 0., 1.))
                              : 1.0f};
        accumulateLight(player.color(), factor);
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

auto renderer::seen_colors() const -> color_pair
{
    return {color::Lerp(colors::Black, colors::White, 0.15f), colors::Black};
}

}
