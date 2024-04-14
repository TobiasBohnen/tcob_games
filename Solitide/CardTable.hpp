// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Cardset.hpp"
#include "Piles.hpp"

namespace solitaire {

////////////////////////////////////////////////////////////

class card_table;

class card_canvas {
public:
    card_canvas(card_table& parent, gfx::window* window, gfx::ui::canvas_widget* canvas, assets::group& resGrp);

    void show_next_hint();

    void draw();
    void update(milliseconds deltaTime);

    void disable_hint();
    void mark_dirty();

private:
    void draw_hint();
    void draw_state();

    card_table&             _parent;
    gfx::window*            _window;
    gfx::ui::canvas_widget* _canvas;
    assets::group&          _resGrp;

    isize      _currentHint {-1};
    bool       _showHint {false};
    timer      _hintTimer;
    bool       _canvasDirty {true};
    game_state _lastState {game_state::Initial};
};

////////////////////////////////////////////////////////////

class card_table : public gfx::entity {
public:
    card_table(gfx::window* window, gfx::ui::canvas_widget* canvas, assets::group& resGrp);

    signal<pile_description const> HoverChange;
    prop<rect_f>                   Bounds;

    void start(std::shared_ptr<games::base_game> const& game);
    void resume(std::shared_ptr<games::base_game> const& game, data::config::object& savegame);

    auto game() const -> std::shared_ptr<games::base_game>;

    void show_next_hint();

    void set_cardset(std::shared_ptr<cardset> cardset);

    void mark_dirty();

protected:
    void on_update(milliseconds deltaTime) override;

    void on_draw_to(gfx::render_target& target) override;
    auto can_draw() const -> bool override;

    void on_key_down(input::keyboard::event& ev) override;
    void on_mouse_motion(input::mouse::motion_event& ev) override;
    void on_mouse_button_down(input::mouse::button_event& ev) override;
    void on_mouse_button_up(input::mouse::button_event& ev) override;

private:
    void move_camera(size_f cardBounds);

    void start_game(std::shared_ptr<games::base_game> const& game, std::optional<data::config::object> const& savegame);

    void draw_cards(gfx::render_target& target);

    void create_markers(size_f const& cardSize);
    void get_pile_quads(std::vector<gfx::quad>::iterator& quadIt, pile const* pile) const;

    void drag_cards(input::mouse::motion_event const& ev);
    auto get_hover_color(pile* pile, isize idx) const -> color;
    auto get_drop_color() const -> color;

    void get_drop_target();
    auto get_drop_target_at(rect_f const& rect, card const& card, isize numCards) const -> hit_test_result;
    void get_hovered(point_i pos);

    auto get_pile_at(point_i pos, bool ignoreHoveredPile) const -> hit_test_result;

    gfx::window*   _window;
    assets::group& _resGrp;

    std::shared_ptr<games::base_game> _currentGame;

    std::shared_ptr<cardset> _cardset;

    // render
    gfx::sprite_batch      _markerSprites;
    gfx::quad_renderer     _cardRenderer;
    std::vector<gfx::quad> _cardQuads;
    bool                   _renderDirty {true};

    // canvas
    card_canvas _cardCanvas;

    // hover/drag
    hit_test_result _hovered {};
    hit_test_result _dropTarget {};
    rect_f          _dragRect {rect_f::Zero};
    bool            _buttonDown {false};
    bool            _isDragging {false};

    // camera
    bool                                             _camInstant {true};
    std::unique_ptr<tweening::linear_tween<point_f>> _camPosTween;
    std::unique_ptr<tweening::linear_tween<size_f>>  _camZoomTween;
};

}
