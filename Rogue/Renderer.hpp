// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

struct render_context {
    ui::terminal* Terminal {};

    level*                    Level {};
    player*                   Player {};
    std::vector<log_message>* Log {};

    point_i Center {};
};

class renderer {
public:
    void draw(render_context const& ctx);

private:
    void draw_map(render_context const& ctx);
    void draw_player(render_context const& ctx);
    void draw_log(render_context const& ctx);

    auto lighting(render_context const& ctx, point_i gridPos) const -> std::pair<color, color>;
};

}