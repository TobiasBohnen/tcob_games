// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Canvases.hpp"
#include "CardRenderer.hpp"
#include "CardSet.hpp"
#include "Piles.hpp"
#include "ui/Themes.hpp"

namespace solitaire {

////////////////////////////////////////////////////////////

struct hit_test_result {
    pile* Pile {nullptr};
    isize Index {INDEX_INVALID};
};

////////////////////////////////////////////////////////////

class card_table : public gfx::entity {
public:
    card_table(gfx::camera& camera, assets::group& resGrp, settings* settings);

    signal<pile* const> HoverChange;
    prop<rect_f>        Bounds;

    void start(std::shared_ptr<base_game> const& game, std::optional<u64> seed);
    void resume(std::shared_ptr<base_game> const& game, data::config::object& savegame);

    auto game() const -> base_game*;

    void show_next_hint();

    void set_theme(color_themes const& theme);
    void set_card_set(std::shared_ptr<card_set> const& cardset);

protected:
    void on_update(milliseconds deltaTime) override;

    void on_draw_to(gfx::render_target& target) override;
    auto can_draw() const -> bool override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_mouse_motion(input::mouse::motion_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;

private:
    void layout();
    void mark_dirty();

    void reset();

    auto get_description(pile const* pile) -> pile_description;

    void move_camera(rect_f const& cardBounds);

    void start_game(std::shared_ptr<base_game> const& game, std::optional<data::config::object> const& savegame, std::optional<u64> seed);

    void drag_cards(input::mouse::motion_event const& ev);
    auto get_hover_color(pile* pile, isize idx) const -> color;
    auto get_drop_color() const -> color;

    void get_drop_target();
    auto get_drop_target_at(rect_f const& rect, card const& card, isize numCards) const -> hit_test_result;
    void get_hovered(point_i pos);

    auto get_pile_at(point_i pos, bool ignoreHoveredPile) const -> hit_test_result;

    gfx::camera&   _camera;
    assets::group& _resGrp;
    settings*      _settings;

    std::shared_ptr<base_game> _currentGame;
    size_f                     _cardSize;

    // render
    card_renderer     _cardRenderer;
    background_canvas _bgCanvas;
    foreground_canvas _fgCanvas;

    // hover/drag
    hit_test_result _hovered {};
    hit_test_result _dropTarget {};
    rect_f          _dragRect {rect_f::Zero};
    bool            _buttonDown {false};

    // camera
    bool                                             _camInstant {true};
    std::unique_ptr<tweening::linear_tween<point_f>> _camPosTween;
    std::unique_ptr<tweening::linear_tween<size_f>>  _camZoomTween;
};

}
