// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <tcob/tcob.hpp>

using namespace tcob;
using namespace tcob::gfx::ui;
using namespace std::chrono_literals;

namespace stn {

constexpr size_i CANVAS_SIZE {160, 144};
constexpr size_f CANVAS_SIZE_F {size_f {CANVAS_SIZE}};

constexpr size_i TILE_SIZE {16, 16};
constexpr size_f TILE_SIZE_F {size_f {TILE_SIZE}};
constexpr size_i TILE_COUNT {10, 10};
constexpr isize  TILE_COUNT_TOTAL {TILE_COUNT.Width * TILE_COUNT.Height};
constexpr size_i TILESET_SIZE {(TILE_COUNT.Width * TILE_SIZE.Width), (TILE_COUNT.Height * TILE_SIZE.Height)};

constexpr color COLOR0 {color::FromRGB(0x9bbc0f)};
constexpr color COLOR1 {color::FromRGB(0x8bac0f)};
constexpr color COLOR2 {color::FromRGB(0x306230)};
constexpr color COLOR3 {color::FromRGB(0x0f380f)};

}
