// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <functional>
#include <iostream> // IWYU pragma: keep
#include <tcob/tcob.hpp>

namespace solitaire {
using namespace tcob;
using namespace tcob::ui;
using namespace std::chrono_literals;

constexpr isize       INDEX_MARKER {-1};
constexpr isize       INDEX_INVALID {-2};
constexpr i32         UNLIMITED {-1};
constexpr i32         AUTO_LIMIT {-2};
constexpr f32         CARD_MARGIN {1.05f};
constexpr char const* FONT {"Roboto"};

class base_game;
class card_set;
class card_table;
struct color_themes;
struct game_info;
class pile;

using lua_value     = std::variant<i64, f64, bool, std::string>;
using lua_return    = std::optional<lua_value>;
using lua_params    = scripting::parameter_pack<base_game*, i64, f64, bool, std::string>;
using reg_game_func = std::function<std::shared_ptr<base_game>()>;

////////////////////////////////////////////////////////////

auto inline multiply(point_f pos, size_f size) -> point_f
{
    return {pos.X * CARD_MARGIN * size.Width, pos.Y * CARD_MARGIN * size.Height};
}

////////////////////////////////////////////////////////////

}
