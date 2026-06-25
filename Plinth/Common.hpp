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

inline constexpr i32 textureCount {25};
inline constexpr i32 textureBPP {3};
inline constexpr i32 maxFacings {8};

inline constexpr size_i wallSize {64, 64};
inline constexpr size_i skySize {1024, 64};

inline constexpr f64 FOV {66};

class cache;
class level;
class player;
