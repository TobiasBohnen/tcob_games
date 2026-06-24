// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <vector>

#include "Common.hpp"

class raycaster {
public:
    raycaster(cache& cache, size_i screenSize);

    void prepare_draw();
    void draw(level const& level, player const& player, u32* screenBuf, i32 columnStart, i32 columnEnd);

private:
    void draw_walls(level const& level, player const& player, u32* screenBuf, i32 columnStart, i32 columnEnd);
    void draw_sprites(level const& level, player const& player, u32* screenBuf, i32 columnStart, i32 columnEnd);

    cache& _cache;
    size_i _screenSize {};

    std::vector<f64> _zBuffer;
    std::vector<f64> _rowDist;
    std::vector<f64> _spriteDepthBuffer;
};
