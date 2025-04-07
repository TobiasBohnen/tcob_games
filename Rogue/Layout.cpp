// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Layout.hpp"

// based on: https://github.com/AtTheMatinee/dungeon-generation/blob/master/dungeonGenerationAlgorithms.py
// MIT License Copyright (c) 2017 AtTheMatinee

namespace Rogue {

static tile const FLOOR {.Type = tile_type::Floor, .Symbol = ".", .ForegroundColor = colors::Gray, .BackgroundColor = colors::SeaShell, .Seen = true};

static tile const WALL0 {.Type = tile_type::Wall, .Symbol = "#", .ForegroundColor = colors::Black, .BackgroundColor = colors::LightSlateGray, .Seen = true};
static tile const WALL1 {.Type = tile_type::Wall, .Symbol = "#", .ForegroundColor = colors::Black, .BackgroundColor = colors::DimGray, .Seen = true};
static tile const WALL2 {.Type = tile_type::Wall, .Symbol = "#", .ForegroundColor = colors::Black, .BackgroundColor = colors::Silver, .Seen = true};

////////////////////////////////////////////////////////////

void base_layout::clear_grid(grid<tile>& grid, rng& rng) const
{
    for (usize i {0}; i < grid.size(); ++i) {
        switch (rng(0, 2)) {
        case 0: grid[i] = WALL0; break;
        case 1: grid[i] = WALL1; break;
        case 2: grid[i] = WALL2; break;
        }
    }
}

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

void base_layout::draw_hallway(grid<tile>& grid, rect_i const& room1, rect_i const& room2, tile const& floor, bool b) const
{
    auto const [x1, y1] {room1.center()};
    auto const [x2, y2] {room2.center()};

    if (b) {
        draw_horizontal_tunnel(grid, x1, x2, y1, FLOOR);
        draw_vertical_tunnel(grid, x2, y1, y2, FLOOR);
    } else {
        draw_vertical_tunnel(grid, x1, y1, y2, FLOOR);
        draw_horizontal_tunnel(grid, x1, x2, y2, FLOOR);
    }
}

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
    clear_grid(retValue, rng);

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
            draw_hallway(retValue, newRoom, rooms.back(), FLOOR, rng(0, 1) == 1);
        }

        rooms.push_back(newRoom);
    }

    return retValue;
}
////////////////////////////////////////////////////////////

static i32 constexpr MIN_LEAF_SIZE {10};
static i32 constexpr MAX_LEAF_SIZE {24};

bsp_tree::bsp_tree(i32 minSize, i32 maxSize)
    : _minSize {minSize}
    , _maxSize {maxSize}
{
}

auto bsp_tree::generate(u64 seed, size_i size) -> grid<tile>
{
    rng rng {seed};

    grid<tile> retValue {size};
    clear_grid(retValue, rng);
    auto [mapWidth, mapHeight] {size};

    std::deque<std::shared_ptr<leaf>> leafs;
    leafs.emplace_back(std::make_shared<leaf>(rect_i {0, 0, mapWidth, mapHeight}));

    bool splitSuccessfully {true};

    while (splitSuccessfully) {
        splitSuccessfully = false;
        for (auto& l : leafs) {
            if (l->Child1 && l->Child2) { continue; }
            if ((l->Rect.width() > MAX_LEAF_SIZE) || (l->Rect.height() > MAX_LEAF_SIZE) || (rng(0, 100) > 80)) {
                if (!l->split_leaf(rng)) { continue; }
                leafs.push_back(l->Child1);
                leafs.push_back(l->Child2);
                splitSuccessfully = true;
            }
        }
    }
    leafs.front()->create_rooms(this, retValue, _minSize, _maxSize, rng);

    return retValue;
}

////////////////////////////////////////////////////////////

drunkards_walk::drunkards_walk(f32 percentGoal, i32 walkIterations, f32 weightedTowardCenter, f32 weightedTowardPreviousDirection)
    : _percentGoal {percentGoal}
    , _walkIterations {walkIterations}
    , _weightedTowardCenter {weightedTowardCenter}
    , _weightedTowardPreviousDirection {weightedTowardPreviousDirection}
{
}

auto drunkards_walk::generate(u64 seed, size_i size) -> grid<tile>
{
    rng rng {seed};

    grid<tile> retValue {size};
    auto [mapWidth, mapHeight] {size};
    clear_grid(retValue, rng);

    _walkIterations = std::max(_walkIterations, mapWidth * mapHeight * 10);

    _drunkardX  = rng(2, mapWidth - 2);
    _drunkardY  = rng(2, mapHeight - 2);
    _filledGoal = mapWidth * mapHeight * _percentGoal;

    for (i32 i {0}; i < _walkIterations; ++i) {
        walk(retValue, rng);
        if (_filled >= _filledGoal) { break; }
    }

    return retValue;
}

