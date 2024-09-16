// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep
#include "GameCanvas.hpp"

namespace stn {

/////////////////////////////////////////////////////

struct tileset {
    gfx::texture*        Texture {};
    data::config::object Set;
};

/////////////////////////////////////////////////////

struct tile {
    std::string Texture;
    bool        Solid;

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

private:
    assets& _assets;

    point_f _offset {0, -1};
    tileset _tiles;

    using tile_map = std::array<std::array<tile, MAP_TILES.Width>, MAP_TILES.Height>;
    tile_map _tilemap;
};

}
