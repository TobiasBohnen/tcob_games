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
    void fill_grid(grid<tile>& grid, rng& rng, std::span<tile const> tiles) const;

    void draw_room(grid<tile>& grid, rect_i const& rect, tile const& floor);
    void draw_hallway(grid<tile>& grid, rect_i const& room1, rect_i const& room2, tile const& floor, bool b);

private:
    void draw_horizontal_tunnel(grid<tile>& grid, i32 x0, i32 x1, i32 y, tile const& floor);
    void draw_vertical_tunnel(grid<tile>& grid, i32 x, i32 y0, i32 y1, tile const& floor);
};

////////////////////////////////////////////////////////////

class tunneling : public base_layout {
public:
    tunneling(i32 maxRooms, i32 minRoomSize, i32 maxRoomSize);

    auto generate(u64 seed, size_i size) -> grid<tile> override;

private:
    i32 _maxRooms;
    i32 _minRoomSize;
    i32 _maxRoomSize;
};

////////////////////////////////////////////////////////////

class bsp_tree : public base_layout {
public:
    bsp_tree(i32 minRoomSize, i32 maxRoomSize);

    auto generate(u64 seed, size_i size) -> grid<tile> override;

private:
    i32 _minRoomSize;
    i32 _maxRoomSize;
};

////////////////////////////////////////////////////////////

class drunkards_walk : public base_layout {
public:
    drunkards_walk(f32 percentGoal, i32 walkIterations, f32 weightedTowardCenter, f32 weightedTowardPreviousDirection);

    auto generate(u64 seed, size_i size) -> grid<tile> override;

private:
    void walk(rng& rng);

    f32 _percentGoal;
    i32 _walkIterations;
    f32 _weightedTowardCenter;
    f32 _weightedTowardPreviousDirection;

    i32       _filled {0};
    f32       _filledGoal {0};
    direction _previousDirection {direction::None};
    i32       _drunkardX {0};
    i32       _drunkardY {0};

    tile _floor;

    grid<tile> _grid;
};

////////////////////////////////////////////////////////////

class cellular_automata : public base_layout {
public:
    cellular_automata(i32 iterations, i32 neighbors, f32 wallProbability, i32 minRoomSize, i32 maxRoomSize, bool smoothEdges, i32 smoothing);

    auto generate(u64 seed, size_i size) -> grid<tile> override;

private:
    void random_fill_map();
    void draw_caves();
    void cleanup_map();
    void create_tunnel(point_i point1, point_i point2, std::unordered_set<point_i>& currentCave);
    auto get_adjacent_walls(point_i p) -> i32;
    auto get_adjacent_walls_simple(point_i p) -> i32;
    void get_caves();
    void flood_fill(point_i pos);
    void connect_caves();
    auto check_connectivity(std::unordered_set<point_i>& cave1, std::unordered_set<point_i>& cave2) const -> bool;

    i32  _iterations;
    i32  _neighbors;
    f32  _wallProbability;
    i32  _minRoomSize;
    i32  _maxRoomSize;
    bool _smoothEdges;
    i32  _smoothing;

    tile _floor;
    tile _wall;

    std::vector<std::unordered_set<point_i>> _caves;
    grid<tile>                               _grid;
    rng                                      _rng;
};

////////////////////////////////////////////////////////////

