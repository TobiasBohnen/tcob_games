// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

namespace Rogue {

class renderer {
public:
    explicit renderer(level& level);

    void draw(ui::terminal& term);

private:
    auto lighting(tile& tile, point_i gridPos) const -> std::pair<color, color>;

    level& _level;
};

}