void drunkards_walk::walk(grid<tile>& grid, rng& rng)
{
    f32 up {1.0};
    f32 down {1.0};
    f32 right {1.0};
    f32 left {1.0};

    auto [mapWidth, mapHeight] {grid.extent()};

    // weight the random walk against edges
    if (_drunkardX < mapWidth * 0.25f) {
        right += _weightedTowardCenter;          // drunkard is at far left side of map
    } else if (_drunkardX > mapWidth * 0.75f) {
        left += _weightedTowardCenter;           // drunkard is at far right side of map
    }
    if (_drunkardY < mapHeight * 0.25f) {        // drunkard is at the top of the map
        down += _weightedTowardCenter;
    } else if (_drunkardY > mapHeight * 0.75f) { // drunkard is at the bottom of the map
        up += _weightedTowardCenter;
    }

    // weight the random walk in favor of the previous direction
    if (_previousDirection == direction::Up) { up += _weightedTowardPreviousDirection; }
    if (_previousDirection == direction::Down) { down += _weightedTowardPreviousDirection; }
    if (_previousDirection == direction::Right) { right += _weightedTowardPreviousDirection; }
    if (_previousDirection == direction::Left) { left += _weightedTowardPreviousDirection; }

    // normalize probabilities so they form a range from 0 to 1
    f32 total = up + down + right + left;

    up /= total;
    down /= total;
    right /= total;
    left /= total;

    // choose the direction
    f32       choice = rng(0.0f, 1.0f);
    i32       dx {};
    i32       dy {};
    direction direction {};
    if (choice < up) {
        dx        = 0;
        dy        = -1;
        direction = direction::Up;
    } else if (choice < (up + down)) {
        dx        = 0;
        dy        = 1;
        direction = direction::Down;
    } else if (choice < (up + down + right)) {
        dx        = 1;
        dy        = 0;
        direction = direction::Right;
    } else {
        dx        = -1;
        dy        = 0;
        direction = direction::Left;
    }

    // check colision at edges TODO: change so it stops one tile from edge
    if ((_drunkardX + dx > 0 && _drunkardX + dx < mapWidth - 1) && (_drunkardY + dy > 0 && _drunkardY + dy < mapHeight - 1)) {
        _drunkardX += dx;
        _drunkardY += dy;
        auto& tile {grid[{_drunkardX, _drunkardY}]};
        if (!tile_type_traits::passable(tile.Type)) {
            tile = FLOOR;
            _filled += 1;
        }
        _previousDirection = direction;
    }
}

////////////////////////////////////////////////////////////

leaf::leaf(rect_i const& rect)
    : Rect {rect}
{
}

auto leaf::split_leaf(rng& rng) -> bool
{
    // Begin splitting the leaf into two children
    if (Child1 || Child2) { return false; } // This leaf has already been split

    // Determine the direction of the split
    bool splitHorizontally {}; // Randomly choose split direction
    if (Rect.width() / static_cast<f32>(Rect.height()) >= 1.25f) {
        splitHorizontally = false;
    } else if (Rect.height() / static_cast<f32>(Rect.width()) >= 1.25f) {
        splitHorizontally = true;
    } else {
        splitHorizontally = rng(0, 1) == 0;
    }

    i32 const max {splitHorizontally
                       ? Rect.height() - MIN_LEAF_SIZE
                       : Rect.width() - MIN_LEAF_SIZE};
    if (max <= MIN_LEAF_SIZE) { return false; } // The leaf is too small to split further

    i32 const split {rng(MIN_LEAF_SIZE, max)};

    if (splitHorizontally) {
        Child1 = std::make_shared<leaf>(rect_i {Rect.left(), Rect.top(), Rect.width(), split});
        Child2 = std::make_shared<leaf>(rect_i {Rect.left(), Rect.top() + split, Rect.width(), Rect.height() - split});
    } else {
        Child1 = std::make_shared<leaf>(rect_i {Rect.left(), Rect.top(), split, Rect.height()});
        Child2 = std::make_shared<leaf>(rect_i {Rect.left() + split, Rect.top(), Rect.width() - split, Rect.height()});
    }

    return true;
}

void leaf::create_rooms(base_layout* layout, grid<tile>& grid, i32 minSize, i32 maxSize, rng& rng)
{
    if (Child1 || Child2) {
        // Recursively search for children until you hit the end of the branch
        if (Child1) {
            Child1->create_rooms(layout, grid, minSize, maxSize, rng);
        }
        if (Child2) {
            Child2->create_rooms(layout, grid, minSize, maxSize, rng);
        }

        if (Child1 && Child2) {
            layout->draw_hallway(grid, Child1->get_room(rng), Child2->get_room(rng), FLOOR, rng(0, 1) == 1);
        }
    } else {
        // Create rooms in the end branches of the BSP tree
        i32 const w {rng(minSize, std::min(maxSize, Rect.width() - 1))};
        i32 const h {rng(minSize, std::min(maxSize, Rect.height() - 1))};
        i32 const x {rng(Rect.left(), Rect.left() + Rect.width() - 1 - w)};
        i32 const y {rng(Rect.top(), Rect.top() + (Rect.height() - 1) - h)};
        _room = {x, y, w, h};
        layout->draw_room(grid, *_room, FLOOR);
    }
}

auto leaf::get_room(rng& rng) -> rect_i
{
    if (_room) { return *_room; }

    assert(Child1 || Child2);

    std::optional<rect_i> room1 {Child1 ? std::optional<rect_i> {Child1->get_room(rng)} : std::nullopt};
    std::optional<rect_i> room2 {Child2 ? std::optional<rect_i> {Child2->get_room(rng)} : std::nullopt};

    if (!room2) { return *room1; } // Only room_1 exists
    if (!room1) { return *room2; } // Only room_2 exists

    // If both room_1 and room_2 exist, pick one
    return (rng(0, 1) == 0) ? *room1 : *room2;
}
}