class room_addition : public base_layout { }; // TODO
/*class RoomAddition:
        '''
        What I'm calling the Room Addition algorithm is an attempt to
        recreate the dungeon generation algorithm used in Brogue, as
        discussed at https://www.rockpapershotgun.com/2015/07/28/how-do-roguelikes-generate-levels/
        I don't think Brian Walker has ever given a name to his
        dungeon generation algorithm, so I've taken to calling it the
        Room Addition Algorithm, after the way in which it builds the
        dungeon by adding rooms one at a time to the existing dungeon.
        This isn't a perfect recreation of Brian Walker's algorithm,
        but I think it's good enough to demonstrait the concept.
        '''
        def __init__(self):
                self.level = []

                self.ROOM_MAX_SIZE = 18 # max height and width for cellular automata rooms
                self.ROOM_MIN_SIZE = 16 # min size in number of floor tiles, not height and width
                self.MAX_NUM_ROOMS = 30

                self.SQUARE_ROOM_MAX_SIZE = 12
                self.SQUARE_ROOM_MIN_SIZE = 6

                self.CROSS_ROOM_MAX_SIZE = 12
                self.CROSS_ROOM_MIN_SIZE = 6

                self.cavernChance = 0.40 # probability that the first room will be a cavern
                self.CAVERN_MAX_SIZE = 35 # max height an width

                self.wallProbability = 0.45
                self.neighbors = 4

                self.squareRoomChance = 0.2
                self.crossRoomChance = 0.15

                self.buildRoomAttempts = 500
                self.placeRoomAttempts = 20
                self.maxTunnelLength = 12

                self.includeShortcuts = True
                self.shortcutAttempts = 500
                self.shortcutLength = 5
                self.minPathfindingDistance = 50

        def generateLevel(self,mapWidth,mapHeight):
                self.rooms = []

                self.level = [[1
                        for y in range(mapHeight)]
                                for x in range(mapWidth)]

                # generate the first room
                room = self.generateRoom()
                roomWidth,roomHeight = self.getRoomDimensions(room)
                roomX = (mapWidth/2 - roomWidth/2)-1
                roomY = (mapHeight/2 - roomHeight/2)-1
                self.addRoom(roomX,roomY,room)

                # generate other rooms
                for i in range(self.buildRoomAttempts):
                        room = self.generateRoom()
                        # try to position the room, get roomX and roomY
                        roomX,roomY,wallTile,direction, tunnelLength = self.placeRoom(room,mapWidth,mapHeight)
                        if roomX and roomY:
                                self.addRoom(roomX,roomY,room)
                                self.addTunnel(wallTile,direction,tunnelLength)
                                if len(self.rooms) >= self.MAX_NUM_ROOMS:
                                        break

                if self.includeShortcuts == True:
                        self.addShortcuts(mapWidth,mapHeight)

                return self.level

        def generateRoom(self):
                # select a room type to generate
                # generate and return that room
                if self.rooms:
                        #There is at least one room already
                        choice = random.random()

                        if choice <self.squareRoomChance:
                                room = self.generateRoomSquare()
                        elif self.squareRoomChance <= choice < (self.squareRoomChance+self.crossRoomChance):
                                room = self.generateRoomCross()
                        else:
                                room = self.generateRoomCellularAutomata()

                else: #it's the first room
                        choice = random.random()
                        if choice < self.cavernChance:
                                room = self.generateRoomCavern()
                        else:
                                room = self.generateRoomSquare()

                return room

        def generateRoomCross(self):
                roomHorWidth = (random.randint(self.CROSS_ROOM_MIN_SIZE+2,self.CROSS_ROOM_MAX_SIZE))/2*2

                roomVirHeight = (random.randint(self.CROSS_ROOM_MIN_SIZE+2,self.CROSS_ROOM_MAX_SIZE))/2*2

                roomHorHeight = (random.randint(self.CROSS_ROOM_MIN_SIZE,roomVirHeight-2))/2*2

                roomVirWidth = (random.randint(self.CROSS_ROOM_MIN_SIZE,roomHorWidth-2))/2*2

                room = [[1
                        for y in xrange(roomVirHeight)]
                                for x in xrange(roomHorWidth)]

                # Fill in horizontal space
                virOffset = roomVirHeight/2 - roomHorHeight/2
                for y in xrange(virOffset,roomHorHeight+virOffset):
                        for x in xrange(0,roomHorWidth):
                                room[x][y] = 0

                # Fill in virtical space
                horOffset = roomHorWidth/2 - roomVirWidth/2
                for y in xrange(0,roomVirHeight):
                        for x in xrange(horOffset,roomVirWidth+horOffset):
                                room[x][y] = 0

                return room

        def generateRoomSquare(self):
                roomWidth = random.randint(self.SQUARE_ROOM_MIN_SIZE,self.SQUARE_ROOM_MAX_SIZE)
                roomHeight = random.randint(max(int(roomWidth*0.5),self.SQUARE_ROOM_MIN_SIZE),min(int(roomWidth*1.5),self.SQUARE_ROOM_MAX_SIZE))

                room = [[1
                        for y in range(roomHeight)]
                                for x in range(roomWidth)]

                room = [[0
                        for y in range(1,roomHeight-1)]
                                for x in range(1,roomWidth-1)]

                return room

        def generateRoomCellularAutomata(self):
                while True:
                        # if a room is too small, generate another
                        room = [[1
                                for y in range(self.ROOM_MAX_SIZE)]
                                        for x in range(self.ROOM_MAX_SIZE)]

                        # random fill map
                        for y in range (2,self.ROOM_MAX_SIZE-2):
                                for x in range (2,self.ROOM_MAX_SIZE-2):
                                        if random.random() >= self.wallProbability:
                                                room[x][y] = 0

                        # create distinctive regions
                        for i in range(4):
                                for y in range (1,self.ROOM_MAX_SIZE-1):
                                        for x in range (1,self.ROOM_MAX_SIZE-1):

                                                # if the cell's neighboring walls > self.neighbors, set it to 1
                                                if self.getAdjacentWalls(x,y,room) > self.neighbors:
                                                        room[x][y] = 1
                                                # otherwise, set it to 0
                                                elif self.getAdjacentWalls(x,y,room) < self.neighbors:
                                                        room[x][y] = 0

                        # floodfill to remove small caverns
                        room = self.floodFill(room)

                        # start over if the room is completely filled in
                        roomWidth,roomHeight = self.getRoomDimensions(room)
                        for x in range (roomWidth):
                                for y in range (roomHeight):
                                        if room[x][y] == 0:
                                                return room

        def generateRoomCavern(self):
                while True:
                        # if a room is too small, generate another
                        room = [[1
                                for y in range(self.CAVERN_MAX_SIZE)]
                                        for x in range(self.CAVERN_MAX_SIZE)]

                        # random fill map
                        for y in range (2,self.CAVERN_MAX_SIZE-2):
                                for x in range (2,self.CAVERN_MAX_SIZE-2):
                                        if random.random() >= self.wallProbability:
                                                room[x][y] = 0

                        # create distinctive regions
                        for i in range(4):
                                for y in range (1,self.CAVERN_MAX_SIZE-1):
                                        for x in range (1,self.CAVERN_MAX_SIZE-1):

                                                # if the cell's neighboring walls > self.neighbors, set it to 1
                                                if self.getAdjacentWalls(x,y,room) > self.neighbors:
                                                        room[x][y] = 1
                                                # otherwise, set it to 0
                                                elif self.getAdjacentWalls(x,y,room) < self.neighbors:
                                                        room[x][y] = 0

                        # floodfill to remove small caverns
                        room = self.floodFill(room)

                        # start over if the room is completely filled in
                        roomWidth,roomHeight = self.getRoomDimensions(room)
                        for x in range (roomWidth):
                                for y in range (roomHeight):
                                        if room[x][y] == 0:
                                                return room

        def floodFill(self,room):
                '''
                Find the largest region. Fill in all other regions.
                '''
                roomWidth,roomHeight = self.getRoomDimensions(room)
                largestRegion = set()

                for x in range (roomWidth):
                        for y in range (roomHeight):
                                if room[x][y] == 0:
                                        newRegion = set()
                                        tile = (x,y)
                                        toBeFilled = set([tile])
                                        while toBeFilled:
                                                tile = toBeFilled.pop()

                                                if tile not in newRegion:
                                                        newRegion.add(tile)

                                                        room[tile[0]][tile[1]] = 1

                                                        # check adjacent cells
                                                        x = tile[0]
                                                        y = tile[1]
                                                        north = (x,y-1)
                                                        south = (x,y+1)
                                                        east = (x+1,y)
                                                        west = (x-1,y)

                                                        for direction in [north,south,east,west]:

                                                                if room[direction[0]][direction[1]] == 0:
                                                                        if direction not in toBeFilled and direction not in newRegion:
                                                                                toBeFilled.add(direction)

                                        if len(newRegion) >= self.ROOM_MIN_SIZE:
                                                if len(newRegion) > len(largestRegion):
                                                        largestRegion.clear()
                                                        largestRegion.update(newRegion)

                for tile in largestRegion:
                        room[tile[0]][tile[1]] = 0

                return room

        def placeRoom(self,room, mapWidth, mapHeight): #(self,room,direction,)
                roomX = None
                roomY = None

                roomWidth, roomHeight = self.getRoomDimensions(room)

                # try n times to find a wall that lets you build room in that direction
                for i in xrange(self.placeRoomAttempts):
                        # try to place the room against the tile, else connected by a tunnel of length i

                        wallTile = None
                        direction = self.getDirection()
                        while not wallTile:
                                '''
                                randomly select tiles until you find
                                a wall that has another wall in the
                                chosen direction and has a floor in the
                                opposite direction.
                                '''
                                #direction == tuple(dx,dy)
                                tileX = random.randint(1,mapWidth-2)
                                tileY = random.randint(1,mapHeight-2)
                                if ((self.level[tileX][tileY] == 1) and
                                        (self.level[tileX+direction[0]][tileY+direction[1]] == 1) and
                                        (self.level[tileX-direction[0]][tileY-direction[1]] == 0)):
                                        wallTile = (tileX,tileY)

                        #spawn the room touching wallTile
                        startRoomX = None
                        startRoomY = None
                        '''
                        TODO: replace this with a method that returns a
                        random floor tile instead of the top left floor tile
                        '''
                        while not startRoomX and not startRoomY:
                                x = random.randint(0,roomWidth-1)
                                y =  random.randint(0,roomHeight-1)
                                if room[x][y] == 0:
                                        startRoomX = wallTile[0] - x
                                        startRoomY = wallTile[1] - y

                        #then slide it until it doesn't touch anything
                        for tunnelLength in xrange(self.maxTunnelLength):
                                possibleRoomX = startRoomX + direction[0]*tunnelLength
                                possibleRoomY = startRoomY + direction[1]*tunnelLength

                                enoughRoom = self.getOverlap(room,possibleRoomX,possibleRoomY,mapWidth,mapHeight)

                                if enoughRoom:
                                        roomX = possibleRoomX
                                        roomY = possibleRoomY

                                        # build connecting tunnel
                                        #Attempt 1
                                        '''
                                        for i in range(tunnelLength+1):
                                                x = wallTile[0] + direction[0]*i
                                                y = wallTile[1] + direction[1]*i
                                                self.level[x][y] = 0
                                        '''
                                        # moved tunnel code into self.generateLevel()

                                        return roomX,roomY, wallTile, direction, tunnelLength

                return None, None, None, None, None

        def addRoom(self,roomX,roomY,room):
                roomWidth,roomHeight = self.getRoomDimensions(room)
                for x in range (roomWidth):
                        for y in range (roomHeight):
                                if room[x][y] == 0:
                                        self.level[roomX+x][roomY+y] = 0

                self.rooms.append(room)

        def addTunnel(self,wallTile,direction,tunnelLength):
                # carve a tunnel from a point in the room back to
                # the wall tile that was used in its original placement

                startX = wallTile[0] + direction[0]*tunnelLength
                startY = wallTile[1] + direction[1]*tunnelLength
                #self.level[startX][startY] = 1

                for i in range(self.maxTunnelLength):
                        x = startX - direction[0]*i
                        y = startY - direction[1]*i
                        self.level[x][y] = 0
                        # If you want doors, this is where the code should go
                        if ((x+direction[0]) == wallTile[0] and
                                (y+direction[1]) == wallTile[1]):
                                break

        def getRoomDimensions(self,room):
                if room:
                        roomWidth = len(room)
                        roomHeight = len(room[0])
                        return roomWidth, roomHeight
                else:
                        roomWidth = 0
                        roomHeight = 0
                        return roomWidth, roomHeight

        def getAdjacentWalls(self, tileX, tileY, room): # finds the walls in 8 directions
                wallCounter = 0
                for x in range (tileX-1, tileX+2):
                        for y in range (tileY-1, tileY+2):
                                if (room[x][y] == 1):
                                        if (x != tileX) or (y != tileY): # exclude (tileX,tileY)
                                                wallCounter += 1
                return wallCounter

        def getDirection(self):
                # direction = (dx,dy)
                north = (0,-1)
                south = (0,1)
                east = (1,0)
                west = (-1,0)

                direction = random.choice([north,south,east,west])
                return direction

        def getOverlap(self,room,roomX,roomY,mapWidth,mapHeight):
                '''
                for each 0 in room, check the cooresponding tile in
                self.level and the eight tiles around it. Though slow,
                that should insure that there is a wall between each of
                the rooms created in this way.
                <> check for overlap with self.level
                <> check for out of bounds
                '''
                roomWidth, roomHeight = self.getRoomDimensions(room)
                for x in range(roomWidth):
                        for y in range(roomHeight):
                                if room[x][y] == 0:
                                        # Check to see if the room is out of bounds
                                        if ((1 <= (x+roomX) < mapWidth-1) and
                                                (1 <= (y+roomY) < mapHeight-1)):
                                                #Check for overlap with a one tile buffer
                                                if self.level[x+roomX-1][y+roomY-1] == 0: # top left
                                                        return False
                                                if self.level[x+roomX][y+roomY-1] == 0: # top center
                                                        return False
                                                if self.level[x+roomX+1][y+roomY-1] == 0: # top right
                                                        return False

                                                if self.level[x+roomX-1][y+roomY] == 0: # left
                                                        return False
                                                if self.level[x+roomX][y+roomY] == 0: # center
                                                        return False
                                                if self.level[x+roomX+1][y+roomY] == 0: # right
                                                        return False

                                                if self.level[x+roomX-1][y+roomY+1] == 0: # bottom left
                                                        return False
                                                if self.level[x+roomX][y+roomY+1] == 0: # bottom center
                                                        return False
                                                if self.level[x+roomX+1][y+roomY+1] == 0: # bottom right
                                                        return False

                                        else: #room is out of bounds
                                                return False
                return True

        def addShortcuts(self,mapWidth,mapHeight):
                '''
                I use libtcodpy's built in pathfinding here, since I'm
                already using libtcodpy for the iu. At the moment,
                the way I find the distance between
                two points to see if I should put a shortcut there
                is horrible, and its easily the slowest part of this
                algorithm. If I think of a better way to do this in
                the future, I'll implement it.
                '''


                #initialize the libtcodpy map
                libtcodMap = libtcod.map_new(mapWidth,mapHeight)
                self.recomputePathMap(mapWidth,mapHeight,libtcodMap)

                for i in xrange(self.shortcutAttempts):
                        # check i times for places where shortcuts can be made
                        while True:
                                #Pick a random floor tile
                                floorX = random.randint(self.shortcutLength+1,(mapWidth-self.shortcutLength-1))
                                floorY = random.randint(self.shortcutLength+1,(mapHeight-self.shortcutLength-1))
                                if self.level[floorX][floorY] == 0:
                                        if (self.level[floorX-1][floorY] == 1 or
                                                self.level[floorX+1][floorY] == 1 or
                                                self.level[floorX][floorY-1] == 1 or
                                                self.level[floorX][floorY+1] == 1):
                                                break

                        # look around the tile for other floor tiles
                        for x in xrange(-1,2):
                                for y in xrange(-1,2):
                                        if x != 0 or y != 0: # Exclude the center tile
                                                newX = floorX + (x*self.shortcutLength)
                                                newY = floorY + (y*self.shortcutLength)
                                                if self.level[newX][newY] == 0:
                                                # run pathfinding algorithm between the two points
                                                        #back to the libtcodpy nonesense
                                                        pathMap = libtcod.path_new_using_map(libtcodMap)
                                                        libtcod.path_compute(pathMap,floorX,floorY,newX,newY)
                                                        distance = libtcod.path_size(pathMap)

                                                        if distance > self.minPathfindingDistance:
                                                                # make shortcut
                                                                self.carveShortcut(floorX,floorY,newX,newY)
                                                                self.recomputePathMap(mapWidth,mapHeight,libtcodMap)


                # destroy the path object
                libtcod.path_delete(pathMap)

        def recomputePathMap(self,mapWidth,mapHeight,libtcodMap):
                for x in xrange(mapWidth):
                        for y in xrange(mapHeight):
                                if self.level[x][y] == 1:
                                        libtcod.map_set_properties(libtcodMap,x,y,False,False)
                                else:
                                        libtcod.map_set_properties(libtcodMap,x,y,True,True)

        def carveShortcut(self,x1,y1,x2,y2):
                if x1-x2 == 0:
                        # Carve virtical tunnel
                        for y in xrange(min(y1,y2),max(y1,y2)+1):
                                self.level[x1][y] = 0

                elif y1-y2 == 0:
                        # Carve Horizontal tunnel
                        for x in xrange(min(x1,x2),max(x1,x2)+1):
                                self.level[x][y1] = 0

                elif (y1-y2)/(x1-x2) == 1:
                        # Carve NW to SE Tunnel
                        x = min(x1,x2)
                        y = min(y1,y2)
                        while x != max(x1,x2):
                                x+=1
                                self.level[x][y] = 0
                                y+=1
                                self.level[x][y] = 0

                elif (y1-y2)/(x1-x2) == -1:
                        # Carve NE to SW Tunnel
                        x = min(x1,x2)
                        y = max(y1,y2)
                        while x != max(x1,x2):
                                x += 1
                                self.level[x][y] = 0
                                y -= 1
                                self.level[x][y] = 0

        def checkRoomExists(self,room):
                roomWidth, roomHeight = self.getRoomDimensions(room)
                for x in range(roomWidth):
                        for y in range(roomHeight):
                                if room[x][y] == 0:
                                        return True
                return False
*/
////////////////////////////////////////////////////////////

