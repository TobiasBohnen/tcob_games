// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class renderer {
public:
    struct context {
        ui::terminal* Terminal {};

        level*  Level {};
        player* Player {};
        profile PlayerProfile {};

        std::vector<log_message>* Log {};

        mode     Mode {};
        mfd_mode MfdMode {};

        point_i Center {};
    };

    void draw(context const& ctx);

private:
    void draw_layout(context const& ctx);
    void draw_map(context const& ctx);
    void draw_objects(context const& ctx, color bg, tile const& tile, point_i gridPos);
    void draw_player(context const& ctx);
    void draw_log(context const& ctx);
    void draw_mfd(context const& ctx);
    void draw_mode(context const& ctx);

    auto lighting(context const& ctx, tile& tile, point_i gridPos) const -> color_pair;
};

}