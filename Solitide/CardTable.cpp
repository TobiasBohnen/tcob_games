// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "CardTable.hpp"

#include "Cards.hpp"
#include "Games.hpp"

namespace solitaire {

constexpr f32 FACE_DOWN_OFFSET {0.10f};
constexpr f32 FACE_UP_OFFSET {0.20f};

card_table::card_table(gfx::camera& camera, assets::group& resGrp, settings* settings)
    : _camera {camera}
    , _resGrp {resGrp}
    , _settings {settings}
    , _cardRenderer {*this}
    , _bgCanvas {*this, resGrp}
    , _fgCanvas {*this, resGrp}
{
    Bounds.Changed.connect([&](auto const&) { mark_dirty(); });
}

void card_table::start(std::shared_ptr<base_game> const& game, std::optional<u64> seed)
{
    start_game(game, std::nullopt, seed);
}

void card_table::resume(std::shared_ptr<base_game> const& game, data::config::object& savegame)
{
    start_game(game, savegame, std::nullopt);
}

void card_table::start_game(std::shared_ptr<base_game> const& game, std::optional<data::config::object> const& savegame, std::optional<u64> seed)
{
    _currentGame = game;

    _currentGame->Layout.connect([&]() { layout(); });
    reset();

    _fgCanvas.disable_hint();
    mark_dirty();

    _currentGame->start(savegame, seed);
    _cardRenderer.start();
}

void card_table::set_card_set(std::shared_ptr<card_set> const& cardset)
{
    _cardSize = cardset->get_card_size();
    _cardRenderer.set_card_set(cardset);
    if (_currentGame) { layout(); }
}

void card_table::layout()
{
    auto const& cardSize {_cardSize};
    rect_f      tableBounds;
    rect_f      pileBounds;
    for (auto const& [_, piles] : _currentGame->piles()) {
        for (auto* pile : piles) {
            if (pile->Cards.empty() && !pile->HasMarker) { continue; }

            point_f pos {multiply(pile->Position, cardSize)};

            switch (pile->Layout) {
            case layout_type::Squared: {
                rect_f const bounds {pos, cardSize};
                for (auto& card : pile->Cards) {
                    card.Bounds = bounds;
                }
                pileBounds = bounds;
            } break;
            case layout_type::Column: { // TODO: break large columns
                pileBounds = {pos, cardSize};
                for (auto& card : pile->Cards) {
                    card.Bounds = {pos, cardSize};
                    pileBounds  = pileBounds.as_union_with(card.Bounds);
                    if (card.is_face_down()) {
                        pos.Y += cardSize.Height * FACE_DOWN_OFFSET;
                    } else {
                        pos.Y += cardSize.Height * FACE_UP_OFFSET;
                    }
                }
            } break;
            case layout_type::Row: {
                pileBounds = {pos, cardSize};
                for (auto& card : pile->Cards) {
                    card.Bounds = {pos, cardSize};
                    pileBounds  = pileBounds.as_union_with(card.Bounds);
                    if (card.is_face_down()) {
                        pos.X += cardSize.Width * FACE_DOWN_OFFSET;
                    } else {
                        pos.X += cardSize.Width * FACE_UP_OFFSET;
                    }
                }
            } break;
            case layout_type::Fan: {
                pileBounds = {pos, cardSize};
                if (pile->empty()) { break; }
                for (isize i {0}; i < std::ssize(pile->Cards); ++i) {
                    auto& card {pile->Cards[i]};
                    if (i < std::ssize(pile->Cards) - 3) {
                        card.Bounds = {pos, cardSize};
                    } else {
                        card.Bounds = {pos, cardSize};
                        pos.X += cardSize.Width * FACE_UP_OFFSET;
                    }
                    pileBounds = pileBounds.as_union_with(card.Bounds);
                }
            } break;
            }

            tableBounds = tableBounds == rect_f::Zero ? pileBounds : tableBounds.as_union_with(pileBounds);
        }
    }

    move_camera(tableBounds);
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

    _currentGame->use_hint();
    _fgCanvas.show_hint();
}

void card_table::set_theme(color_themes const& theme)
{
    _bgCanvas.set_background_colors(theme.TableBackgroundA, theme.TableBackgroundB);
}

void card_table::on_update(milliseconds deltaTime)
{
    if (!_currentGame) { return; }
    if (_currentGame->Status != game_status::Running) { reset(); }

    _bgCanvas.update(deltaTime);
    _fgCanvas.update(deltaTime);

    _cardRenderer.update(deltaTime);

    if (_camPosTween) { _camPosTween->update(deltaTime); }
    if (_camZoomTween) { _camZoomTween->update(deltaTime); }
}

void card_table::on_draw_to(gfx::render_target& target)
{
    _bgCanvas.draw(target);
    if (_hovered.Pile && _hovered.Pile->IsDragging) { _cardRenderer.mark_dirty(); }
    _cardRenderer.draw(target);
    _fgCanvas.draw(target);
}

void card_table::move_camera(rect_f const& cardBounds)
{
    using namespace tcob::tweening;

    auto const winSize {_camera.Size()};

    f32 const     hDiff {static_cast<f32>(winSize.Height - Bounds->Height)};
    f32 const     zoom {std::min(winSize.Width / cardBounds.Width, (winSize.Height - hDiff) / cardBounds.Height)};
    point_f const pos {cardBounds.get_center() + point_f {0, (Bounds->Y / zoom) / 2}};

    if (_camInstant) {
        _camera.look_at(pos);
        _camera.Zoom = {zoom, zoom};
        _bgCanvas.mark_dirty();
        _camInstant = false;
    } else {
        _camPosTween = make_unique_tween<linear_tween<point_f>>(0.75s, _camera.get_look_at(), pos);
        _camPosTween->start();
        _camPosTween->Value.Changed.connect([&](auto val) {
            if (auto* pile {_hovered.Pile}; pile && pile->IsDragging) {
                auto& cards {pile->Cards};
                for (isize i {_hovered.Index}; i < std::ssize(cards); ++i) {
                    cards[i].Bounds.move_by(val - _camera.get_look_at());
                }

                _dragRect = cards[_hovered.Index].Bounds;
            }
            _camera.look_at(val);
            _bgCanvas.mark_dirty();
        });

        _camZoomTween = make_unique_tween<linear_tween<size_f>>(0.75s, _camera.Zoom(), size_f {zoom, zoom});
        _camZoomTween->start();
        _camZoomTween->Value.Changed.connect([&](auto val) {
            _camera.Zoom = val;
            _bgCanvas.mark_dirty();
        });
    }
}

auto card_table::can_draw() const -> bool
{
    return _currentGame != nullptr;
}

void card_table::on_key_down(input::keyboard::event const& ev)
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
        } else if (ev.KeyCode == input::key_code::h && (ev.KeyMods & input::key_mod::LeftControl) == input::key_mod::LeftControl) {
            show_next_hint();
            ev.Handled = true;
        } else if (ev.KeyCode == input::key_code::c && (ev.KeyMods & input::key_mod::LeftControl) == input::key_mod::LeftControl) {
            _currentGame->collect_all();
            ev.Handled = true;
        }
    }
}

