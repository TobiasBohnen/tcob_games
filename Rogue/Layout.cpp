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

void base_layout::fill_grid(grid<tile>& grid, rng& rng, std::span<tile const> tiles) const
{
    for (usize i {0}; i < grid.count(); ++i) {
        i32 const idx {rng(0, static_cast<i32>(tiles.size() - 1))};
        grid[i] = tiles[idx];
    }
}

void base_layout::draw_horizontal_tunnel(grid<tile>& grid, i32 x0, i32 x1, i32 y, tile const& floor)
{
    i32 const startX {std::min(x0, x1)};
    i32 const endX {std::max(x0, x1)};

    for (i32 x {startX}; x <= endX; ++x) {
        if (x == 0 || y == 0 || x == grid.width() - 1 || y == grid.height() - 1) { continue; }

        point_i const pos {x, y};
        assert(grid.contains(pos));
        grid[pos] = floor;
    }
}

void base_layout::draw_vertical_tunnel(grid<tile>& grid, i32 x, i32 y0, i32 y1, tile const& floor)
{
    i32 const startY {std::min(y0, y1)};
    i32 const endY {std::max(y0, y1)};

    for (i32 y {startY}; y <= endY; ++y) {
        if (x == 0 || y == 0 || x == grid.width() - 1 || y == grid.height() - 1) { continue; }

        point_i const pos {x, y};
        assert(grid.contains(pos));
        grid[pos] = floor;
    }
}

void base_layout::draw_room(grid<tile>& grid, rect_i const& rect, tile const& floor)
{
    for (i32 y {rect.top()}; y < rect.bottom(); ++y) {
        for (i32 x {rect.left()}; x < rect.right(); ++x) {
            if (x == 0 || y == 0 || x == grid.width() - 1 || y == grid.height() - 1) { continue; }

            point_i const pos {x, y};
            assert(grid.contains(pos));
            grid[pos] = floor;
        }
    }
}

void base_layout::draw_hallway(grid<tile>& grid, rect_i const& room1, rect_i const& room2, tile const& floor, bool b)
{
    auto const [x1, y1] {room1.center()};
    auto const [x2, y2] {room2.center()};

    if (b) {
        draw_horizontal_tunnel(grid, x1, x2, y1, floor);
        draw_vertical_tunnel(grid, x2, y1, y2, floor);
    } else {
        draw_vertical_tunnel(grid, x1, y1, y2, floor);
        draw_horizontal_tunnel(grid, x1, x2, y2, floor);
    }
}

////////////////////////////////////////////////////////////

tunneling::tunneling(i32 maxRooms, i32 minRoomSize, i32 maxRoomSize)
    : _maxRooms {maxRooms}
    , _minRoomSize {minRoomSize}
    , _maxRoomSize {maxRoomSize}
{
}

