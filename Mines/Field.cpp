// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Field.hpp"

namespace Mines {

// clang-format off
std::array<point_i, 8> neighbours { { 
    { -1, -1 }, { 0,  1 }, { 1, -1 },
    { -1,  0 },            { 1,  0 },
    { -1,  1 }, { 0, -1 }, { 1,  1 } 
} };
// clang-format on

constexpr gfx::tile_index_t TS_NONE {0};
constexpr gfx::tile_index_t TS_MINE {10};
constexpr gfx::tile_index_t TS_CONCEALED {11};
constexpr gfx::tile_index_t TS_SELECTED {12};
constexpr gfx::tile_index_t TS_FLAG {13};
constexpr gfx::tile_index_t TS_MAYBE {14};
constexpr gfx::tile_index_t TS_MINE_EXP {15};

field::field(assets::asset_ptr<gfx::material> const& material)
    : _map {{{
          {1, {"clear"}},
          {2, {"1"}},
          {3, {"2"}},
          {4, {"3"}},
          {5, {"4"}},
          {6, {"5"}},
          {7, {"6"}},
          {8, {"7"}},
          {9, {"8"}},
          {TS_MINE, {"mine"}},
          {TS_CONCEALED, {"concealed"}},
          {TS_SELECTED, {"selected"}},
          {TS_FLAG, {"flag"}},
          {TS_MAYBE, {"maybe"}},
          {TS_MINE_EXP, {"mine-exp"}},
      }}}
{
    _map.Material = material;
}

void field::start(size_i gridSize, i32 windowHeight, u32 mines)
{
    if (mines >= gridSize.Width * gridSize.Height) {
        return;
    }

    _gridSize     = gridSize;
    _windowHeight = windowHeight;
    _map.Grid     = {.TileSize = get_tile_size()};

    _numMines          = mines;
    _numRemainingCells = gridSize.Width * gridSize.Height;
    _firstClick        = true;

    _cells = {};
    _cells.resize(gridSize.Width * gridSize.Height);

    // place mines
    for (u32 i {0}; i < mines;) {
        point_i spot {_rand(0, gridSize.Width - 1), _rand(0, gridSize.Height - 1)};
        auto&   cell {get_cell(spot)};
        if (!cell.IsMine) {
            for (auto& nb : neighbours) {
                point_i nc {spot + nb};
                if (nc.X < gridSize.Width && nc.Y < gridSize.Height && nc.X >= 0 && nc.Y >= 0) {
                    get_cell(nc).NeighborMines++;
                }
            }
            ++i;
            cell.IsMine = true;
        }
    }

    // create tilemap layer
    _map.clear();
    gfx::tilemap_layer             backLayer;
    std::vector<gfx::tile_index_t> numberTiles;
    numberTiles.reserve(_cells.size());
    for (auto& field : _cells) {
        if (field.IsMine) {
            numberTiles.push_back(TS_MINE);
        } else {
            numberTiles.push_back(field.NeighborMines + 1);
        }
    }
    backLayer.Tiles = numberTiles;
    backLayer.Size  = gridSize;
    _layerBack      = _map.add_layer(backLayer);

    gfx::tilemap_layer frontLayer;
    std::vector        vec(_cells.size(), TS_CONCEALED);
    frontLayer.Tiles = vec;
    frontLayer.Size  = gridSize;
    _layerFront      = _map.add_layer(frontLayer);

    _state          = game_state::Running;
    _mouseOverPoint = std::nullopt;
}

void field::process_left_click()
{
    if (_state != game_state::Running || !_mouseOverPoint.has_value()) {
        return;
    }

    auto& cell {get_cell(*_mouseOverPoint)};
    if (cell.Flag != flag_type::Flag) {
        if (cell.IsMine) {
            if (_firstClick) {
                move_mine(*_mouseOverPoint);
            } else {
                _state = game_state::Failure;
                reveal_mines();
                set_tile(_layerFront, *_mouseOverPoint, TS_MINE_EXP);
                return;
            }
        }
        _firstClick = false;

        clear_fields(*_mouseOverPoint);
        while (!_markedForClearing.empty()) {
            auto clearCell {_markedForClearing.top()};
            _markedForClearing.pop();
            clear_fields(clearCell);
        }
    }
}

void field::process_right_click()
{
    if (_state != game_state::Running || !_mouseOverPoint.has_value()) {
        return;
    }

    auto& cell {get_cell(*_mouseOverPoint)};
    if (cell.IsCleared) {
        return;
    }

    switch (cell.Flag) {
    case flag_type::None:
        cell.Flag = flag_type::Flag;
        set_tile(_layerFront, *_mouseOverPoint, TS_FLAG);
        break;
    case flag_type::Flag:
        cell.Flag = flag_type::Maybe;
        set_tile(_layerFront, *_mouseOverPoint, TS_MAYBE);
        break;
    case flag_type::Maybe:
        cell.Flag = flag_type::None;
        set_tile(_layerFront, *_mouseOverPoint, TS_CONCEALED);
        break;
    default:
        break;
    }
}

void field::process_mouse_motion(point_f mouse)
{
    if (_state != game_state::Running) {
        return;
    }

    if (_mouseOverPoint.has_value()) {
        auto& mouseCell {get_cell(*_mouseOverPoint)};
        if (!mouseCell.IsCleared && mouseCell.Flag == flag_type::None) {
            set_tile(_layerFront, *_mouseOverPoint, TS_CONCEALED);
        }
    }

    auto tileSize {_map.Grid->TileSize};
    i32  x {static_cast<i32>((mouse.X - _map.Position->X) / tileSize.Width)};
    i32  y {static_cast<i32>((mouse.Y - _map.Position->Y) / tileSize.Height)};
    if (x >= _gridSize.Width || y >= _gridSize.Height) {
        _mouseOverPoint = std::nullopt;
    } else {
        _mouseOverPoint = {x, y};
    }

    if (_mouseOverPoint.has_value()) {
        auto& mouseCell {get_cell(*_mouseOverPoint)};
        if (!mouseCell.IsCleared && mouseCell.Flag == flag_type::None) {
            set_tile(_layerFront, *_mouseOverPoint, TS_SELECTED);
        }
    }
}

void field::on_update(milliseconds deltaTime)
{
    _map.update(deltaTime);
}

void field::on_fixed_update(milliseconds deltaTime)
{
}

void field::on_draw_to(gfx::render_target& target)
{
    _map.draw_to(target);
}

auto field::can_draw() const -> bool
{
    return true;
}

void field::clear_fields(point_i const& point)
{
    auto& cell {get_cell(point)};
    if (cell.IsCleared) {
        return;
    }

    set_tile(_layerFront, point, TS_NONE);
    cell.IsCleared = true;

    if (--_numRemainingCells == _numMines) {
        _state = game_state::Success;
        reveal_mines();
        return;
    }

    if (cell.NeighborMines == 0) {
        for (auto& nb : neighbours) {
            point_i nc {point + nb};
            if (nc.X < _gridSize.Width && nc.Y < _gridSize.Height && nc.X >= 0 && nc.Y >= 0) {
                if (!get_cell(nc).IsCleared) {
                    _markedForClearing.push(nc);
                }
            }
        }
    }
}

void field::reveal_mines()
{
    for (i32 x {0}; x < _gridSize.Width; ++x) {
        for (i32 y {0}; y < _gridSize.Height; ++y) {
            auto& cell {get_cell({x, y})};
            if (cell.IsMine) {
                cell.IsCleared = true;
                set_tile(_layerFront, {x, y}, TS_NONE);
            }
        }
    }
}

void field::move_mine(point_i const& point)
{
    auto& cell {get_cell(point)};
    cell.IsMine = false;
    set_tile(_layerBack, point, cell.NeighborMines + 1);

    // subtract mine amount
    for (auto& nb : neighbours) {
        point_i nc {point + nb};
        if (nc.X >= 0 && nc.X < _gridSize.Width && nc.Y >= 0 && nc.Y < _gridSize.Height) {
            auto& nbcell {get_cell(nc)};
            --nbcell.NeighborMines;
            if (!nbcell.IsMine) {
                set_tile(_layerBack, nc, nbcell.NeighborMines + 1);
            }
        }
    }

    // search for new spot
    for (;;) {
        point_i newSpot {_rand(0, _gridSize.Width - 1), _rand(0, _gridSize.Height - 1)};
        auto&   candidate {get_cell(newSpot)};
        if (!candidate.IsMine && point != newSpot) {
            candidate.IsMine = true;
            set_tile(_layerBack, newSpot, TS_MINE);

            // add mine amount
            for (auto& nb : neighbours) {
                point_i nc {newSpot + nb};
                if (nc.X >= 0 && nc.X < _gridSize.Width && nc.Y >= 0 && nc.Y < _gridSize.Height) {
                    auto& nbcell {get_cell(nc)};
                    ++nbcell.NeighborMines;
                    if (!nbcell.IsMine) {
                        set_tile(_layerBack, nc, nbcell.NeighborMines + 1);
                    }
                }
            }
            return;
        }
    }
}

void field::set_tile(u32 layer, point_i const& point, gfx::tile_index_t id)
{
    _map.set_tile(layer, point, id);
}

auto field::get_cell(point_i const& point) -> cell&
{
    return _cells.at(point.X + point.Y * _gridSize.Width);
}

void field::toggle_front_layer_visibility()
{
    _map.set_layer_visible(_layerFront, !_map.is_layer_visible(_layerFront));
}

auto field::state() const -> game_state
{
    return _state;
}

auto field::get_tile_size() const -> size_f
{
    f32 const max {std::max<f32>(_gridSize.Width, _gridSize.Height)};
    f32 const size {_windowHeight / max};
    return size_f {size, size};
}

void field::on_key_down(input::keyboard::event const& ev)
{
    switch (ev.ScanCode) {
    case input::scan_code::C:
        toggle_front_layer_visibility();
        break;
    default:
        break;
    }
}

void field::on_mouse_button_up(input::mouse::button_event const& ev)
{
    switch (ev.Button) {
    case input::mouse::button::Left:
        process_left_click();
        break;
    case input::mouse::button::Right:
        process_right_click();
        break;
    default:
        break;
    }
}

void field::on_mouse_motion(input::mouse::motion_event const& ev)
{
    process_mouse_motion(static_cast<point_f>(ev.Position));
}

}
