// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <vector>

#include "Common.hpp"

class map_renderer {
public:
    map_renderer(texture_cache& cache, size_i screenSize);

    auto draw(level const& level, player const& player) -> u32 const*;

private:
    std::vector<u32> _screen;

    texture_cache& _cache;
    size_i         _screenSize;
};
