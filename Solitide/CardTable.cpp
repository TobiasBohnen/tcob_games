// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "CardTable.hpp"

#include "Cards.hpp"
#include "Games.hpp"

namespace solitaire {

constexpr f32 FACE_DOWN_OFFSET {15.0f};
constexpr f32 FACE_UP_OFFSET {6.0f};

card_table::card_table(gfx::window* window, gfx::ui::canvas_widget* canvas, assets::group& resGrp)
    : _window {window}
    , _resGrp {resGrp}
    , _cardRenderer {*this}
    , _bgCanvas {*this, resGrp}
    , _fgCanvas {*this, canvas, resGrp}
{
    Bounds.Changed.connect([&](auto const&) { mark_dirty(); });
}

void card_table::start(std::shared_ptr<base_game> const& game)
{
    start_game(game, std::nullopt);
}

void card_table::resume(std::shared_ptr<base_game> const& game, data::config::object& savegame)
{
    start_game(game, savegame);
}

void card_table::start_game(std::shared_ptr<base_game> const& game, std::optional<data::config::object> const& savegame)
{
    _currentGame = game;

    _currentGame->Layout.connect([&]() { layout(); });
    reset();

    _fgCanvas.disable_hint();
    mark_dirty();

    _currentGame->start(savegame);
    _cardRenderer.start();
}

void card_table::set_cardset(std::shared_ptr<cardset> const& cardset)
{
    _cardSize = cardset->get_card_size();
    _cardRenderer.set_cardset(cardset);
    if (_currentGame) {
        layout();
    }
}

void card_table::layout()
{
    _descriptionCache.clear();

    auto const& cardSize {_cardSize};
    rect_f      pileBounds;
    for (auto const& [_, piles] : _currentGame->piles()) {
        for (auto* pile : piles) {
            point_f   pos {multiply(pile->Position, cardSize)};
            f32 const offsetMod {static_cast<f32>(pile->Cards.size() / 5)};

            switch (pile->Layout) {
            case layout_type::Squared: {
                rect_f const bounds {pos, cardSize};
                for (auto& card : pile->Cards) {
                    card.Bounds = bounds;
                }
                pile->Bounds = bounds;
            } break;
            case layout_type::Column: { // TODO: break large columns
                pile->Bounds = {pos, cardSize};
                for (auto& card : pile->Cards) {
                    card.Bounds  = {pos, cardSize};
                    pile->Bounds = pile->Bounds.as_merged(card.Bounds);
                    if (card.is_face_down()) {
                        pos.Y += cardSize.Height / (FACE_DOWN_OFFSET + offsetMod);
                    } else {
                        pos.Y += cardSize.Height / (FACE_UP_OFFSET + offsetMod);
                    }
                }
            } break;
            case layout_type::Row: {
                pile->Bounds = {pos, cardSize};
                for (auto& card : pile->Cards) {
                    card.Bounds  = {pos, cardSize};
                    pile->Bounds = pile->Bounds.as_merged(card.Bounds);
                    if (card.is_face_down()) {
                        pos.X += cardSize.Width / (FACE_DOWN_OFFSET + offsetMod);
                    } else {
                        pos.X += cardSize.Height / (FACE_UP_OFFSET + offsetMod);
                    }
                }
            } break;
            case layout_type::Fan: {
                pile->Bounds = {pos, cardSize};
                if (pile->empty()) { break; }
                for (isize i {0}; i < std::ssize(pile->Cards); ++i) {
                    auto& card {pile->Cards[i]};
                    if (i < std::ssize(pile->Cards) - 3) {
                        card.Bounds = {pos, cardSize};
                    } else {
                        card.Bounds = {pos, cardSize};
                        pos.X += cardSize.Height / FACE_UP_OFFSET;
                    }
                    pile->Bounds = pile->Bounds.as_merged(card.Bounds);
                }
            } break;
            }

            pileBounds = pileBounds == rect_f::Zero ? pile->Bounds : pileBounds.as_merged(pile->Bounds);
        }
    }

    move_camera(pileBounds.get_size());
    mark_dirty();
}

void card_table::mark_dirty()
{
    _cardRenderer.mark_dirty();
    _bgCanvas.mark_dirty();
    _fgCanvas.mark_dirty();
}

auto card_table::game() const -> base_game*
{
    return _currentGame.get();
}

void card_table::show_next_hint()
{
    if (!_currentGame) { return; }

    _fgCanvas.show_hint();
}

void card_table::set_theme(color_themes const& theme)
{
    _bgCanvas.set_background_colors(theme.TableBackgroundA, theme.TableBackgroundB);
}

void card_table::on_update(milliseconds deltaTime)
{
    if (!_currentGame) { return; }
    if (_currentGame->Status != game_status::Running) {
        reset();
    }

    _bgCanvas.update(deltaTime);
    _fgCanvas.update(deltaTime);

    _cardRenderer.update(deltaTime);

    if (_camPosTween) { _camPosTween->update(deltaTime); }
    if (_camZoomTween) { _camZoomTween->update(deltaTime); }
}

void card_table::on_draw_to(gfx::render_target& target)
{
    _bgCanvas.draw(target);
    if (_isDragging) { _cardRenderer.mark_dirty(); }
    _cardRenderer.draw(target);
    _fgCanvas.draw(target);
}

void card_table::move_camera(size_f cardBounds)
{
    using namespace tcob::tweening;

    auto&      camera {*_window->Camera};
    auto const winSize {_window->Size()};

    f32 const     hDiff {static_cast<f32>(winSize.Height - Bounds->Height)};
    f32 const     zoom {std::min(winSize.Width / cardBounds.Width, (winSize.Height - hDiff) / cardBounds.Height)};
    point_f const pos {cardBounds.Width / 2, (cardBounds.Height + (Bounds->Y / zoom)) / 2};

    if (_camInstant) {
        camera.look_at(pos);
        camera.set_zoom(size_f {zoom, zoom});
        _bgCanvas.mark_dirty();
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
            _bgCanvas.mark_dirty();
        });

