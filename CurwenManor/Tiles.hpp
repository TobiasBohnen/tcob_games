// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

namespace stn {

/////////////////////////////////////////////////////

struct lighting {
    bool                                               Off {false};
    std::unordered_map<direction, std::pair<i32, i32>> VisionCones;
    std::array<i32, 3>                                 Falloff {};
};

/////////////////////////////////////////////////////

struct tileset {
    gfx::texture*        Texture {};
    data::config::object Set;
};

/////////////////////////////////////////////////////

struct tile {
    std::string Texture {};
    bool        Solid {false};

    void static Serialize(tile const& v, auto&& s)
    {
        s["texture"] = v.Texture;
        s["solid"]   = v.Solid;
    }

    auto static Deserialize(tile& v, auto&& s) -> bool
    {
        return s.try_get(v.Texture, "texture") && s.try_get(v.Solid, "solid");
    }
};

/////////////////////////////////////////////////////

class tilemap {
public:
    tilemap(assets& assets);

    void draw(canvas& canvas);

    void draw_shadow(canvas& canvas, player const& player);

    auto check_solid(point_i pos) const -> bool;

    auto get_offset() const -> point_f;
    void set_offset(point_f pos);

    void make_basic_layout(std::string const& walls, std::string const& floor);
    void set_tiles(std::array<std::array<std::string, MAP_TILES.Width>, MAP_TILES.Height> const& tiles);
    void set_tiles(std::vector<std::vector<std::string>> const& tiles, point_i offset);

    void set_object(point_i pos, std::string const& obj);

    void set_lighting(lighting const& setting);

private:
    auto is_in_cone(player const& player, point_f playerPos, point_f canvasPos) const -> bool;
    void cast_shadow(point_f playerPos, point_f canvasPos, color& target) const;

    assets& _assets;

    point_f _offset {0, -1};

    tileset _tiles;
    tileset _objects;

    std::array<std::array<tile, MAP_TILES.Width>, MAP_TILES.Height> _tilemap;
    std::array<std::array<tile, MAP_TILES.Width>, MAP_TILES.Height> _objmap;

    lighting _lighting;
};

}
