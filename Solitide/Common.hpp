// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <tcob/tcob.hpp>

namespace solitaire {
using namespace tcob;
using namespace tcob::gfx::ui;
using namespace std::chrono_literals;

enum class game_state {
    Initial,
    Running,
    Failure,
    Success
};

constexpr isize INDEX_MARKER {-1};
constexpr isize INDEX_INVALID {-2};
constexpr f32   CARD_PADDING {1.05f};

class card_table;
class pile;
class start_scene;

namespace games {
    class base_game;
}

using lua_value  = std::variant<i64, f64, bool, std::string>;
using lua_return = std::optional<lua_value>;
using lua_params = scripting::parameter_pack<std::variant<games::base_game*, i64, f64, bool, std::string>>;
using rng        = random::rng_xoshiro_256_plus_plus;

////////////////////////////////////////////////////////////

auto inline multiply(point_f pos, size_f size) -> point_f
{
    return {pos.X * CARD_PADDING * size.Width, pos.Y * CARD_PADDING * size.Height};
}

struct hit_test_result {
    pile* Pile {nullptr};
    isize Index {INDEX_INVALID};
};

}