auto tunneling::generate(u64 seed, size_i size) -> grid<tile>
{
    rng rng {seed};

    grid<tile> retValue {size};
    auto const [mapWidth, mapHeight] {size};
    fill_grid(retValue, rng, std::array {WALL0, WALL1, WALL2});

    std::vector<rect_i> rooms;
    for (i32 r {0}; r < _maxRooms; ++r) {
        i32 const w {rng(_minRoomSize, _maxRoomSize)};
        i32 const h {rng(_minRoomSize, _maxRoomSize)};
        i32 const x {rng(1, mapWidth - w - 2)};
        i32 const y {rng(1, mapHeight - h - 2)};

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

bsp_tree::bsp_tree(i32 minRoomSize, i32 maxRoomSize)
    : _minRoomSize {minRoomSize}
    , _maxRoomSize {maxRoomSize}
{
}

auto bsp_tree::generate(u64 seed, size_i size) -> grid<tile>
{
    static i32 constexpr MAX_LEAF_SIZE {24};

    rng rng {seed};

    grid<tile> retValue {size};
    auto const [mapWidth, mapHeight] {size};
    fill_grid(retValue, rng, std::array {WALL0, WALL1, WALL2});

    leaf root {rect_i {0, 0, mapWidth, mapHeight}};
    root.split_leaf(rng, MAX_LEAF_SIZE);
    root.create_rooms(
        [&](rect_i const& room1, rect_i const& room2) { draw_hallway(retValue, room1, room2, FLOOR, rng(0, 1) == 1); },
        [&](rect_i const& room0) { draw_room(retValue, room0, FLOOR); },
        _minRoomSize, _maxRoomSize, rng);

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
    _filled            = 0;
    _filledGoal        = 0;
    _previousDirection = direction::None;
    _drunkardX         = 0;
    _drunkardY         = 0;

    rng rng {seed};

    _grid = grid<tile> {size};
    auto const [mapWidth, mapHeight] {size};
    fill_grid(_grid, rng, std::array {WALL0, WALL1, WALL2});

    _floor = FLOOR;

    _walkIterations = std::max(_walkIterations, mapWidth * mapHeight * 10);

    _drunkardX  = rng(2, mapWidth - 2);
    _drunkardY  = rng(2, mapHeight - 2);
    _filledGoal = mapWidth * mapHeight * _percentGoal;

    for (i32 i {0}; i < _walkIterations; ++i) {
        walk(rng);
        if (_filled >= _filledGoal) { break; }
    }

    return _grid;
}

void drunkards_walk::walk(rng& rng)
{
    f32 up {1.0};
    f32 down {1.0};
    f32 right {1.0};
    f32 left {1.0};

    auto [mapWidth, mapHeight] {_grid.size()};

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
    f32 const choice {rng(0.0f, 1.0f)};
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
        auto& tile {_grid[{_drunkardX, _drunkardY}]};
        if (!tile_traits::passable(tile)) {
            tile = _floor;
            _filled += 1;
        }
        _previousDirection = direction;
    }
}

////////////////////////////////////////////////////////////

cellular_automata::cellular_automata(i32 iterations, i32 neighbors, f32 wallProbability, i32 minRoomSize, i32 maxRoomSize, bool smoothEdges, i32 smoothing)
    : _iterations {iterations}
    , _neighbors {neighbors}
    , _wallProbability {wallProbability}
    , _minRoomSize {minRoomSize}
    , _maxRoomSize {maxRoomSize}
    , _smoothEdges {smoothEdges}
    , _smoothing {smoothing}
{
}

auto cellular_automata::generate(u64 seed, size_i size) -> grid<tile>
{
    _caves.clear();

    _rng = rng {seed};

    _grid = grid<tile> {size};
    fill_grid(_grid, _rng, std::array {WALL0, WALL1, WALL2});

    _floor = FLOOR;
    _wall  = WALL0;

    random_fill_map();
    draw_caves();
    get_caves();
    connect_caves();
    cleanup_map();

    return _grid;
}

void cellular_automata::random_fill_map()
{
    for (usize i {0}; i < _grid.count(); ++i) {
        if (_rng(0.0f, 1.0f) >= _wallProbability) {
            _grid[i] = _floor;
        }
    }
}

void cellular_automata::draw_caves()
{
    for (i32 i {0}; i < _iterations; ++i) {
        // Pick a random point with a buffer around the edges of the map
        i32 const tileX {_rng(1, _grid.width() - 2)};  //(2,mapWidth-3)
        i32 const tileY {_rng(1, _grid.height() - 2)}; //(2,mapHeight-3)

        i32 const neighbors {get_adjacent_walls({tileX, tileY})};
        if (neighbors > _neighbors) {
            _grid[{tileX, tileY}] = _wall;  // 1
        } else if (neighbors < _neighbors) {
            _grid[{tileX, tileY}] = _floor; // 0
        }
    }

    // ==== Clean Up Map ====
    cleanup_map();
}

void cellular_automata::cleanup_map()
{
    if (!_smoothEdges) { return; }

    for (i32 i {0}; i < 5; ++i) {
        for (i32 x {1}; x < _grid.width() - 1; ++x) {
            for (i32 y {1}; y < _grid.height() - 1; ++y) {
                point_i const pos {x, y};
                if (tile_traits::passable(_grid[pos]) && get_adjacent_walls_simple(pos) <= _smoothing) {
                    _grid[pos] = _floor;
                }
            }
        }
    }
}

void cellular_automata::create_tunnel(point_i point1, point_i point2, std::unordered_set<i32> const& currentCave)
{
    // run a heavily weighted random Walk
    // from point2 to point1
    auto [drunkardX, drunkardY] {point2};

    while (!currentCave.contains(get_index({drunkardX, drunkardY}))) {
        // ==== Choose Direction ====
        f32 up {1.0f};
        f32 down {1.0f};
        f32 right {1.0f};
        f32 left {1.0f};

        i32 const weight {1};

        // weight the random walk against edges
        if (drunkardX < point1.X) {        // drunkard is left of point1
            right += weight;
        } else if (drunkardX > point1.X) { // drunkard is right of point1
            left += weight;
        }
        if (drunkardY < point1.Y) {        // drunkard is above point1
            down += weight;
        } else if (drunkardY > point1.Y) { // drunkard is below point1
            up += weight;
        }

        // normalize probabilities so they form a range from 0 to 1
        f32 const total {up + down + right + left};
        up /= total;
        down /= total;
        right /= total;
        left /= total;

        // choose the direction
        f32 const choice = _rng(0.0f, 1.0f);
        i32       dx {};
        i32       dy {};
        if (choice < up) {
            dx = 0;
            dy = -1;
        } else if (choice < (up + down)) {
            dx = 0;
            dy = 1;
        } else if (choice < (up + down + right)) {
            dx = 1;
            dy = 0;
        } else {
            dx = -1;
            dy = 0;
        }

        // ==== Walk ====
        // check colision at edges
        if (_grid.contains({drunkardX + dx, drunkardY + dy})) {
            drunkardX += dx;
            drunkardY += dy;
            if (!tile_traits::passable(_grid[{drunkardX, drunkardY}])) {
                _grid[{drunkardX, drunkardY}] = _floor;
            }
        }
    }
}

auto cellular_automata::get_adjacent_walls(point_i p) -> i32
{
    i32 retValue {0};
    for (i32 x {p.X - 1}; x <= p.X + 1; ++x) {
        for (i32 y {p.Y - 1}; y <= p.Y + 1; ++y) {
            point_i const pos {x, y};
            if (pos == p) { continue; }
            if (!tile_traits::passable(_grid[pos])) {
                retValue++;
            }
        }
    }
    return retValue;
}

auto cellular_automata::get_adjacent_walls_simple(point_i p) -> i32
{
    i32 retValue {0};
    if (!tile_traits::passable(_grid[{p.X, p.Y - 1}])) { ++retValue; }
    if (!tile_traits::passable(_grid[{p.X, p.Y + 1}])) { ++retValue; }
    if (!tile_traits::passable(_grid[{p.X - 1, p.Y}])) { ++retValue; }
    if (!tile_traits::passable(_grid[{p.X + 1, p.Y}])) { ++retValue; }
    return retValue;
}

void cellular_automata::get_caves()
{
    // locate all the caves within self.level and stor them in self.caves
    for (i32 x {0}; x < _grid.width(); ++x) {
        for (i32 y {0}; y < _grid.height(); ++y) {
            flood_fill({x, y});
        }
    }

    for (auto& set : _caves) {
        for (auto const& tile : set) {
            _grid[tile] = _floor;
        }
    }
}

void cellular_automata::flood_fill(point_i pos)
{
    /*
    flood fill the separate regions of the level, discard
    the regions that are smaller than a minimum size, and
    create a reference for the rest.
    */
    std::unordered_set<i32> cave;
    std::queue<point_i>     toBeFilled {};
    toBeFilled.push(pos);

    while (!toBeFilled.empty()) {
        point_i const tile {toBeFilled.front()};
        toBeFilled.pop();

        if (!cave.contains(get_index(tile))) {
            cave.insert(get_index(tile));
            _grid[tile] = _wall;

            // check adjacent cells
            auto const [x, y] {tile};
            std::array<point_i, 4> const directions {{{x, y - 1}, {x, y + 1}, {x + 1, y}, {x - 1, y}}};
            for (auto const& direction : directions) {
                if (_grid.contains(direction) && tile_traits::passable(_grid[direction])) {
                    if (!cave.contains(get_index(direction))) { toBeFilled.push(direction); }
                }
            }
        }
    }
    if (cave.size() >= _minRoomSize) {
        _caves.push_back(cave);
    }
}

void cellular_automata::connect_caves()
{
    // Find the closest cave to the current cave
    for (auto& currentCave : _caves) {
        i32 const          point1 {*currentCave.begin()};
        std::optional<i32> point2;
        std::optional<f64> distance;

        for (auto& nextCave : _caves) {
            if (nextCave != currentCave && !check_connectivity(currentCave, nextCave)) {
                // choose a random point from nextCave
                i32 const nextPoint {*nextCave.begin()};
                f64 const newDistance {euclidean_distance(get_point(point1), get_point(nextPoint))};
                if (newDistance < distance || !distance) {
                    point2   = nextPoint;
                    distance = newDistance;
                }
            }
        }
        if (point2) { // if all tunnels are connected, point2 == None
            create_tunnel(get_point(point1), get_point(*point2), currentCave);
        }
    }
}

auto cellular_automata::check_connectivity(std::unordered_set<i32> const& cave1, std::unordered_set<i32> const& cave2) const -> bool
{
    // floods cave1, then checks a point in cave2 for the flood
    std::unordered_set<i32> connectedRegion;
    i32 const               start {*cave1.begin()};
    i32 const               end {*cave2.begin()};
    if (start == end) { return true; }

    std::queue<i32> toBeFilled {};
    toBeFilled.push(start);

    while (!toBeFilled.empty()) {
        i32 const tile {toBeFilled.front()};
        toBeFilled.pop();

        if (!connectedRegion.contains(tile)) {
            connectedRegion.insert(tile);

            // check adjacent cells
            auto const [x, y] {get_point(tile)};
            std::array<point_i, 4> const directions {{{x, y - 1}, {x, y + 1}, {x + 1, y}, {x - 1, y}}};

            for (auto const& direction : directions) {
                i32 const dir {get_index(direction)};
                if (dir == end) { return true; }

                if (_grid.contains(direction) && tile_traits::passable(_grid[direction])) {
                    if (!connectedRegion.contains(dir)) { toBeFilled.push(dir); }
                }
            }
        }
    }
    return connectedRegion.contains(end);
}

////////////////////////////////////////////////////////////

city_walls::city_walls(i32 minRoomSize, i32 maxRoomSize)
    : _minRoomSize {minRoomSize}
    , _maxRoomSize {maxRoomSize}
{
}

auto city_walls::generate(u64 seed, size_i size) -> grid<tile>
{
    static i32 constexpr MAX_LEAF_SIZE {30};
    _rooms.clear();

    _rng  = rng {seed};
    _grid = grid<tile> {size, FLOOR};
    auto const [mapWidth, mapHeight] {size};

    _floor = FLOOR;
    _wall  = WALL0;

    leaf root {rect_i {0, 0, mapWidth, mapHeight}};
    root.split_leaf(_rng, MAX_LEAF_SIZE);
    root.create_rooms(
        [&](rect_i const& room1, rect_i const& room2) {
            _rooms.insert(room1);
            _rooms.insert(room2);
        },
        [&](rect_i const& room0) { draw_room(room0); },
        _minRoomSize, _maxRoomSize, _rng);

    draw_doors();

    return _grid;
}

void city_walls::draw_room(rect_i const& rect)
{
    for (i32 y {rect.top() + 1}; y < rect.bottom(); ++y) {
        for (i32 x {rect.left() + 1}; x < rect.right(); ++x) {
            if (x == 0 || y == 0 || x == _grid.width() - 1 || y == _grid.height() - 1) { continue; }

            point_i const pos {x, y};
            assert(_grid.contains(pos));
            _grid[pos] = _wall;
        }
    }
    for (i32 y {rect.top() + 2}; y < rect.bottom() - 1; ++y) {
        for (i32 x {rect.left() + 2}; x < rect.right() - 1; ++x) {
            if (x == 0 || y == 0 || x == _grid.width() - 1 || y == _grid.height() - 1) { continue; }

            point_i const pos {x, y};
            assert(_grid.contains(pos));
            _grid[pos] = _floor;
        }
    }
}

void city_walls::draw_doors()
{
    for (auto const& room : _rooms) {
        auto const [x, y] {point_i {room.center()}};

        point_i wallPos;

        i32 const wall {_rng(0, 3)};
        switch (wall) {
        case 0: wallPos = {x, room.top() + 1}; break;
        case 1: wallPos = {x, room.bottom() - 1}; break;
        case 2: wallPos = {room.right() - 1, y}; break;
        case 3: wallPos = {room.left() + 1, y}; break;
        }

        _grid[wallPos] = _floor;
    }
}

////////////////////////////////////////////////////////////

maze_with_rooms::maze_with_rooms(i32 minRoomSize, i32 maxRoomSize, i32 buildRoomAttempts, f32 connectionChance, f32 windingPercent, bool allowDeadEnds)
    : _minRoomSize {minRoomSize}
    , _maxRoomSize {maxRoomSize}
    , _buildRoomAttempts {buildRoomAttempts}
    , _connectionChance {connectionChance}
    , _windingPercent {windingPercent}
    , _allowDeadEnds {allowDeadEnds}
{
}

auto maze_with_rooms::generate(u64 seed, size_i size) -> grid<tile>
{
    if (size.Width % 2 == 0) { --size.Width; }
    if (size.Height % 2 == 0) { --size.Height; }

    _rng  = rng {seed};
    _grid = grid<tile> {size};
    fill_grid(_grid, _rng, std::array {WALL0, WALL1, WALL2});

    _currentRegion = -1;
    _regions       = grid<std::optional<i32>> {size, std::nullopt};

    auto const [mapWidth, mapHeight] {size};

    _floor = FLOOR;
    _wall  = WALL0;

    add_rooms();

    // Fill in the empty space around the rooms with mazes
    for (i32 y {1}; y < mapHeight; y += 2) {
        for (i32 x {1}; x < mapWidth; x += 2) {
            point_i const pos {x, y};
            if (tile_traits::passable(_grid[pos])) { continue; }
            grow_maze(pos);
        }
    }
    connect_regions();
    if (!_allowDeadEnds) { remove_dead_ends(); }

    return _grid;
}

void maze_with_rooms::grow_maze(point_i start)
{
    std::array<point_i, 4> const directions {{{0, -1}, {0, 1}, {1, 0}, {-1, 0}}};

    std::vector<point_i>   cells;
    std::optional<point_i> lastDirection;

    start_region();
    carve(start);

    cells.push_back(start);

    while (!cells.empty()) {
        point_i const               cell {cells.back()};
        std::unordered_set<point_i> unmadeCells;
        for (auto const& direction : directions) {
            if (can_carve(cell, direction)) {
                unmadeCells.insert(direction);
            }
        }
        if (!unmadeCells.empty()) {
            // Prefer to carve in the same direction, when
            // it isn't necessary to do otherwise.
            point_i direction;
            if (lastDirection && unmadeCells.contains(*lastDirection) && _rng(0.0f, 1.0f) > _windingPercent) {
                direction = *lastDirection;
            } else {
                direction = *unmadeCells.begin();
                unmadeCells.erase(unmadeCells.begin());
            }
            point_i newCell {cell + direction};
            carve(newCell);

            newCell = {cell + (direction * 2)};
            carve(newCell);

            cells.push_back(newCell);
            lastDirection = direction;
        } else {
            cells.pop_back();
            lastDirection = std::nullopt;
        }
    }
}

void maze_with_rooms::add_rooms()
{
    std::vector<rect_i> rooms;

    for (i32 i {0}; i < _buildRoomAttempts; ++i) {
        // Pick a random room size and ensure that rooms have odd
        // dimensions and that rooms are not too narrow.
        i32 const    roomWidth {_rng((_minRoomSize / 2), (_maxRoomSize / 2)) * 2 + 1};
        i32 const    roomHeight {_rng((_minRoomSize / 2), (_maxRoomSize / 2)) * 2 + 1};
        i32 const    x {(_rng(0, _grid.width() - roomWidth - 1) / 2) * 2 + 1};
        i32 const    y {(_rng(0, _grid.height() - roomHeight - 1) / 2) * 2 + 1};
        rect_i const room {x, y, roomWidth, roomHeight};

        // check for overlap with previous rooms
        bool failed {false};
        for (auto const& outerRoom : rooms) {
            if (room.intersects(outerRoom, true)) {
                failed = true;
                break;
            }
        }
        if (!failed) {
            rooms.push_back(room);
            start_region();
            create_room(room);
        }
    }
}

template <typename T>
void difference_update(std::unordered_set<T>& a, std::unordered_set<T> const& b)
{
    for (auto it {a.begin()}; it != a.end();) {
        if (b.contains(*it)) {
            it = a.erase(it); // erase returns the next iterator
        } else {
            ++it;
        }
    }
}

void maze_with_rooms::connect_regions()
{
    // Find all of the tiles that can connect two regions
    std::array<point_i, 4> const directions {{{0, -1}, {0, 1}, {1, 0}, {-1, 0}}};

    grid<std::unordered_set<i32>> connectorRegions {_regions.size()};
    auto const [mapWidth, mapHeight] {_regions.size()};

    for (i32 x {1}; x < mapWidth - 1; ++x) {
        for (i32 y {1}; y < mapHeight - 1; ++y) {
            if (tile_traits::passable(_grid[{x, y}])) { continue; }
            std::unordered_set<i32> regions;
            for (auto const& direction : directions) {
                point_i const newPos {x + direction.X, y + direction.Y};
                auto const    region {_regions[newPos]};
                if (region) {
                    regions.insert(*region);
                }
            }
            if (regions.size() < 2) { continue; }
            connectorRegions[{x, y}] = regions;
        }
    }

    // make a list of all of the connectors
    std::unordered_set<point_i> connectors;
    for (i32 x {0}; x < mapWidth; ++x) {
        for (i32 y {0}; y < mapHeight; ++y) {
            if (!connectorRegions[{x, y}].empty()) {
                connectors.insert({x, y});
            }
        }
    }

    // keep track of the regions that have been merged.
    std::unordered_map<i32, i32> merged;
    std::unordered_set<i32>      openRegions;
    for (i32 i {0}; i <= _currentRegion; ++i) {
        merged[i] = i;
        openRegions.insert(i);
    }

    while (openRegions.size() > 1) {
        point_i connector {*connectors.begin()};
        // carve the connection
        add_junction(connector);

        // make a list of the regions at (x,y)
        auto [x, y] {connector};
        std::vector<i32> regions;
        for (i32 n : connectorRegions[{x, y}]) {
            regions.push_back(merged[n]);
        }

        i32            dest {regions[0]};
        std::span<i32> sources {regions.begin() + 1, regions.size() - 1};
        /*
        Merge all of the effective regions. You must look
        at all of the regions, as some regions may have
        previously been merged with the ones we are
        connecting now.
        */
        for (i32 i {0}; i <= _currentRegion; ++i) {
            if (std::find(sources.begin(), sources.end(), merged[i]) != sources.end()) {
                merged[i] = dest;
            }
        }

        // clear the sources, they are no longer needed
        for (i32 s : sources) {
            openRegions.erase(s);
        }

        // remove the unneeded connectors
        std::unordered_set<point_i> toBeRemoved;
        for (auto const& pos : connectors) {
            // remove connectors that are next to the current connector
            if (euclidean_distance(connector, pos) < 2) {
                toBeRemoved.insert(pos);
                continue;
            }

            std::unordered_set<i32> regions;
            for (i32 n : connectorRegions[pos]) {
                regions.insert(merged[n]);
            }
            if (regions.size() > 1) { continue; }

            if (_rng(0.0f, 1.0f) < _connectionChance) {
                add_junction(pos);
            }

            if (regions.size() == 1) {
                toBeRemoved.insert(pos);
            }
        }

        difference_update(connectors, toBeRemoved);
    }
}

void maze_with_rooms::create_room(rect_i const& room)
{
    for (i32 x {room.left()}; x < room.right(); ++x) {
        for (i32 y {room.top()}; y < room.bottom(); ++y) {
            carve({x, y});
        }
    }
}

void maze_with_rooms::add_junction(point_i pos)
{
    _grid[pos] = _floor;
}

void maze_with_rooms::remove_dead_ends()
{
    bool                         done {false};
    std::array<point_i, 4> const directions {{{0, -1}, {0, 1}, {1, 0}, {-1, 0}}};

    while (!done) {
        done = true;
        for (i32 x {1}; x < _grid.width(); ++x) {
            for (i32 y {1}; y < _grid.height(); ++y) {
                point_i const pos {x, y};
                if (tile_traits::passable(_grid[pos])) {
                    i32 exits {0};
                    for (auto const& direction : directions) {
                        if (tile_traits::passable(_grid[pos + direction])) {
                            ++exits;
                        }
                    }
                    if (exits > 1) { continue; }
                    done       = false;
                    _grid[pos] = _wall;
                }
            }
        }
    }
}

auto maze_with_rooms::can_carve(point_i pos, point_i dir) -> bool
{

    /*
    gets whether an opening can be carved at the location
    adjacent to the cell at (pos) in the (dir) direction.
    returns False if the location is out of bounds or if the cell
    is already open.
    */

    i32 x = pos.X + dir.X * 3;
    i32 y = pos.Y + dir.Y * 3;

    if (!_grid.contains({x, y})) { return false; }

    x = pos.X + dir.X * 2;
    y = pos.Y + dir.Y * 2;

    if (!_grid.contains({x, y})) { return false; }

    // return True if the cell is a wall (1)
    // false if the cell is a floor (0)
    return !tile_traits::passable(_grid[{x, y}]);
}

void maze_with_rooms::start_region()
{
    ++_currentRegion;
}

void maze_with_rooms::carve(point_i pos)
{
    _grid[pos]    = _floor;
    _regions[pos] = _currentRegion;
}

////////////////////////////////////////////////////////////

messy_bsp_tree::messy_bsp_tree(i32 minRoomSize, i32 maxRoomSize, bool smoothEdges, i32 smoothing, i32 filling)
    : _minRoomSize {minRoomSize}
    , _maxRoomSize {maxRoomSize}
    , _smoothEdges {smoothEdges}
    , _smoothing {smoothing}
    , _filling {filling}
{
}

auto messy_bsp_tree::generate(u64 seed, size_i size) -> grid<tile>
{
    static i32 constexpr MAX_LEAF_SIZE {24};

    _rng  = rng {seed};
    _grid = grid<tile> {size};
    auto const [mapWidth, mapHeight] {size};
    fill_grid(_grid, _rng, std::array {WALL0, WALL1, WALL2});

    _floor = FLOOR;
    _wall  = WALL0;

    leaf root {rect_i {0, 0, mapWidth, mapHeight}};
    root.split_leaf(_rng, MAX_LEAF_SIZE);
    root.create_rooms(
        [&](rect_i const& room1, rect_i const& room2) { draw_hallway(room1, room2); },
        [&](rect_i const& room0) { draw_room(room0); },
        _minRoomSize, _maxRoomSize, _rng);
    cleanup_map();

    return _grid;
}

void messy_bsp_tree::draw_room(rect_i const& rect)
{
    for (i32 y {rect.top() + 1}; y < rect.bottom(); ++y) {
        for (i32 x {rect.left() + 1}; x < rect.right(); ++x) {
            if (x == 0 || y == 0 || x == _grid.width() - 1 || y == _grid.height() - 1) { continue; }

            point_i const pos {x, y};
            assert(_grid.contains(pos));
            _grid[pos] = _floor;
        }
    }
}

void messy_bsp_tree::draw_hallway(rect_i const& room1, rect_i const& room2)
{
    // run a heavily weighted random Walk
    // from point2 to point1
    auto [drunkardX, drunkardY] {point_i {room2.center()}};
    auto [goalX, goalY] {point_i {room1.center()}};

    while (!room1.contains(point_i {drunkardX, drunkardY})) {
        // ==== Choose Direction ====
        f32 up {1.0f};
        f32 down {1.0f};
        f32 right {1.0f};
        f32 left {1.0f};

        f32 weight {1};

        // weight the random walk against edges
        if (drunkardX < goalX) {        // drunkard is left of point1
            right += weight;
        } else if (drunkardX > goalX) { // drunkard is right of point1
            left += weight;
        }
        if (drunkardY < goalY) {        // drunkard is above point1
            down += weight;
        } else if (drunkardY > goalY) { // drunkard is below point1
            up += weight;
        }

        // normalize probabilities so they form a range from 0 to 1
        f32 const total {up + down + right + left};
        up /= total;
        down /= total;
        right /= total;
        left /= total;

        // choose the direction
        i32 dx {0};
        i32 dy {0};
        f32 choice = _rng(0.0f, 1.0f);
        if (choice < up) {
            dx = 0;
            dy = -1;
        } else if (choice < (up + down)) {
            dx = 0;
            dy = 1;
        } else if (choice < (up + down + right)) {
            dx = 1;
            dy = 0;
        } else {
            dx = -1;
            dy = 0;
        }
        // ==== Walk ====
        // check colision at edges
        if ((drunkardX + dx > 0 && drunkardX + dx < _grid.width() - 1) && (drunkardY + dy > 0 && drunkardY + dy < _grid.height() - 1)) {
            drunkardX += dx;
            drunkardY += dy;
            if (!tile_traits::passable(_grid[{drunkardX, drunkardY}])) {
                _grid[{drunkardX, drunkardY}] = _floor;
            }
        }
    }
}

void messy_bsp_tree::cleanup_map()
{
    if (!_smoothEdges) { return; }
    for (i32 i {0}; i < 3; ++i) {
        for (i32 x {1}; x < _grid.width() - 1; ++x) {
            for (i32 y {1}; y < _grid.height() - 1; ++y) {
                point_i const pos {x, y};
                auto const    walls {get_adjacent_walls_simple(pos)};
                if (tile_traits::passable(_grid[pos]) && walls <= _smoothing) {
                    _grid[pos] = _floor;
                }
                if (!tile_traits::passable(_grid[pos]) && walls >= _filling) {
                    _grid[pos] = _wall;
                }
            }
        }
    }
}

auto messy_bsp_tree::get_adjacent_walls_simple(point_i p) -> i32
{
    i32 retValue {0};
    if (!tile_traits::passable(_grid[{p.X, p.Y - 1}])) { ++retValue; }
    if (!tile_traits::passable(_grid[{p.X, p.Y + 1}])) { ++retValue; }
    if (!tile_traits::passable(_grid[{p.X - 1, p.Y}])) { ++retValue; }
    if (!tile_traits::passable(_grid[{p.X + 1, p.Y}])) { ++retValue; }
    return retValue;
}

////////////////////////////////////////////////////////////

leaf::leaf(rect_i const& rect)
    : Rect {rect}
{
}

void leaf::split_leaf(rng& rng, i32 maxRoomSize)
{
    static i32 constexpr MIN_LEAF_SIZE {10};

    // Determine if this leaf is eligible for splitting based on its dimensions or a random chance.
    if ((Rect.width() > maxRoomSize) || (Rect.height() > maxRoomSize) || (rng(0, 100) > 80)) {
        // Determine the split direction.
        bool splitHorizontally {};
        if (Rect.width() / static_cast<f32>(Rect.height()) >= 1.25f) {
            splitHorizontally = false;
        } else if (Rect.height() / static_cast<f32>(Rect.width()) >= 1.25f) {
            splitHorizontally = true;
        } else {
            splitHorizontally = rng(0, 1) == 0;
        }

        // Calculate the maximum split value ensuring a minimum leaf size remains.
        i32 const max {splitHorizontally
                           ? Rect.height() - MIN_LEAF_SIZE
                           : Rect.width() - MIN_LEAF_SIZE};
        if (max <= MIN_LEAF_SIZE) { return; } // Too small to split further
        i32 const split {rng(MIN_LEAF_SIZE, max)};

        // Create children based on the chosen split direction.
        if (splitHorizontally) {
            Child1 = std::make_unique<leaf>(rect_i {Rect.left(), Rect.top(), Rect.width(), split});
            Child2 = std::make_unique<leaf>(rect_i {Rect.left(), Rect.top() + split, Rect.width(), Rect.height() - split});
        } else {
            Child1 = std::make_unique<leaf>(rect_i {Rect.left(), Rect.top(), split, Rect.height()});
            Child2 = std::make_unique<leaf>(rect_i {Rect.left() + split, Rect.top(), Rect.width() - split, Rect.height()});
        }

        // After a successful split, recursively attempt to split the children.
        Child1->split_leaf(rng, maxRoomSize);
        Child2->split_leaf(rng, maxRoomSize);
    }
}

void leaf::create_rooms(hall_func&& hall, room_func&& room, i32 minRoomSize, i32 maxRoomSize, rng& rng)
{
    if (Child1 || Child2) {
        // Recursively search for children until you hit the end of the branch
        if (Child1) {
            Child1->create_rooms(std::forward<hall_func>(hall), std::forward<room_func>(room), minRoomSize, maxRoomSize, rng);
        }
        if (Child2) {
            Child2->create_rooms(std::forward<hall_func>(hall), std::forward<room_func>(room), minRoomSize, maxRoomSize, rng);
        }

        if (Child1 && Child2) {
            hall(Child1->get_room(rng), Child2->get_room(rng));
        }
    } else {
        // Create rooms in the end branches of the BSP tree
        i32 const w {rng(minRoomSize, std::min(maxRoomSize, Rect.width() - 1))};
        i32 const h {rng(minRoomSize, std::min(maxRoomSize, Rect.height() - 1))};
        i32 const x {rng(Rect.left(), Rect.left() + Rect.width() - 1 - w)};
        i32 const y {rng(Rect.top(), Rect.top() + (Rect.height() - 1) - h)};
        _room = {x, y, w, h};

        room(*_room);
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
