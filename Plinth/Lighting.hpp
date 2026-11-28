// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <algorithm>
#include <concepts>
#include <span>
#include <vector>

#include "Common.hpp"

class map;

inline constexpr f64 SURFACE_LIGHT_BIAS {0.001}; // lights a surface from the open side so the cell lookup never lands inside the wall

////////////////////////////////////////////////////////////

struct dynamic_light {
    point3_d Position {};
    f64      Range {8.0};
    color    Color {colors::White};
    f64      Intensity {1.0};
    u32      Layers {LIGHT_LAYER_WORLD};
};

////////////////////////////////////////////////////////////

// what the shading of a pixel needs of a light, worked out once per frame
struct light_params {
    point_d Position {};
    f64     Z {0.0};
    f64     RangeSq {0.0};
    f64     InvRangeSq {0.0};
    f64     R {0.0}; // color * intensity, in 0..1
    f64     G {0.0};
    f64     B {0.0};
    u32     Layers {LIGHT_LAYER_WORLD};
};

inline auto MakeLightParams(point_d pos, f64 z, f64 range, f64 intensity, color c, u32 layers = LIGHT_LAYER_WORLD) -> light_params
{
    f64 const scale {intensity / 255.0};
    f64 const rangeSq {range * range};
    return {.Position = pos, .Z = z, .RangeSq = rangeSq, .InvRangeSq = 1.0 / rangeSq, .R = c.R * scale, .G = c.G * scale, .B = c.B * scale, .Layers = layers};
}

////////////////////////////////////////////////////////////

// a light that reaches a cell
struct cell_light {
    u32 Light {0};                        // index into lights()
    u16 Seen {0xFFFF};                    // the 4x4 parts of the cell, row by row, that see the light: all, unless something is in the way of some of them

    auto sees(point_d pos) const -> bool; // whether the part of the cell that `pos` is in sees the light
};

////////////////////////////////////////////////////////////

// who looks: where doors are seen from, and the light the viewer carries
struct light_viewer {
    point_d      Position {};
    light_params Light {};
};

// the lights of a stretch of surface, looked up once for all its pixels
struct light_span {
    u32  Mask {LIGHT_LAYERS_ALL};
    bool InBlocker {false};              // the cell blocks light: a surface in it has two sides, and what lights it is what the viewer's side sees

    std::span<cell_light const> Entries; // the lights listed for the cell, tested for every pixel

    // a span of one point only (a wall column): which lights reach it and the glow are the same for every pixel
    bool             Resolved {false};
    std::vector<u32> Reaching {}; // indices into lighting::lights()
    color_d          Glow {};
};

// The light along a run of pixels changes slowly from one pixel to the next: it is worked out for every `step`th one and blended in between.
// Rows are asked for in ascending order, `sample(row)` gives the exact light of a row.
class light_ramp {
public:
    light_ramp(i32 first, i32 last, i32 step)
        : _prevRow {first}
        , _nextRow {first}
        , _last {last}
        , _step {step}
    {
    }

    template <std::invocable<i32> Func>
    auto at(i32 row, Func const& sample) -> color_d
    {
        if (!_started) {
            _prev    = sample(_prevRow);
            _next    = _prev;
            _started = true;
        }

        while (row > _nextRow) {
            _prevRow = _nextRow;
            _prev    = _next;
            _nextRow = std::min(_prevRow + _step, _last);
            _next    = sample(_nextRow);
        }

        if (_nextRow == _prevRow) { return _prev; }
        return _prev.lerp(_next, static_cast<f64>(row - _prevRow) / static_cast<f64>(_nextRow - _prevRow));
    }

private:
    i32     _prevRow;
    i32     _nextRow;
    i32     _last;
    i32     _step;
    color_d _prev {};
    color_d _next {};
    bool    _started {false};
};

////////////////////////////////////////////////////////////

class lighting {
public:
    lighting();

    void update(level const& lvl, usize losSamples);

    auto lights() const -> std::span<light_params const>;
    auto lights_at(point_i map) const -> std::span<cell_light const>;

    // the lights of the pixels of a stretch of surface in one cell (floors, ceilings), tested per pixel
    auto span_in_cell(point_i cell, u32 lightMask) const -> light_span;
    // the lights of a single point (a wall column), tested once
    auto span_at(map const& map, light_viewer const& viewer, point_d pos, u32 lightMask) const -> light_span;
    // the light that reaches a surface point of a span: every light, the viewer's own and the glow of emissive floors
    auto accumulate(light_span const& span, map const& map, light_viewer const& viewer, point_d pos, f64 z) const -> color_d;

    auto glow_at(point_d pos) const -> color_d;    // the light that emissive floors spread, see rebuild_glow
    auto blocks_light(point_i cell) const -> bool; // a flat lookup of what map::is_blocking_light said at the last rebuild
    void invalidate();

private:
    void rebuild_blocking(level const& lvl);
    void rebuild_glow(level const& lvl);

    std::vector<u8>                      _blocked; // per cell: whether it blocks light, as of the last rebuild
    std::vector<color_d>                 _glow;    // per cell: the light of the emissive floors around
    std::vector<dynamic_light>           _lights;
    std::vector<light_params>            _params;  // per light, same order
    std::vector<std::vector<cell_light>> _cellLights;
    usize                                _losSamples {0};
    bool                                 _dirty {true};

    task_manager& _taskManager;
};
