// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include <optional>
#include <span>
#include <type_traits>
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

    auto is_clear(point_d pos, f64 radius, uid ignore = 0) const -> bool;
    auto is_wall_clear(point_d pos, f64 radius) const -> bool;
    auto is_blocking_light(point_i idx) const -> bool;

    auto hit_test(this auto&& self, point_d pos, f64 radius, uid ignore = 0);

    void toggle_wall(point_i idx);

    auto get_visibility(point_i idx) const -> tile_visibility;
    void mark_visible(point_i idx);
    void reveal_map();
    void fade_vision();

    auto motes_at(point_i idx) const -> motes_settings const&;
    void set_motes(u8 idx, motes_settings const& motes);

    auto spawn(level_object_desc desc) -> uid;

    auto find(uid id) -> level_object*;

    auto objects() const -> std::span<level_object const>;
    auto objects() -> std::span<level_object>;

private:
    auto closest_point_on_wall(point_i map, point_d pos) const -> std::optional<point_d>;

    map_t                                               _map;
    static_grid<tile_visibility, MAP_WIDTH, MAP_HEIGHT> _seen;

    std::vector<motes_settings> _motes;

    std::vector<level_object> _objects;
    uid                       _nextId {0};

    std::vector<slopes> _cellSlopes;

    std::vector<dynamic_light>    _lights;
    std::vector<std::vector<u32>> _cellLights;
    usize                         _losSamples {0};

    task_manager& _taskManager;
};

inline auto level::hit_test(this auto&& self, point_d pos, f64 radius, uid ignore)
{
    using object_t = std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(self)>>, level_object const, level_object>;

    object_t* found {nullptr};
    for (auto& go : self._objects) {
        if (go.id() != ignore && go.overlaps(pos, radius)) {
            found = &go;
            break;
        }
    }
    return found;
}
