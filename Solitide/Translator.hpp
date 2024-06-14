// Copyright (c) 2024 Tobias Bohnen
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

    auto bind(prop<std::string>& target, auto&& category, auto&& id, auto&&... context) -> i32
    {
        return bind([&target](std::string const& val) { target = val; }, category, id, context...);
    }

    auto bind(std::function<void(std::string)>&& target, auto&& category, auto&& id, auto&&... context) -> i32
    {
        return bind([=, this]() {
            auto v {_func(category, id, context...)};
            if (std::string * item {std::get_if<std::string>(&v)}) {
                target(*item);
            }
        });
    }

    auto bind(std::function<void(std::vector<std::string>)>&& target, auto&& category, auto&& id, auto&&... context) -> i32
    {
        return bind([=, this]() {
            auto v {_func(category, id, context...)};
            if (std::vector<std::string> * item {std::get_if<std::vector<std::string>>(&v)}) {
                target(*item);
            }
        });
    }

    void unbind(i32 id);

private:
    auto bind(auto&& func) -> i32
    {
        func();
        auto const retValue {++_currentId};
        _bindings.emplace_back(retValue, func);
        return retValue;
    }

    scripting::lua::script _script;

    using return_type = std::variant<std::string, std::vector<std::string>>;
    scripting::lua::function<return_type> _func;

    std::vector<std::pair<i32, std::function<void(void)>>> _bindings;

    std::string _langid;
    i32         _currentId {-1};
};
}
