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
};

struct slope {
    f64 TL {0};
    f64 TR {0};
    f64 BL {0};
    f64 BR {0};
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
    auto surface(point_i idx) const -> cell_surface;

    void precompute_slopes();
    auto slope_at(point_i const& cell) const -> slope;

    void update_light_visibility(usize losSamples);
    auto lights() const -> std::span<dynamic_light const>;
    auto lights_at(point_i const& cell) const -> std::span<u32 const>;

    auto is_clear(point_d pos, f64 radius, uid ignore = 0) const -> bool;
    auto is_wall_clear(point_d pos, f64 radius) const -> bool;
    auto is_blocking_light(point_i idx) const -> bool;

    auto hit_test(this auto&& self, point_d pos, f64 radius, uid ignore = 0);

    void toggle_wall(point_i idx);

    auto get_visibility(point_i idx) const -> tile_visibility;
    void mark_visible(point_i idx);
    void reveal_map();
    void fade_vision();

    auto spawn(level_object_desc desc) -> uid;

    auto find(uid id) -> level_object*;

    auto objects() const -> std::span<level_object const>;
    auto objects() -> std::span<level_object>;

private:
    auto closest_point_on_wall(point_i map, point_d pos) const -> std::optional<point_d>;

    map_t                                               _map;
    static_grid<tile_visibility, MAP_WIDTH, MAP_HEIGHT> _seen;

    std::vector<level_object> _objects;
    uid                       _nextId {0};

    std::vector<slope> _cellSlopes;

    std::vector<dynamic_light>    _lights;
    std::vector<std::vector<u32>> _cellLights;
    usize                         _losSamples {0};
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
