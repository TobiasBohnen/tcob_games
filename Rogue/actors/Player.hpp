// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp"
#include "Actor.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class player : public actor {
public:
    player(master_control& parent, profile profile);

    signal<> FinishedPath;

    void start_turn();
    void update(milliseconds deltaTime);
    auto busy() const -> bool;

    auto try_pickup(std::shared_ptr<item> const& item) -> bool;

    void start_path(std::vector<point_i> const& path);
    auto try_move(point_i pos) -> bool;

    void search();

    auto light_color() const -> tcob::color;
    auto light_range() const -> f32;

    auto position() const -> point_i override;

    void draw(renderer& renderer, point_i center, mode mode);
    void draw_inventory(renderer& renderer, i32 x, i32 y);
    void draw_attributes(renderer& renderer, i32 x, i32 y);

private:
    void do_search();

    auto symbol() const -> string override;
    auto color() const -> tcob::color override;

    auto count_gold() const -> i32;

    milliseconds         _animationTimer {};
    std::vector<point_i> _path;
    point_i              _position;
};

}