        _camZoomTween = make_unique_tween<linear_tween<size_f>>(0.75s, camera.get_zoom(), size_f {zoom, zoom});
        _camZoomTween->start();
        _camZoomTween->Value.Changed.connect([&](auto val) {
            camera.set_zoom(val);
            _bgCanvas.mark_dirty();
        });
    }
}

auto card_table::can_draw() const -> bool
{
    return _currentGame != nullptr;
}

void card_table::on_key_down(input::keyboard::event& ev)
{
    if (!_currentGame) { return; }

    using namespace tcob::enum_ops;

    if (!ev.Repeat) {
        if (ev.KeyCode == input::key_code::SPACE) {
            _currentGame->deal_cards();
            ev.Handled = true;
        } else if (ev.KeyCode == input::key_code::z && (ev.KeyMods & input::key_mod::LeftControl) == input::key_mod::LeftControl) {
            _currentGame->undo();
            ev.Handled = true;
        }
    }
}

void card_table::on_mouse_motion(input::mouse::motion_event& ev)
{
    if (!_currentGame || _currentGame->Status != game_status::Running) { return; }

    if (_buttonDown) {
        if (_hovered.Pile) { drag_cards(ev); }
        if (_isDragging) { get_drop_target(); }
    } else {
        get_hovered(ev.Position);
    }

    HoverChange(get_description(_hovered.Pile));

    ev.Handled = true;
}

