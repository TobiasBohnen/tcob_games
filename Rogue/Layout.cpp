// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Layout.hpp"

namespace Rogue {

////////////////////////////////////////////////////////////

void base_layout::draw_horizontal_tunnel(grid<tile>& grid, i32 x0, i32 x1, i32 y, tile const& floor) const
{
    i32 const startX {std::min(x0, x1)};
    i32 const endX {std::max(x0, x1)};

    for (i32 x {startX}; x <= endX; ++x) {
        assert(grid.contains({x, y}));
        grid[{x, y}] = floor;
    }
}

void base_layout::draw_vertical_tunnel(grid<tile>& grid, i32 x, i32 y0, i32 y1, tile const& floor) const
{
    i32 const startY {std::min(y0, y1)};
    i32 const endY {std::max(y0, y1)};

    for (i32 y {startY}; y <= endY; ++y) {
        assert(grid.contains({x, y}));
        grid[{x, y}] = floor;
    }
}

void base_layout::draw_room(grid<tile>& grid, rect_i const& rect, tile const& floor) const
{
    for (i32 y {rect.top()}; y < rect.bottom(); ++y) {
        for (i32 x {rect.left()}; x < rect.right(); ++x) {
            assert(grid.contains({x, y}));
            grid[{x, y}] = floor;
        }
    }
}

static tile const FLOOR {.Type = tile_type::Floor, .Symbol = ".", .ForegroundColor = colors::Gray, .BackgroundColor = colors::SeaShell};

static tile const WALL0 {.Type = tile_type::Wall, .Symbol = "#", .ForegroundColor = colors::Black, .BackgroundColor = colors::LightSlateGray};
static tile const WALL1 {.Type = tile_type::Wall, .Symbol = "#", .ForegroundColor = colors::Black, .BackgroundColor = colors::DimGray};
static tile const WALL2 {.Type = tile_type::Wall, .Symbol = "#", .ForegroundColor = colors::Black, .BackgroundColor = colors::Silver};

////////////////////////////////////////////////////////////

tunneling::tunneling(i32 maxRooms, i32 minSize, i32 maxSize)
    : _maxRooms {maxRooms}
    , _minSize {minSize}
    , _maxSize {maxSize}
{
}

auto tunneling::generate(u64 seed, size_i size) -> grid<tile>
{
    rng rng {seed};

    grid<tile> retValue {size};
    for (usize i {0}; i < retValue.size(); ++i) {
        switch (rng(0, 2)) {
        case 0: retValue[i] = WALL0; break;
        case 1: retValue[i] = WALL1; break;
        case 2: retValue[i] = WALL2; break;
        }
    }

    std::vector<rect_i> rooms;
    for (i32 r {0}; r < _maxRooms; ++r) {
        i32 const w {rng(_minSize, _maxSize)};
        i32 const h {rng(_minSize, _maxSize)};
        i32 const x {rng(1, size.Width - w - 2)};
        i32 const y {rng(1, size.Height - h - 2)};

        rect_i const newRoom {x, y, w, h};
        bool         failed {false};
        for (auto const& outerRoom : rooms) {
            if (newRoom.intersects(outerRoom, true)) {
                failed = true;
                break;
            }
        }
        if (failed) { continue; }

        draw_room(retValue, newRoom, FLOOR);

        if (!rooms.empty()) {
            auto const newCenter {point_i {newRoom.center()}};
            auto const prevCenter {point_i {rooms.back().center()}};
            if (rng(0, 1) == 1) {
                draw_horizontal_tunnel(retValue, prevCenter.X, newCenter.X, prevCenter.Y, FLOOR);
                draw_vertical_tunnel(retValue, newCenter.X, prevCenter.Y, newCenter.Y, FLOOR);
            } else {
                draw_vertical_tunnel(retValue, prevCenter.X, prevCenter.Y, newCenter.Y, FLOOR);
                draw_horizontal_tunnel(retValue, prevCenter.X, newCenter.X, newCenter.Y, FLOOR);
            }
        }
        rooms.push_back(newRoom);
    }

    return retValue;
}
}
