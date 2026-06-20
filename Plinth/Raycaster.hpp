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
    raycaster(Cache& cache, WorldMap const& worldMap, size_i screenSize, f64 horizontalFovDegrees)
        : _cache {cache}
        , _worldMap {worldMap}
        , _mapSize {worldMap.size()}
        , _screenSize {screenSize}
        , _texSize {_cache.tex_size()}
        , _texBpp(_cache.tex_bpp())
    {

        _zBuffer.resize(_screenSize.Width);

        _rowDist.resize(_screenSize.Height);
        for (i32 y {0}; y < _screenSize.Height; y++) {
            if (2.0 * y == _screenSize.Height) {
                _rowDist[y] = std::numeric_limits<f64>::infinity();
                continue;
            }
            _rowDist[y] = _screenSize.Height / ((2.0 * y) - _screenSize.Height);
        }

        f64 const fov {horizontalFovDegrees * TAU / 360.0};
        _dir   = {-1, 0};
        _plane = {0, std::tan(fov / 2.0)};

        _projPlaneDist = (_screenSize.Width / 2.0) / std::tan(fov / 2.0);
    }

    [[nodiscard]] auto position() const -> point_d const& { return _pos; }
    void               set_position(point_d pos) { _pos = pos; }

    auto sprites() -> std::vector<sprite>& { return _sprites; }

    auto move(f64 forwardAmount, f64 strafeAmount, f64 rotateAmount) -> bool;
    void cast(i32 x, u32* screenBuf);
    void draw_sprites(u32* screenBuf);

private:
    [[nodiscard]] auto is_position_clear(point_d pos) const -> bool
    {
        i32 const tileX {static_cast<i32>(pos.X)};
        i32 const tileY {static_cast<i32>(pos.Y)};

        if (tileX < 0 || tileX >= _mapSize.Width || tileY < 0 || tileY >= _mapSize.Height) {
            return false;
        }

        return _worldMap[point_i {tileX, tileY}] == 0;
    }

    Cache&          _cache;
    WorldMap const& _worldMap;
    size_i          _mapSize {};
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