class city_walls : public base_layout {
public:
    city_walls(i32 minRoomSize, i32 maxRoomSize);

    auto generate(u64 seed, size_i size) -> grid<tile> override;

private:
    void draw_room(rect_i const& rect);
    void draw_doors();

    i32 _minRoomSize;
    i32 _maxRoomSize;

    tile _floor;
    tile _wall;

    std::unordered_set<rect_i> _rooms;
    grid<tile>                 _grid;
    rng                        _rng;
};

////////////////////////////////////////////////////////////

class maze_with_rooms : public base_layout {
public:
    maze_with_rooms(i32 minRoomSize, i32 maxRoomSize, i32 buildRoomAttempts, f32 connectionChance, f32 windingPercent, bool allowDeadEnds);

    auto generate(u64 seed, size_i size) -> grid<tile> override;

private:
    void grow_maze(point_i start);
    void add_rooms();
    void connect_regions();
    void create_room(rect_i const& room);
    void add_junction(point_i pos);
    void remove_dead_ends();
    auto can_carve(point_i pos, point_i dir) -> bool;
    void start_region();
    void carve(point_i pos);

    i32  _minRoomSize;
    i32  _maxRoomSize;
    i32  _buildRoomAttempts;
    f32  _connectionChance;
    f32  _windingPercent;
    bool _allowDeadEnds;

