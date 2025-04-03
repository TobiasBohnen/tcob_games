// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Common.hpp"
#include "Layout.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class level {
public:
    level();

    void draw(ui::terminal& term);
    void update(milliseconds deltaTime);

    void key_down(input::key_code kc); // TODO: to StartScene
    void mouse_down(point_i pos);      // TODO: to StartScene
    void mouse_hover(point_i pos);     // TODO: to StartScene

    void move_player(point_i pos);
    void move_player(direction dir);

    auto is_passable(point_i pos) const -> bool;

private:
    void end_turn();

    grid<tile> _tiles;

    point_i _hoveredTile {-1, -1};

    std::vector<object>  _objects;
    std::vector<monster> _monsters;
    player               _player;
    point_i              _pointOfView;

    bool _redraw {true};

    std::queue<std::function<bool()>> _queue;         // TODO: to StartScene
    milliseconds                      _queueTimer {}; // TODO: to StartScene
};
}
