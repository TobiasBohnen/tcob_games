// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include <optional>
#include <span>
#include <vector>

#include "LevelObject.hpp"
#include "Walls.hpp"

struct cell_surface {
    i32 FloorTexture {INVALID_INDEX};
    i32 CeilingTexture {INVALID_INDEX};
    f64 FloorReflectivity {0.0};
    f64 FloorZ {0.0};
    f64 CeilingZ {1.0};
    u8  Motes {0};
    u8  Level {0};
};

struct slope {
    f64 TL {0};
    f64 TR {0};
    f64 BL {0};
    f64 BR {0};
};
struct slopes {
    slope Ceiling;
    slope Floor;
};

struct motes_settings {
    u32                  Count {0};         // motes per cell; 0 disables the pass
    f64                  FallSpeed {0.0};   // cell heights per second; negative rises
    f64                  SpeedJitter {0.0}; // per-mote speed variation, 0..1
    point_d              Wind {0.0, 0.0};   // constant drift in cells per second; motes wrap inside their cell
    f64                  Drift {0.0};       // wobble radius in world units
    f64                  DriftSpeed {1.0};  // wobble speed
    f64                  MinHeight {0.0};   // vertical band as a fraction of the cell height
    f64                  MaxHeight {1.0};
    f64                  Size {0.0};        // world units; 0 = single pixel
    f64                  Length {0.0};      // streak length in world units, trailing upward; 0 = dot
    f64                  Alpha {1.0};
    f64                  Twinkle {0.0};     // 0..1 depth of the brightness pulse
    f64                  TwinkleSpeed {1.0};
    f64                  Emissive {0.0};    // 0 = fully lit by the level's lights, 1 = self-lit
    bool                 Additive {false};  // glow instead of alpha blend
    std::vector<color_d> Colors {};         // each mote picks a random blend along this list; empty = white
};

struct sky_body_frame {
    point3_d Dir;
    f64      CosR {};
    f64      SinR {};
    f64      SunDot {};
    f64      Ambient {};
    bool     Lit {};
    color_d  Color;
    f64      Noise {};
    f64      NoiseScale {};
    f64      NoiseSeed {};
    color_d  NoiseColor {};
};

struct sky_frame {
    point3_d Sun;
    f64      Day {};
    f64      Night {};
    f64      Dusk {};
    f64      CosSun {};
    f64      Time {};
    bool     Stars {};
    f64      StarDensity {};
    f64      StarSize {};
    f64      StarTwinkle {};
    f64      CloudCoverage {};
    f64      CloudScale {};
    f64      CloudSoftness {};
    point_d  CloudWind {};
    color_d  CloudColor {};
    color_d  Zenith;
    color_d  Horizon;
    color_d  SunColor;
    color_d  DuskColor;

    std::vector<sky_body_frame> Bodies;
};

struct sky_body {
    radian_d Azimuth {0.0};    // radians at time 0
    radian_d Elevation {0.5};  // radians
    radian_d Radius {0.05};    // angular radius in radians
    radian_d OrbitSpeed {0.0}; // radians per second added to the azimuth
    f64      Ambient {0.06};   // brightness of the unlit side
    bool     Lit {true};       // false = emissive, ignores the sun
    color_d  Color {.R = 0.9, .G = 0.9, .B = 0.85};
    f64      Noise {0.0};      // 0 = plain disc, otherwise how much NoiseColor shows through
    f64      NoiseScale {4.0};
    f64      NoiseSeed {0.0};  // offsets the pattern, so two bodies don't look the same
    color_d  NoiseColor {.R = 0.7, .G = 0.7, .B = 0.65};
};

struct sky_settings {
    f64 TimeOfDay {0.5};             // 0..1, 0 = midnight, 0.5 = noon
    f64 DayLength {0.0};             // seconds per full day, 0 = frozen

    radian_d SunSize {0.04};         // angular radius in radians
    f64      SunTilt {0.3};          // sun path leans toward +Y

    f64 StarDensity {0.01};          // chance per star cell, 0 = no stars
    f64 StarSize {1.0};              // star size in screen pixels; below 1 stays one pixel but fainter
    f64 StarTwinkle {0.5};

    f64     CloudCoverage {0.0};     // 0 = no clouds, 1 = overcast
    f64     CloudScale {2.0};        // larger = smaller clouds
    f64     CloudSoftness {0.12};    // edge softness
    point_d CloudWind {0.01, 0.005}; // noise units per second
    color_d CloudColor {.R = 1.0, .G = 1.0, .B = 1.0};

    color_d DayZenith {.R = 0.25, .G = 0.5, .B = 0.95};
    color_d DayHorizon {.R = 0.65, .G = 0.8, .B = 1.0};
    color_d NightZenith {.R = 0.01, .G = 0.01, .B = 0.05};
    color_d NightHorizon {.R = 0.04, .G = 0.04, .B = 0.12};
    color_d DuskColor {.R = 1.0, .G = 0.45, .B = 0.2};
    color_d SunColor {.R = 1.0, .G = 0.95, .B = 0.8};

    std::vector<sky_body> Bodies {};
};

class level {
public:
    enum class tile_visibility : u8 {
        Hidden,  // Never seen
        Visible, // Seen in the current frame / FOV
        Explored // Seen before, but currently out of view (memory)
    };

    level();

    void update(milliseconds deltaTime);

    auto get_cell(point_i idx) const -> cell const&;
    void set_cell(point_i idx, cell const& cell);
    auto surface_at(point_i idx) const -> cell_surface;

    void precompute_slopes();
    auto slopes_at(point_i idx) const -> slopes;

    void update_light_visibility(usize losSamples);
    auto lights() const -> std::span<dynamic_light const>;
    auto lights_at(point_i idx) const -> std::span<u32 const>;

    auto can_move_to(point_d from, point_d to, f64 radius, uid ignore = 0) const -> bool;

    void toggle_wall(point_i idx);

    auto get_visibility(point_i idx) const -> tile_visibility;
    void mark_visible(point_i idx);
    void reveal_map();
    void fade_vision();

    auto motes_at(point_i idx) const -> motes_settings const&;
    void set_motes(u8 idx, motes_settings const& motes);

    auto sky() const -> sky_frame const&;
    void set_sky(sky_settings const& sky);

    auto spawn(level_object_desc desc) -> uid;

    auto find(uid id) -> level_object*;

    auto objects() const -> std::span<level_object const>;
    auto objects() -> std::span<level_object>;

    auto time() const -> milliseconds;

private:
    auto is_wall_clear(point_d pos, f64 radius) const -> bool;
    auto is_blocking_light(point_i idx) const -> bool;
    auto hit_test(point_d pos, f64 radius, uid ignore = 0) const -> level_object const*;

    auto closest_point_on_wall(point_i map, point_d pos) const -> std::optional<point_d>;

    void prepare_sky();

    map_t                                               _map;
    static_grid<tile_visibility, MAP_WIDTH, MAP_HEIGHT> _seen;

    std::vector<motes_settings> _motes;
    sky_settings                _skySettings;
    sky_frame                   _sky;

    std::vector<level_object> _objects;
    uid                       _nextId {0};

    std::vector<slopes> _cellSlopes;

    std::vector<dynamic_light>    _lights;
    std::vector<std::vector<u32>> _cellLights;
    usize                         _losSamples {0};

    milliseconds _time {};

    task_manager& _taskManager;
};