    tile _floor;
    tile _wall;

    grid<tile>               _grid;
    rng                      _rng;
    grid<std::optional<i32>> _regions;
    i32                      _currentRegion {-1};
};

////////////////////////////////////////////////////////////

class messy_bsp_tree : public base_layout { }; // TODO
/*class MessyBSPTree:
        '''
        A Binary Space Partition connected by a severely weighted
        drunkards walk algorithm.
        Requires Leaf and Rect classes.
        '''
        def __init__(self):
                        self.level = []
                        self.room = None
                        self.MAX_LEAF_SIZE = 24
                        self.ROOM_MAX_SIZE = 15
                        self.ROOM_MIN_SIZE = 6
                        self.smoothEdges = True
                        self.smoothing = 1
                        self.filling = 3

        def generateLevel(self, mapWidth, mapHeight):
                # Creates an empty 2D array or clears existing array
                self.mapWidth = mapWidth
                self.mapHeight = mapHeight
                self.level = [[1
                        for y in range(mapHeight)]
                                for x in range(mapWidth)]

                self._leafs = []

                rootLeaf = Leaf(0,0,mapWidth,mapHeight)
                self._leafs.append(rootLeaf)

                splitSuccessfully = True
                # loop through all leaves until they can no longer split successfully
                while (splitSuccessfully):
                        splitSuccessfully = False
                        for l in self._leafs:
                                if (l.child_1 == None) and (l.child_2 == None):
                                        if ((l.width > self.MAX_LEAF_SIZE) or
                                        (l.height > self.MAX_LEAF_SIZE) or
                                        (random.random() > 0.8)):
                                                if (l.splitLeaf()): #try to split the leaf
                                                        self._leafs.append(l.child_1)
                                                        self._leafs.append(l.child_2)
                                                        splitSuccessfully = True

                rootLeaf.createRooms(self)
                self.cleanUpMap(mapWidth,mapHeight)

                return self.level

        def createRoom(self, room):
                # set all tiles within a rectangle to 0
                for x in range(room.x1 + 1, room.x2):
                        for y in range(room.y1+1, room.y2):
                                self.level[x][y] = 0

        def createHall(self, room1, room2):
                # run a heavily weighted random Walk
                # from point2 to point1
                drunkardX, drunkardY = room2.center()
                goalX,goalY = room1.center()
                while not (room1.x1 <= drunkardX <= room1.x2) or not (room1.y1 < drunkardY < room1.y2): #
                        # ==== Choose Direction ====
                        north = 1.0
                        south = 1.0
                        east = 1.0
                        west = 1.0

                        weight = 1

                        # weight the random walk against edges
                        if drunkardX < goalX: # drunkard is left of point1
                                east += weight
                        elif drunkardX > goalX: # drunkard is right of point1
                                west += weight
                        if drunkardY < goalY: # drunkard is above point1
                                south += weight
                        elif drunkardY > goalY: # drunkard is below point1
                                north += weight

                        # normalize probabilities so they form a range from 0 to 1
                        total = north+south+east+west
                        north /= total
                        south /= total
                        east /= total
                        west /= total

                        # choose the direction
                        choice = random.random()
                        if 0 <= choice < north:
                                dx = 0
                                dy = -1
                        elif north <= choice < (north+south):
                                dx = 0
                                dy = 1
                        elif (north+south) <= choice < (north+south+east):
                                dx = 1
                                dy = 0
                        else:
                                dx = -1
                                dy = 0

                        # ==== Walk ====
                        # check colision at edges
                        if (0 < drunkardX+dx < self.mapWidth-1) and (0 < drunkardY+dy < self.mapHeight-1):
                                drunkardX += dx
                                drunkardY += dy
                                if self.level[drunkardX][drunkardY] == 1:
                                        self.level[drunkardX][drunkardY] = 0

        def cleanUpMap(self,mapWidth,mapHeight):
                if (self.smoothEdges):
                        for i in xrange (3):
                                # Look at each cell individually and check for smoothness
                                for x in xrange(1,mapWidth-1):
                                        for y in xrange (1,mapHeight-1):
                                                if (self.level[x][y] == 1) and (self.getAdjacentWallsSimple(x,y) <= self.smoothing):
                                                        self.level[x][y] = 0

                                                if (self.level[x][y] == 0) and (self.getAdjacentWallsSimple(x,y) >= self.filling):
                                                        self.level[x][y] = 1

        def getAdjacentWallsSimple(self, x, y): # finds the walls in four directions
                wallCounter = 0
                #print("(",x,",",y,") = ",self.level[x][y])
                if (self.level[x][y-1] == 1): # Check north
                        wallCounter += 1
                if (self.level[x][y+1] == 1): # Check south
                        wallCounter += 1
                if (self.level[x-1][y] == 1): # Check west
                        wallCounter += 1
                if (self.level[x+1][y] == 1): # Check east
                        wallCounter += 1

                return wallCounter*/
////////////////////////////////////////////////////////////

struct leaf {
    using hall_func = std::function<void(rect_i, rect_i)>;
    using room_func = std::function<void(rect_i)>;

    leaf(rect_i const& rect);

    rect_i Rect;

    std::unique_ptr<leaf> Child1 {nullptr};
    std::unique_ptr<leaf> Child2 {nullptr};

    void split_leaf(rng& rng, i32 maxRoomSize);
    void create_rooms(hall_func&& hall, room_func&& room, i32 minRoomSize, i32 maxRoomSize, rng& rng);
    auto get_room(rng& rng) -> rect_i;

private:
    std::optional<rect_i> _room;
};
}
