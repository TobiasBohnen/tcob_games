// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

// IWYU pragma: always_keep

#include <tcob/tcob.hpp>

using namespace tcob;
using namespace tcob::ui;
using namespace std::chrono_literals;

inline constexpr i32 sprite1Texture {11};
inline constexpr i32 door1Texture {12};
inline constexpr i32 door1FrameTexture {13};

inline constexpr i32 TEXTURE_BPP {3};

inline constexpr size_i WALL_SIZE {64, 64};
inline constexpr size_i FLOOR_SIZE {64, 64};
inline constexpr size_i SKY_SIZE {512, 64};

inline constexpr isize MAX_TRANSPARENT_WALLS {8};

inline constexpr f64 FOV {90};

class cache;
class level;
class player;
