// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp" // IWYU pragma: keep

namespace stn {

class assets {

public:
    assets(game& game);

    auto get_default_font() -> gfx::font*;
    auto get_texture(std::string const& name) -> gfx::texture*;
    auto get_cutscene_text() -> data::config::object const&;

    auto get_group() -> tcob::assets::group&;

private:
    gfx::canvas          _canvas;
    gfx::canvas_renderer _renderer {_canvas};
    bool                 _canvasDirty {true};

    gfx::font*           _font;
    tcob::assets::group& _group;
    data::config::object _cutsceneObj;
};

}
