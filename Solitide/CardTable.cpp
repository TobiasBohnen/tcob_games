// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "CardTable.hpp"

#include <utility>

#include "Cards.hpp"
#include "Games.hpp"

namespace solitaire {

card_table::card_table(gfx::window* parent, gfx::ui::canvas_widget* canvas, assets::group& resGrp)
    : _parentWindow {parent}
    , _resGrp {resGrp}
    , _cardRenderer {gfx::buffer_usage_hint::DynamicDraw}
    , _text {resGrp.get<gfx::font_family>("Poppins")->get_font({.Weight = gfx::font::weight::Bold}, 96)}
    , _canvas {canvas}
{
    auto& effects {_text.get_effects()};
    effects.add(1, gfx::make_unique_quad_tween<gfx::wave_effect>(3s, {30, 4.f}));
    effects.start_all(playback_style::Looped);
    _text.hide();

    _hintTimer.Tick.connect([&](auto&&) {
        _canvasDirty = true;
        _showHint    = false;
        _hintTimer.stop();
    });

    Bounds.Changed.connect([&](auto const& val) {
        _text.Bounds = val;
        mark_dirty();
    });
}

void card_table::start(std::shared_ptr<games::base_game> const& game)
{
    start_game(game, std::nullopt);
}

void card_table::resume(std::shared_ptr<games::base_game> const& game, data::config::object& savegame)
{
    start_game(game, savegame);
}

void card_table::start_game(std::shared_ptr<games::base_game> const& game, std::optional<data::config::object> const& savegame)
{
    _currentGame = game;

    _text.hide();

    _dropTarget   = {};
    _hovered      = {};
    _camManual    = false;
    _camInstant   = true;
    _camPosTween  = nullptr;
    _camZoomTween = nullptr;
    _hintTimer.stop();
    mark_dirty();

    _cardQuads.clear();
    _cardQuads.resize(_currentGame->info().DeckCount * 52);

    auto const& cardSize {_cardset->get_card_size()};
    _currentGame->start(cardSize, savegame);
    create_markers(cardSize);
}

void card_table::set_cardset(std::shared_ptr<cardset> cardset)
{
    _cardset = std::move(cardset);
    mark_dirty();
}

void card_table::on_pile_layout()
{
    mark_dirty();
}

void card_table::on_end_turn()
{
    mark_dirty();
    _hintTimer.stop();
}

void card_table::create_markers(size_f const& cardSize)
{
    _markerSprites.clear();
    for (auto const& [_, piles] : _currentGame->piles()) {
        for (auto* pile : piles) {
            if (!pile->HasMarker) { continue; }

            pile->Marker                = _markerSprites.create_sprite();
            pile->Marker->Material      = _cardset->get_material();
            pile->Marker->TextureRegion = pile->get_marker_texture_name();
            pile->Marker->Bounds        = {multiply(pile->Position, cardSize), cardSize};
        }
    }
}

void card_table::mark_dirty()
{
    _renderDirty = true;
    _canvasDirty = true;
}

auto card_table::game() const -> std::shared_ptr<games::base_game>
{
    return _currentGame;
}

void card_table::show_next_hint()
{
    if (!_currentGame) { return; }

    auto const& hints {_currentGame->get_available_hints()};
    if (hints.empty()) { return; }

    _currentHint++;
    if (_currentHint >= std::ssize(hints)) { // TODO:reset _currentHint on game change
        _currentHint = 0;
    }

    _hintTimer.start(5s, timer::mode::BusyLoop);
    _canvasDirty = true;
    _showHint    = true;
}

void card_table::on_update(milliseconds deltaTime)
{
    if (!_currentGame) { return; }

    _markerSprites.update(deltaTime);

    auto const gameState {_currentGame->State()};
    if (gameState == game_state::Success) {
        _text.Style = {.Color = colors::Green, .Alignment = {.Horizontal = gfx::horizontal_alignment::Centered, .Vertical = gfx::vertical_alignment::Middle}};
        _text.Text  = "{EFFECT:1}GAME WON";
        _text.show();
    } else if (gameState == game_state::Failure) {
        _text.Style = {.Color = colors::Red, .Alignment = {.Horizontal = gfx::horizontal_alignment::Centered, .Vertical = gfx::vertical_alignment::Middle}};
        _text.Text  = "{EFFECT:1}YOU LOSE";
        _text.show();
    } else {
        _text.hide();
    }

    _text.update(deltaTime);
    if (_camPosTween) { _camPosTween->update(deltaTime); }
    if (_camZoomTween) { _camZoomTween->update(deltaTime); }
}

void card_table::on_draw_to(gfx::render_target& target)
{
    _markerSprites.draw_to(target);

    draw_cards(target);
    draw_canvas();

    _text.draw_to(target);
}

void card_table::draw_cards(gfx::render_target& target)
{
    if (_renderDirty || _isDragging) {
        _cardRenderer.set_material(_cardset->get_material());

        size_f bounds {size_f::Zero};

        pile const* dragPile {nullptr};
        {
            auto quadIt {_cardQuads.begin()};
            for (auto const& [_, piles] : _currentGame->piles()) {
                for (auto const* pile : piles) {
                    if (pile->Marker) {
                        bounds.Width  = std::max(bounds.Width, pile->Marker->Bounds->right());
                        bounds.Height = std::max(bounds.Height, pile->Marker->Bounds->bottom());
                    }

                    if (_isDragging && pile->is_hovering()) {
                        dragPile = pile;
                    } else {
                        for (auto const& card : pile->Cards) {
                            bounds.Width  = std::max(bounds.Width, card.Bounds.right());
                            bounds.Height = std::max(bounds.Height, card.Bounds.bottom());
                        }

                        get_pile_quads(quadIt, pile);
                    }
                }
            }

            _cardRenderer.set_geometry({_cardQuads.begin(), quadIt});
            _cardRenderer.render_to_target(target);
        }
        if (dragPile) {
            auto quadIt {_cardQuads.begin()};
            get_pile_quads(quadIt, dragPile);

            _cardRenderer.set_geometry({_cardQuads.begin(), quadIt});
            _cardRenderer.render_to_target(target);
        }

        move_camera(bounds);

        _renderDirty = false;
    } else {
        _cardRenderer.set_geometry(_cardQuads);
        _cardRenderer.render_to_target(target);
    }
}

void card_table::draw_canvas()
{
    if (_canvasDirty) {
        _canvas->clear();

        _canvas->save();
        _canvas->translate(-Bounds->get_position());

        if (_showHint) { draw_hint(); }

        _canvas->restore();
        _canvasDirty = false;
    }
}

void card_table::draw_hint()
{
    auto const& hints {_currentGame->get_available_hints()};
    if (hints.empty()) { return; }
    auto const& hint {hints[_currentHint]};

    rect_f srcBounds {hint.Src->Cards[hint.SrcCardIdx].Bounds};
    for (isize i {hint.SrcCardIdx + 1}; i < std::ssize(hint.Src->Cards); ++i) {
        srcBounds = srcBounds.as_merged(hint.Src->Cards[i].Bounds);
    }

    rect_f dstBounds;
    if (hint.DstCardIdx >= 0) {
        dstBounds = hint.Dst->Cards[hint.DstCardIdx].Bounds;
    } else if (hint.Dst->Marker) {
        dstBounds = hint.Dst->Marker->Bounds();
    }

    auto& camera {*_parentWindow->Camera};

    // Draw bounds
    _canvas->begin_path();
    _canvas->rounded_rect(rect_f {camera.convert_world_to_screen(dstBounds)}, 10);
    _canvas->set_stroke_style(colors::Red);
    _canvas->set_stroke_width(10);
    _canvas->stroke();

    auto const screenSrcBounds {rect_f {camera.convert_world_to_screen(srcBounds)}};
    _canvas->set_global_composite_blendfunc(gfx::blend_func::One, gfx::blend_func::Zero);
    _canvas->begin_path();
    _canvas->rounded_rect(screenSrcBounds, 10);
    _canvas->set_fill_style(colors::Transparent);
    _canvas->fill();
    _canvas->set_global_composite_blendfunc(gfx::blend_func::SrcAlpha, gfx::blend_func::OneMinusSrcAlpha);

    _canvas->begin_path();
    _canvas->rounded_rect(screenSrcBounds, 10);
    _canvas->set_stroke_style(colors::Green);
    _canvas->set_stroke_width(10);
    _canvas->stroke();

    // Draw arrow
    auto from {point_f {camera.convert_world_to_screen(srcBounds.get_center())}};
    auto to {point_f {camera.convert_world_to_screen(dstBounds.get_center())}};

    f32 const borderWidth {3};
    f32 const arrowWidth {6};
    f32 const headLength {arrowWidth * 6};

    _canvas->set_line_cap(gfx::line_cap::Round);
    _canvas->begin_path();
    _canvas->move_to(from);
    _canvas->line_to(to);

    f32 const angle {std::atan2(to.Y - from.Y, to.X - from.X)};

    _canvas->move_to(to);
    _canvas->line_to({to.X - headLength * std::cos(angle - TAU_F / 12), to.Y - headLength * std::sin(angle - TAU_F / 12)});
    _canvas->move_to(to);
    _canvas->line_to({to.X - headLength * std::cos(angle + TAU_F / 12), to.Y - headLength * std::sin(angle + TAU_F / 12)});
    _canvas->set_stroke_style(colors::Black);
    _canvas->set_stroke_width(arrowWidth + borderWidth);
    _canvas->stroke();
    _canvas->set_stroke_style(colors::Gray);
    _canvas->set_stroke_width(arrowWidth);
    _canvas->stroke();
}

void card_table::move_camera(size_f cardBounds)
{
    if (!_camManual) {
        using namespace tcob::tweening;

        auto&      camera {*_parentWindow->Camera};
        auto const winSize {_parentWindow->Size()};

        f32 const     hDiff {static_cast<f32>(winSize.Height - Bounds->Height)};
        f32 const     zoom {std::min(winSize.Width / cardBounds.Width, (winSize.Height - hDiff) / cardBounds.Height)};
        //  point_f const pos {cardBounds.Width / 2, (cardBounds.Height + (off / zoom)) / 2}; // FIXME: _bounds X and Y are ignored
        point_f const pos {cardBounds.Width / 2, cardBounds.Height / 2};

        if (_camInstant) {
            camera.look_at(pos);
            camera.set_zoom(size_f {zoom, zoom});
            _camInstant = false;
        } else {
            _camPosTween = make_unique_tween<linear_tween<point_f>>(0.75s, camera.get_look_at(), pos);
            _camPosTween->start();
            _camPosTween->Value.Changed.connect([&](auto val) {
                if (_hovered.Pile && _isDragging) {
                    auto& cards {_hovered.Pile->Cards};
                    for (isize i {_hovered.Index}; i < std::ssize(cards); ++i) {
                        cards[i].Bounds.move_by(val - camera.get_look_at());
                    }

                    _dragRect = cards[_hovered.Index].Bounds;
                }
                camera.look_at(val);
            });

            _camZoomTween = make_unique_tween<linear_tween<size_f>>(0.75s, camera.get_zoom(), size_f {zoom, zoom});
            _camZoomTween->start();
            _camZoomTween->Value.Changed.connect([&](auto val) { camera.set_zoom(val); });
        }
    }
}

void card_table::drag_camera(point_i rel)
{
    auto&         camera {*_parentWindow->Camera};
    size_f const  zoom {camera.get_zoom()};
    point_f const off {-rel.X / zoom.Width, -rel.Y / zoom.Height};
    camera.move_by(off);
    _camManual   = true;
    _canvasDirty = true;
}

void card_table::get_pile_quads(std::vector<gfx::quad>::iterator& quadIt, pile const* pile) const
{
    auto const mat {_cardset->get_material()};
    for (auto const& card : pile->Cards) {
        auto& quad {*quadIt};
        gfx::geometry::set_color(quad, card.Color);
        gfx::geometry::set_texcoords(quad, mat->Texture->get_region(card.get_texture_name()));
        gfx::geometry::set_position(quad, card.Bounds);
        ++quadIt;
    }
}

auto card_table::can_draw() const -> bool
{
    return _currentGame != nullptr;
}

void card_table::on_key_down(input::keyboard::event& ev)
{
    if (!_currentGame) { return; }

    _currentGame->key_down(ev);
}

void card_table::on_mouse_motion(input::mouse::motion_event& ev)
{
    if (!_currentGame) { return; }

    if (input::system::IsMouseButtonDown(input::mouse::button::Right)) {
        drag_camera(ev.RelativeMotion);
    } else {
        if (_buttonDown) {
            if (_hovered.Pile) { drag_cards(ev); }
            if (_isDragging) { get_drop_target(); }
        } else {
            get_hovered(ev.Position);
        }

        HoverChange(_currentGame->get_description(_hovered.Pile));
    }

    ev.Handled = true;
}

void card_table::on_mouse_wheel(input::mouse::wheel_event& ev)
{
    auto& camera {*_parentWindow->Camera};
    if (ev.Scroll.Y > 0) {
        camera.zoom_by({1.1f, 1.1f});
    } else {
        camera.zoom_by({1 / 1.1f, 1 / 1.1f});
    }

    _camManual   = true;
    _canvasDirty = true;
    ev.Handled   = true;
}

void card_table::on_mouse_button_down(input::mouse::button_event& ev)
{
    if (!_currentGame) { return; }

    if (ev.Button == input::mouse::button::Left) {
        _buttonDown = true;
        _currentGame->click(_hovered.Pile, ev.Clicks);

        if (ev.Clicks > 1) { get_hovered(ev.Position); }
        HoverChange(_currentGame->get_description(_hovered.Pile));
    }
}

void card_table::on_mouse_button_up(input::mouse::button_event& ev)
{
    if (!_currentGame) { return; }

    if (ev.Button == input::mouse::button::Left) {
        _buttonDown = false;
        if (_isDragging) {
            _currentGame->drop_cards(_hovered, _dropTarget);
            _isDragging = false;
        }

        get_hovered(ev.Position);
        HoverChange(_currentGame->get_description(_hovered.Pile));
    }
}

void card_table::drag_cards(input::mouse::motion_event const& ev)
{
    _isDragging = false;

    auto& cards {_hovered.Pile->Cards};
    if (!_hovered.Pile->is_playable() || cards.empty() || _hovered.Index < 0) {
        mark_dirty();
        return;
    }

    auto const    zoom {(*_parentWindow->Camera).get_zoom()};
    point_f const off {ev.RelativeMotion.X / zoom.Width, ev.RelativeMotion.Y / zoom.Height};
    for (isize i {_hovered.Index}; i < std::ssize(cards); ++i) {
        cards[i].Bounds.move_by(off);
    }

    _dragRect   = cards[_hovered.Index].Bounds;
    _isDragging = true;

    _hintTimer.stop();
    mark_dirty();
}

void card_table::get_drop_target()
{
    auto        oldPile {_dropTarget};
    auto const& card {_hovered.Pile->Cards[_hovered.Index]};
    isize const numCards {std::ssize(_hovered.Pile->Cards) - _hovered.Index};
    _dropTarget = get_drop_target_at(_dragRect, card, numCards);

    if (_dropTarget.Pile != oldPile.Pile || _dropTarget.Index != oldPile.Index) {
        if (oldPile.Pile) {
            oldPile.Pile->remove_tint();
            mark_dirty();
        }
        if (_dropTarget.Pile) {
            _dropTarget.Pile->tint_cards(get_drop_color(), _dropTarget.Index);
            mark_dirty();
        }
    }
}

auto card_table::get_drop_target_at(rect_f const& rect, card const& card, isize numCards) const -> hit_test_result
{
    std::array<point_i, 4> points {
        point_i {rect.top_left()}, point_i {rect.top_right()},
        point_i {rect.bottom_left()}, point_i {rect.bottom_right()}};

    std::vector<hit_test_result> candidates;
    for (auto const& point : points) {
        if (auto target {get_pile_at(point, true)};
            target.Pile
            && target.Index == std::ssize(target.Pile->Cards) - 1
            && _currentGame->can_play(*target.Pile, target.Index, card, numCards)) {
            candidates.push_back(target);
        }
    }

    if (candidates.empty()) { return {}; }
    if (candidates.size() == 1) { return candidates[0]; }

    f32             maxArea {0};
    hit_test_result retValue;
    for (auto const& candidate : candidates) {
        auto const interSect {rect.as_intersected(candidate.Index == INDEX_MARKER
                                                      ? candidate.Pile->Marker->Bounds
                                                      : candidate.Pile->Cards[candidate.Index].Bounds)};
        if (interSect.Width * interSect.Height > maxArea) {
            maxArea  = interSect.Width * interSect.Height;
            retValue = candidate;
        }
    }

    return retValue;
}

void card_table::get_hovered(point_i pos)
{
    auto oldPile {_hovered};
    _hovered = get_pile_at(point_i {(*_parentWindow->Camera).convert_screen_to_world(pos)}, false);

    if (oldPile.Pile) {
        oldPile.Pile->set_hovering(false, oldPile.Index, colors::White);
        mark_dirty();
    }

    if (_hovered.Pile) {
        _hovered.Pile->set_hovering(true, _hovered.Index, get_hover_color(_hovered.Pile, _hovered.Index));
        mark_dirty();
    }
}

auto card_table::get_pile_at(point_i pos, bool ignoreHoveredPile) const -> hit_test_result
{
    auto const checkPile {[&](pile const& p) -> isize {
        auto const top {std::ssize(p.Cards) - 1};

        if (ignoreHoveredPile && p.is_hovering()) { return INDEX_INVALID; }
        if (p.empty() && p.Marker && p.Marker->Bounds->contains(pos)) { return INDEX_MARKER; }
        if (!p.is_playable() && !p.empty() && p.Cards[top].Bounds.contains(pos)) { return top; }

        for (isize i {top}; i >= 0; --i) {
            if (p.Cards[i].Bounds.contains(pos) && _currentGame->check_movable(p, i)) { return i; }
        }

        return INDEX_INVALID;
    }};

    for (auto const& [type, piles] : _currentGame->piles()) {
        for (auto* pile : piles | std::views::reverse) {
            isize const idx {checkPile(*pile)};
            if (idx != INDEX_INVALID) { return {pile, idx}; }
        }
    }

    return {};
}

auto card_table::get_hover_color(pile* pile, isize idx) const -> color
{
    auto const& moves {_currentGame->get_available_hints()};
    for (auto const& move : moves) {
        if (move.Src == pile && move.SrcCardIdx == idx) {
            return colors::LightGreen; // TODO: add option to disable
        }
    }

    return colors::LightBlue;
}

auto card_table::get_drop_color() const -> color
{
    return colors::LightGreen; // TODO: add option to disable
}

}
