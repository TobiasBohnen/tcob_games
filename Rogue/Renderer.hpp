// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class renderer {
public:
    explicit renderer(master_control& parent);

    void draw(ui::terminal& term);

private:
    void draw_map(ui::terminal& term, point_i const& viewCenter, grid<tile>& tiles);
    void draw_player(ui::terminal& term, point_i const& viewCenter);
    void draw_log(ui::terminal& term);

    auto lighting(tile& tile, point_i gridPos) const -> std::pair<color, color>;

    master_control& _parent;
};

}