// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Common.hpp"
#include "Layout.hpp"

namespace Rogue {

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

    void draw(ui::terminal& term, point_i offset);

private:
    auto can_move_to(point_i pos) const -> bool;
    void end_turn();
    auto line_of_sight(point_i start, point_i end) const -> bool;

    grid<tile>           _tiles;
    std::vector<object>  _objects;
    std::vector<monster> _monsters;
    player               _player;

    bool _redraw {true};
};
}
