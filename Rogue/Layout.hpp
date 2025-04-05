// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class base_layout {
public:
    virtual ~base_layout() = default;

    auto virtual generate(u64 seed, size_i size) -> grid<tile> = 0;

protected:
    void draw_horizontal_tunnel(grid<tile>& grid, i32 x0, i32 x1, i32 y, tile const& floor) const;
    void draw_vertical_tunnel(grid<tile>& grid, i32 x, i32 y0, i32 y1, tile const& floor) const;
    void draw_room(grid<tile>& grid, rect_i const& rect, tile const& floor) const;
};

////////////////////////////////////////////////////////////

class tunneling : public base_layout {
public:
    tunneling(i32 maxRooms, i32 minSize, i32 maxSize);

    auto generate(u64 seed, size_i size) -> grid<tile> override;

private:
    i32 _maxRooms;
    i32 _minSize;
    i32 _maxSize;
};

}
