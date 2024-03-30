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

struct pile_description {
    std::string Pile;
    std::string CardCount;

    std::string Description;
    std::string DescriptionLabel;

    std::string Move;
    std::string MoveLabel;

    std::string Base;
    std::string BaseLabel;
};

constexpr isize INDEX_MARKER {-1};
constexpr isize INDEX_INVALID {-2};
constexpr f32   CARD_PADDING {1.05f};
constexpr f32   FACE_DOWN_OFFSET {10.0f};
constexpr f32   FACE_UP_OFFSET {7.0f};

class card_table;
class pile;
class start_scene;

namespace games {
    class base_game;
}

using lua_value  = std::variant<i64, f64, bool, std::string>;
using lua_return = std::optional<lua_value>;
using lua_params = scripting::parameter_pack<std::variant<games::base_game*, i64, f64, bool, std::string>>;

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
