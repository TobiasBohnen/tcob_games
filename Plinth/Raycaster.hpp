// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <vector>

#include "../_common/Common.hpp"

struct sprite {
    point_d Pos;
    size_d  Size; // world-unit width/height; {1,1} = one full tile
    i32     Texture {};
};

template <typename Cache, typename WorldMap>
class raycaster {
public:
    raycaster(Cache& cache, size_i screenSize, f64 horizontalFovDegrees);

    void set_player_position(point_d pos);
    void set_world_map(WorldMap const& worldMap);

    auto sprites() -> std::vector<sprite>&;

    auto move(f64 forwardAmount, f64 strafeAmount, f64 rotateAmount) -> bool;
    void cast(i32 x, u32* screenBuf);
    void draw_sprites(u32* screenBuf);

private:
    auto is_position_clear(point_d pos) const -> bool;

    Cache&          _cache;
    WorldMap const* _worldMap;
    size_i          _screenSize {};
    size_i          _texSize {};
    i32             _texBpp {};

    point_d _pos {};
    point_d _dir {};
    point_d _plane {};
    f64     _projPlaneDist {};

    std::vector<f64>    _zBuffer;
    std::vector<f64>    _rowDist;
    std::vector<sprite> _sprites;

    static constexpr f64 CollisionMargin {0.4};
};

#include "Raycaster.inl"
