// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <tcob/tcob.hpp>

using namespace tcob;
using namespace tcob::scripting;

constexpr size_i GRID_SIZE {512, 512};
constexpr u16    EMPTY_ELEMENT {0};
constexpr u16    ANY_ELEMENT {std::numeric_limits<u16>::max()};