void card_table::on_mouse_motion(input::mouse::motion_event const& ev)
{
    if (!_currentGame || _currentGame->Status != game_status::Running) { return; }

    auto* pile {_hovered.Pile};
    if (_buttonDown) {
        if (pile) {
            drag_cards(ev);
            if (pile->IsDragging) { get_drop_target(); }
        }
    } else {
        get_hovered(ev.Position);
    }

    if (pile != _hovered.Pile) { HoverChange(_hovered.Pile); }

    ev.Handled = true;
}

void card_table::on_mouse_button_down(input::mouse::button_event const& ev)
{
    if (!_currentGame || _currentGame->Status != game_status::Running) { return; }

    if (ev.Button == input::mouse::button::Left) {
        _buttonDown = true;
        if (auto* pile {_hovered.Pile}) {
            if (pile->Type == pile_type::Stock && (pile->empty() || pile->Cards[pile->size() - 1].is_face_down())) {
                // deal card
                pile->remove_tint();
                _currentGame->deal_cards();
            } else if (ev.Clicks > 1) {
                // try move to foundation
                if (_currentGame->auto_play_cards(*pile)) {
                    get_hovered(ev.Position);
                    if (pile != _hovered.Pile) { HoverChange(_hovered.Pile); }
                }
            }
        }
    }
}

void card_table::on_mouse_button_up(input::mouse::button_event const& ev)
{
    if (!_currentGame || _currentGame->Status != game_status::Running) { return; }

    if (ev.Button == input::mouse::button::Left) {
        _buttonDown = false;

        auto* pile {_hovered.Pile};
        if (pile) {
            if (pile->IsDragging) {
                if (_dropTarget.Pile && pile) {
                    _currentGame->play_cards(*pile, *_dropTarget.Pile, _hovered.Index, std::ssize(pile->Cards) - _hovered.Index);
                } else {
                    layout();
                }
                pile->IsDragging = false;
                _fgCanvas.disable_hint();
            }
        }

        get_hovered(ev.Position);
        if (pile != _hovered.Pile) { HoverChange(_hovered.Pile); }
    }
}

void card_table::reset()
{
    _dropTarget = {};
    if (_hovered.Pile) { _hovered.Pile->IsDragging = false; }
    _hovered      = {};
    _camInstant   = true;
    _camPosTween  = nullptr;
    _camZoomTween = nullptr;
    _buttonDown   = false;
}

void card_table::drag_cards(input::mouse::motion_event const& ev)
{
    _hovered.Pile->IsDragging = false;

    auto& cards {_hovered.Pile->Cards};
    if (!_hovered.Pile->is_playable() || cards.empty() || _hovered.Index < 0) {
        mark_dirty();
        return;
    }

    auto const    zoom {_camera.Zoom()};
    point_f const off {ev.RelativeMotion.X / zoom.Width, ev.RelativeMotion.Y / zoom.Height};
    for (isize i {_hovered.Index}; i < std::ssize(cards); ++i) {
        cards[i].Bounds.move_by(off);
    }

    _dragRect                 = cards[_hovered.Index].Bounds;
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
            if (_settings->HintTarget) {
                _dropTarget.Pile->tint_cards(get_drop_color(), _dropTarget.Index);
            }
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
        auto const interSect {rect.as_intersection_with(candidate.Index == INDEX_MARKER
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
    _hovered = get_pile_at(point_i {_camera.convert_screen_to_world(pos)}, false);

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
        auto const top {p.size() - 1};

        if (ignoreHoveredPile && p.IsHovering) { return INDEX_INVALID; }
        if (p.Type == pile_type::Stock && p.empty() && _currentGame->state().Redeals == 0) { return INDEX_INVALID; }
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
    if (_settings->HintMovable) {
        auto const& moves {_currentGame->get_available_hints()};
        for (auto const& move : moves) {
            if (move.Src == pile && move.SrcCardIdx == idx) {
                return colors::LightGreen;
            }
        }
    }

    return colors::LightBlue;
}

auto card_table::get_drop_color() const -> color
{
    return colors::LightGreen; // TODO: add option to disable
}
}
