// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../_common/Common.hpp" // IWYU pragma: keep

namespace Snake {

static constexpr gfx::tile_index_t TS_NONE {0};
static constexpr gfx::tile_index_t TS_FLOOR {1};
static constexpr gfx::tile_index_t TS_SNAKE_BODY {2};
static constexpr gfx::tile_index_t TS_SNAKE_HEAD {3};
static constexpr gfx::tile_index_t TS_SNAKE_DEAD {4};
static constexpr gfx::tile_index_t TS_FOOD {5};
static constexpr gfx::tile_index_t TS_STAR {6};
static constexpr gfx::tile_index_t TS_BOMB {7};

static constexpr point_i HIDDEN {-1, -1};

}