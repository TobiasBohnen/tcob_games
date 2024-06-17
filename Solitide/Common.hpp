// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <functional>
#include <iostream> // IWYU pragma: keep
#include <tcob/tcob.hpp>

namespace solitaire {
using namespace tcob;
using namespace tcob::gfx::ui;
using namespace std::chrono_literals;

constexpr isize       INDEX_MARKER {-1};
constexpr isize       INDEX_INVALID {-2};
constexpr i32         UNLIMITED {-1};
constexpr i32         AUTO_LIMIT {-2};
constexpr f32         CARD_MARGIN {1.05f};
constexpr std::string FONT {"Roboto"};

class base_game;
class card_set;
class card_table;
struct color_themes;
struct game_info;
class pile;
class start_scene;

using lua_value     = std::variant<i64, f64, bool, std::string>;
using lua_return    = std::optional<lua_value>;
using lua_params    = scripting::parameter_pack<std::variant<base_game*, i64, f64, bool, std::string>>;
using reg_game_func = std::function<std::shared_ptr<base_game>()>;
using game_map      = std::map<std::string, std::pair<game_info, reg_game_func>>;
using theme_map     = std::map<std::string, color_themes>;
using card_set_map  = std::map<std::string, std::shared_ptr<card_set>>;

////////////////////////////////////////////////////////////

auto inline multiply(point_f pos, size_f size) -> point_f
{
    return {pos.X * CARD_MARGIN * size.Width, pos.Y * CARD_MARGIN * size.Height};
}

auto inline set_to_string(std::set<i32> const& numbers) -> std::string
{
    if (numbers.empty()) { return ""; }

    std::ostringstream oss;
    auto               it {numbers.begin()};
    i32                start {*it}, end {*it};
    ++it;

    while (it != numbers.end()) {
        if (*it == end + 1) {
            end = *it;
        } else {
            if (start == end) {
                oss << (start + 1);
            } else {
                oss << (start + 1) << "-" << (end + 1);
            }
            oss << ",";
            start = end = *it;
        }
        ++it;
    }

    if (start == end) {
        oss << (start + 1);
    } else {
        oss << (start + 1) << "-" << (end + 1);
    }

    return oss.str();
}

////////////////////////////////////////////////////////////

class settings {
public:
    settings() = default;

    std::string Version {"1.0.0"};

    prop<std::string> Theme {"default"};
    prop<std::string> Cardset {"default"};

    std::string                   LastGame;
    prop<std::deque<std::string>> Recent;

    bool HintMovable {true};
    bool HintTarget {true};

    void static Serialize(settings const& v, auto&& s)
    {
        s["version"]      = v.Version;
        s["theme"]        = v.Theme();
        s["cardset"]      = v.Cardset();
        s["last_game"]    = v.LastGame;
        s["recent"]       = v.Recent();
        s["hint_movable"] = v.HintMovable;
        s["hint_target"]  = v.HintTarget;
    }

    auto static Deserialize(settings& v, auto&& s) -> bool
    {
        return s.try_get(v.Version, "version")
            && s.try_get(*v.Theme, "theme")
            && s.try_get(*v.Cardset, "cardset")
            && s.try_get(v.LastGame, "last_game")
            && s.try_get(*v.Recent, "recent")
            && s.try_get(v.HintMovable, "hint_movable")
            && s.try_get(v.HintTarget, "hint_target");
    }
};

}