void card_table::on_mouse_button_down(input::mouse::button_event& ev)
{
    if (!_currentGame || _currentGame->Status != game_status::Running) { return; }

    if (ev.Button == input::mouse::button::Left) {
        _buttonDown = true;
        if (_hovered.Pile) {
            if (_hovered.Pile->Type == pile_type::Stock) {
                // deal card
                _hovered.Pile->remove_tint();
                _currentGame->deal_cards();
            } else if (ev.Clicks > 1) {
                // try move to foundation
                _currentGame->auto_play_cards(*_hovered.Pile);
            }
        }

        if (ev.Clicks > 1) { get_hovered(ev.Position); }
        HoverChange(get_description(_hovered.Pile));
    }
}

void card_table::on_mouse_button_up(input::mouse::button_event& ev)
{
    if (!_currentGame || _currentGame->Status != game_status::Running) { return; }

    if (ev.Button == input::mouse::button::Left) {
        _buttonDown = false;
        if (_isDragging) {
            if (_hovered.Pile) {
                _hovered.Pile->IsDragging = false;
            }

            if (_dropTarget.Pile && _hovered.Pile) {
                _currentGame->play_cards(*_hovered.Pile, *_dropTarget.Pile, _hovered.Index, std::ssize(_hovered.Pile->Cards) - _hovered.Index);
            } else {
                layout();
            }
            _isDragging = false;
            _fgCanvas.disable_hint();
        }

        get_hovered(ev.Position);
        HoverChange(get_description(_hovered.Pile));
    }
}

void card_table::reset()
{
    _dropTarget   = {};
    _hovered      = {};
    _camInstant   = true;
    _camPosTween  = nullptr;
    _camZoomTween = nullptr;
    _isDragging   = false;
    _buttonDown   = false;
}

auto card_table::get_description(pile const* pile) -> pile_description
{
    if (!pile || !_currentGame) { return {}; }

    if (auto it {_descriptionCache.find(pile)}; it != _descriptionCache.end()) {
        return it->second;
    }

    auto const retValue {pile->get_description(*_currentGame)};
    _descriptionCache[pile] = retValue;
    return retValue;
}

void card_table::drag_cards(input::mouse::motion_event const& ev)
{
    _isDragging = false;

    auto& cards {_hovered.Pile->Cards};
    if (!_hovered.Pile->is_playable() || cards.empty() || _hovered.Index < 0) {
        mark_dirty();
        return;
    }

    auto const    zoom {(*_window->Camera).get_zoom()};
    point_f const off {ev.RelativeMotion.X / zoom.Width, ev.RelativeMotion.Y / zoom.Height};
    for (isize i {_hovered.Index}; i < std::ssize(cards); ++i) {
        cards[i].Bounds.move_by(off);
    }

    _dragRect   = cards[_hovered.Index].Bounds;
    _isDragging = true;

    _hovered.Pile->IsDragging = true;
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
            target.Pile && target.Index == std::ssize(target.Pile->Cards) - 1
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
    _hovered = get_pile_at(point_i {(*_window->Camera).convert_screen_to_world(pos)}, false);

    if (oldPile.Pile) {
        oldPile.Pile->IsHovering = false;
        oldPile.Pile->remove_tint();
        mark_dirty();
    }

    if (_hovered.Pile) {
        _hovered.Pile->IsHovering = true;
        _hovered.Pile->tint_cards(get_hover_color(_hovered.Pile, _hovered.Index), _hovered.Index);
        mark_dirty();
    }
}

auto card_table::get_pile_at(point_i pos, bool ignoreHoveredPile) const -> hit_test_result
{
    auto const checkPile {[&](pile const& p) -> isize {
        auto const top {std::ssize(p.Cards) - 1};

        if (ignoreHoveredPile && p.IsHovering) { return INDEX_INVALID; }
        if (p.empty() && p.Marker && p.Marker->Bounds->contains(pos)) { return INDEX_MARKER; }
        if (!p.is_playable() && !p.empty() && p.Cards[top].Bounds.contains(pos)) { return top; }

        for (isize i {top}; i >= 0; --i) {
            if (p.Cards[i].Bounds.contains(pos) && _currentGame->check_movable(p, i)) {
                return i;
            }
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
