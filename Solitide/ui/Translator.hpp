// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

namespace solitaire {
class translator {

public:
    translator();

    void set_language(std::string const& langid);

    auto translate(auto&&... args) -> std::string;

    auto bind(prop<std::string>& target, auto&& category, auto&& id) -> i32;
    auto bind(std::function<void(std::string)> const& target, auto&& category, auto&& id) -> i32;
    auto bind(std::function<void(std::vector<std::string>)> const& target, auto&& category, auto&& id) -> i32;

    void unbind(i32 id);

    // helper
    void bind_tab(tab_container* container, widget* tab);
    void bind_grid_header(grid_view* gv, std::string const& id);

private:
    auto do_bind(auto&& func) -> i32;

    scripting::script _script;

    using return_type = std::variant<std::string, std::vector<std::string>>;
    scripting::function<return_type> _func;

    std::vector<std::pair<i32, std::function<void(void)>>> _bindings;

    std::string _langid;
    i32         _currentId {-1};
};

inline auto translator::translate(auto&&... args) -> std::string
{
    auto v {_func(args...)};
    if (std::string * item {std::get_if<std::string>(&v)}) {
        return *item;
    }

    return "";
}

inline auto translator::bind(prop<std::string>& target, auto&& category, auto&& id) -> i32
{
    return bind([&target](std::string const& val) { target = val; }, category, id);
}

inline auto translator::bind(std::function<void(std::string)> const& target, auto&& category, auto&& id) -> i32
{
    return do_bind([this, category, id, target] {
        auto v {_func(category, id)};
        if (std::string * item {std::get_if<std::string>(&v)}) {
            target(*item);
        }
    });
}

inline auto translator::bind(std::function<void(std::vector<std::string>)> const& target, auto&& category, auto&& id) -> i32
{
    return do_bind([this, category, id, target] {
        auto v {_func(category, id)};
        if (auto* item {std::get_if<std::vector<std::string>>(&v)}) {
            target(*item);
        }
    });
}

inline auto translator::do_bind(auto&& func) -> i32
{
    func();
    auto const retValue {++_currentId};
    _bindings.emplace_back(retValue, func);
    return retValue;
}
}
