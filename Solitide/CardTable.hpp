// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Cardset.hpp"

namespace solitaire {

////////////////////////////////////////////////////////////

class card_table : public gfx::entity {
public:
    card_table(gfx::window* parent, gfx::ui::canvas_widget* canvas, assets::group& resGrp);

    signal<pile_description const> HoverChange;
    prop<rect_f>                   Bounds;

    void start(std::shared_ptr<games::base_game> const& game);
    void resume(std::shared_ptr<games::base_game> const& game, data::config::object& savegame);

    auto game() const -> std::shared_ptr<games::base_game>;

    void show_next_hint();

    void set_cardset(std::shared_ptr<cardset> cardset);

    void on_pile_layout();
    void on_end_turn();

protected:
    void move_camera(size_f cardBounds);
    void on_update(milliseconds deltaTime) override;

    void on_draw_to(gfx::render_target& target) override;
    auto can_draw() const -> bool override;

    void on_key_down(input::keyboard::event& ev) override;
    void on_mouse_motion(input::mouse::motion_event& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event& ev) override;
    void on_mouse_button_down(input::mouse::button_event& ev) override;
    void on_mouse_button_up(input::mouse::button_event& ev) override;

private:
    void mark_dirty();

    void start_game(std::shared_ptr<games::base_game> const& game, std::optional<data::config::object> const& savegame);

    void draw_cards(gfx::render_target& target);

    void draw_canvas();
    void draw_hint();

    void create_markers(size_f const& cardSize);
    void get_pile_quads(std::vector<gfx::quad>::iterator& quadIt, pile const* pile) const;

    void drag_cards(input::mouse::motion_event const& ev);
    auto get_hover_color(pile* pile, isize idx) const -> color;
    auto get_drop_color() const -> color;

    void get_drop_target();
    auto get_drop_target_at(rect_f const& rect, card const& card, isize numCards) const -> hit_test_result;
    void get_hovered(point_i pos);

    auto get_pile_at(point_i pos, bool ignoreActivePile) const -> hit_test_result;

    gfx::window*   _parentWindow;
    assets::group& _resGrp;

    std::shared_ptr<games::base_game> _currentGame;

    std::shared_ptr<cardset> _cardset;

    // render
    gfx::sprite_batch      _markerSprites;
    gfx::quad_renderer     _cardRenderer;
    std::vector<gfx::quad> _cardQuads;
    bool                   _renderDirty {true};
    gfx::text              _text;

    // canvas
    gfx::ui::canvas_widget* _canvas;
    isize                   _currentHint {-1};
    bool                    _showHint {false};
    timer                   _hintTimer;
    bool                    _canvasDirty {true};

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
