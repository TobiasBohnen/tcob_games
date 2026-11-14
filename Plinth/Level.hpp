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
#include "Lighting.hpp"
#include "Sky.hpp"
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

struct motes {
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

class level {
public:
    enum class tile_visibility : u8 {
        Hidden,  // Never seen
        Visible, // Seen in the current frame / FOV
        Explored // Seen before, but currently out of view (memory)
    };

    level();

    void update(milliseconds deltaTime, i32 lightLosSamples);

    auto can_move_to(point_d from, point_d to, f64 radius, uid ignore = 0) const -> bool;

    void toggle_wall(point_i map);

    auto get_visibility(point_i map) const -> tile_visibility;
    void mark_visible(point_i map);
    void reveal_map();
    void fade_vision();

    auto get_cell(point_i map) const -> cell const&;
    void set_cell(point_i map, cell const& c);
    auto surface_at(point_i map) const -> cell_surface;

    auto spawn(level_object_desc desc) -> uid;
    auto find(uid id) -> level_object*;

    auto objects() const -> std::span<level_object const>;
    auto objects() -> std::span<level_object>;

    auto time() const -> milliseconds;

    auto slopes_at(point_i map) const -> slopes;

    auto lighting() -> light_map&;
    auto lighting() const -> light_map const&;

    auto motes_at(point_i map) const -> motes const&;
    void set_motes(u8 motesIdx, motes const& motes);

    auto sky() -> sky_model&;
    auto sky() const -> sky_model const&;

    auto is_blocking_light(point_i map) const -> bool;

private:
    auto is_wall_clear(point_d pos, f64 radius) const -> bool;
    auto hit_test(point_d pos, f64 radius, uid ignore = 0) const -> level_object const*;

    auto closest_point_on_wall(point_i map, point_d pos) const -> std::optional<point_d>;

    void prepare_slopes();

    map_t                                               _map;
    static_grid<tile_visibility, MAP_WIDTH, MAP_HEIGHT> _seen;

    std::vector<motes> _motes;
    sky_model          _sky;
    light_map          _lighting;

    std::vector<level_object> _objects;
    uid                       _nextId {0};

    std::vector<slopes> _cellSlopes;
    bool                _slopesDirty {false};

    milliseconds _time {};
};
