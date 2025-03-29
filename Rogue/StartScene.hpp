// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "MainMenu.hpp"

namespace Rogue {

struct tile {
    std::string Floor;
    color       Color {colors::White};
    bool        Passable;
};

struct object {
    std::string Symbol;
    color       Color {colors::White};
    point_i     Position {};
};
struct monster {
    std::string Symbol;
    color       Color {colors::White};
    point_i     Position {};
};
struct player {
    color   Color {colors::White};
    point_i Position {};

    auto get_target(direction dir) const -> point_i;
    void move_to(point_i pos);
};

class level {
public:
    level();

    void move_player(direction dir);
    auto can_move_to(point_i pos) const -> bool;
    void draw(terminal& term, point_i offset);
    void end_turn();

private:
    static_grid<tile, 128, 128> _tiles;
    std::vector<object>         _objects;
    std::vector<monster>        _monsters;
    player                      _player;

    bool _redraw {true};
};

////////////////////////////////////////////////////////////

class start_scene : public scene {

public:
    start_scene(game& game);
    ~start_scene() override;

protected:
    void on_start() override;

    void on_draw_to(gfx::render_target& target) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event const& ev) override;

private:
    std::shared_ptr<main_menu> _mainForm {};

    point_i _mapOffset;
    level   _level;
};

}
