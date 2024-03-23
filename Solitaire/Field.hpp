// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Cardset.hpp"
#include "Common.hpp" // IWYU pragma: keep

namespace solitaire {

////////////////////////////////////////////////////////////

class field : public gfx::entity {
public:
    field(gfx::window* parent, size_i size, assets::group& resGrp);

    signal<hover_info const> HoverChange;

    void start(std::shared_ptr<games::base_game> const& game, bool cont);
    void undo();
    void quit();

    auto state() const -> game_state;

    auto get_size() const -> size_i;
    auto get_material() const -> assets::asset_ptr<gfx::material> const&;

    void mark_dirty();

protected:
    void move_camera(size_f bounds);
    void on_update(milliseconds deltaTime) override;

    void on_draw_to(gfx::render_target& target) override;
    auto can_draw() const -> bool override;

    void on_key_down(input::keyboard::event& ev) override;
    void on_mouse_motion(input::mouse::motion_event& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event& ev) override;
    void on_mouse_button_down(input::mouse::button_event& ev) override;
    void on_mouse_button_up(input::mouse::button_event& ev) override;

private:
    void draw_cards(gfx::render_target& target);
    void create_markers(size_f const& cardSize);
    void get_pile_quads(std::vector<gfx::quad>::iterator& quadIt, pile const* pile) const;

    void drag_cards(input::mouse::motion_event const& ev);
    auto get_hover_color(pile* pile, isize idx) const -> color;
    void get_hovered(point_i pos);
    auto get_drop_color() const -> color;
    void get_drop_target();

    gfx::window*   _parentWindow;
    assets::group& _resGrp;
    size_i         _size;

    std::shared_ptr<games::base_game> _currentGame;
    data::config::object              _saveGame;

    // render
    gfx::sprite_batch                _markerSprites;
    gfx::quad_renderer               _cardRenderer;
    std::vector<gfx::quad>           _cardQuads;
    bool                             _cardQuadsDirty {true};
    gfx::text                        _text;
    assets::asset_ptr<gfx::material> _material;

    // cardsets
    i32                                   _currentCardSet {1};
    i32                                   _maxCardSet {1};
    std::vector<std::shared_ptr<cardset>> _cardSets;

    // hover/drag
    hit_test_result _hovered {};
    hit_test_result _dropTarget {};
    rect_f          _dragRect {rect_f::Zero};
    bool            _buttonDown {false};
    bool            _isDragging {false};

    // camera
    bool                                             _camManual {false};
    bool                                             _camInstant {true};
    std::unique_ptr<tweening::linear_tween<point_f>> _camPosTween;
    std::unique_ptr<tweening::linear_tween<size_f>>  _camZoomTween;
};

}
