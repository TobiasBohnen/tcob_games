// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include <span>

class map_renderer {
public:
    void draw(level const& level, player const& player, std::span<u32> screen, size_i screenSize) const;
};
