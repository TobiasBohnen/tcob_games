// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../_common/Common.hpp" // IWYU pragma: keep

#include <stack>
#include <vector>

namespace Mines {

enum class flag_type {
    None,
    Maybe,
    Flag
};

struct cell {
    bool      IsCleared {false};
    bool      IsMine {false};
    flag_type Flag {flag_type::None};
    u8        NeighborMines {0};
};

class field : public gfx::entity {
public:
    explicit field(assets::asset_ptr<gfx::material> const& material);

    void start(size_i gridSize, i32 windowHeight, u32 mines);

    void toggle_front_layer_visibility();

    auto state() const -> game_state;

protected:
    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_draw_to(gfx::render_target& target) override;
    auto can_draw() const -> bool override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;
    void on_mouse_motion(input::mouse::motion_event const& ev) override;

private:
    void process_left_click();
    void process_right_click();
    void process_mouse_motion(point_f mouse);

    auto get_tile_size() const -> size_f;
    void clear_fields(point_i const& point);
    void reveal_mines();
    void move_mine(point_i const& point);
    void set_tile(u32 layer, point_i const& point, gfx::tile_index_t id);
    auto get_cell(point_i const& point) -> cell&;

    rng        _rand {};
    game_state _state {game_state::Initial};

    std::optional<point_i> _mouseOverPoint {};

    gfx::orthogonal_tilemap _map;
    std::vector<cell>       _cells {};
    uid                     _layerBack {0};
    uid                     _layerFront {1};

    size_i              _gridSize {size_i::Zero};
    i32                 _windowHeight {0};
    u32                 _numMines {0};
    u32                 _numRemainingCells {0};
    bool                _firstClick {false};
    std::stack<point_i> _markedForClearing {};
};

}
