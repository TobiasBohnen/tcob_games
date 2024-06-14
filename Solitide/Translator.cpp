// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Translator.hpp"

#include "games/Games.hpp"

namespace solitaire {

using namespace scripting::lua;

translator::translator()
{
    lua_script_game::CreateENV(_script);
    _func = *_script.run_file<function<return_type>>("scripts/lang/translator.lua");
    set_language("en-US");
}

void translator::set_language(std::string const& langid)
{
    if (_langid == langid) { return; }

    _langid                                = langid;
    (*_script.get_environment())["Locale"] = *_script.run_file<table>(std::format("scripts/lang/{}.lua", langid));

    for (auto it {_bindings.begin()}; it != _bindings.end(); ++it) {
        it->second();
    }
}

void translator::unbind(i32 id)
{
    for (auto it {_bindings.begin()}; it != _bindings.end(); ++it) {
        if (it->first == id) {
            _bindings.erase(it);
            return;
        }
    }
}

} // namespace solitaire
