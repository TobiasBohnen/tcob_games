// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Field.hpp"

#include "Cards.hpp"
#include "Games.hpp"

namespace solitaire {
static char const* SAVE_NAME {"save.ini"};

field::field(gfx::window* parent, size_i size, assets::group& resGrp)
    : _parentWindow {parent}
    , _resGrp {resGrp}
    , _cardRenderer {gfx::buffer_usage_hint::DynamicDraw}
    , _text {resGrp.get<gfx::font_family>("Poppins")->get_font({.Weight = gfx::font::weight::Bold}, 96)}
    , _size {size}

{
    _saveGame.load(SAVE_NAME);

    _text.Bounds = {point_f::Zero, size_f {_size}};
    auto& effects {_text.get_effects()};
    effects.add(1, gfx::make_unique_quad_tween<gfx::wave_effect>(3s, {30, 4.f}));
    effects.start_all(playback_style::Looped);
    _text.hide();

    _cardSets.push_back(std::make_shared<cardset>("mat-cards1", resGrp));
    _mat = {resGrp.get<gfx::material>("mat-cards1")};
}

auto field::get_size() const -> size_i
{
    return _size;
}

auto field::get_material() const -> assets::asset_ptr<gfx::material> const&
{
    return _mat;
}

void field::start(std::shared_ptr<games::base_game> const& game, bool cont)
{
    _text.hide();

    if (_currentGame) {
        _currentGame->save(_saveGame);
    }

    _dropTarget = {};
    _hovered    = {};

    _currentGame = game;

    _cardQuads.clear();
    _cardQuads.resize(_currentGame->info().DeckCount * 52);

    auto const cardSize {calc_card_size()};
    if (cont) {
        _currentGame->start(cardSize, _saveGame);
    } else {
        _currentGame->start(cardSize, std::nullopt);
    }
    create_markers(cardSize);
}

void field::undo()
{
    if (_currentGame && _currentGame->can_undo()) {
        _currentGame->undo();
    }
}

void field::quit()
{
    if (_currentGame) {
        _currentGame->save(_saveGame);
        _saveGame.save(SAVE_NAME);
    }
}

void field::create_markers(size_f const& cardSize)
{
    _markerSprites.clear();
    for (auto const& [_, piles] : _currentGame->piles()) {
        for (auto* pile : piles) {
            if (!pile->HasMarker) { continue; }

            pile->Marker                = _markerSprites.create_sprite();
            pile->Marker->Material      = _mat;
            pile->Marker->TextureRegion = pile->get_marker_texture_name();
            pile->Marker->Bounds        = {multiply(pile->Position, cardSize), cardSize};
        }
    }
}

void field::mark_dirty()
{
    _cardQuadsDirty = true;
}

auto field::state() const -> game_state
{
    return _currentGame ? _currentGame->state() : game_state::Initial;
}

void field::on_update(milliseconds deltaTime)
{
    _markerSprites.update(deltaTime);

    auto const gameState {state()};
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
}

void field::on_fixed_update(milliseconds /* deltaTime */)
{
}

void field::on_draw_to(gfx::render_target& target)
{
    _markerSprites.draw_to(target);

    draw_cards(target);

    _text.draw_to(target);
}

void field::draw_cards(gfx::render_target& target)
{
    if (_currentGame && (_cardQuadsDirty || _isDragging)) {
        _cardRenderer.set_material(_mat);

        pile const* dragPile {nullptr};
        {
            auto quadIt {_cardQuads.begin()};
            for (auto const& [_, piles] : _currentGame->piles()) {
                for (auto const* pile : piles) {
                    if (_isDragging && pile->is_hovering()) {
                        dragPile = pile;
                    } else {
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

        _cardQuadsDirty = false;
    } else {
        _cardRenderer.set_geometry(_cardQuads);
        _cardRenderer.render_to_target(target);
    }
}

void field::get_pile_quads(std::vector<gfx::quad>::iterator& quadIt, pile const* pile)
{
    for (auto const& card : pile->Cards) {
        auto& quad {*quadIt};
        gfx::geometry::set_color(quad, card.Color);
        gfx::geometry::set_texcoords(quad, _mat->Texture->get_region(card.get_texture_name()));
        gfx::geometry::set_position(quad, card.Bounds);
        ++quadIt;
    }
}

auto field::can_draw() const -> bool
{
    return _currentGame != nullptr;
}

void field::on_key_down(input::keyboard::event& ev)
{
    // TODO: check if any form has focus

    using namespace tcob::enum_ops;

    if (!_currentGame) { return; }

    if (!ev.Repeat) {
        if (ev.KeyCode == input::key_code::c) {
            if (_currentGame) {
                _currentCardSet++;
                if (_currentCardSet > _maxCardSet) { _currentCardSet = 1; }
                _mat = _resGrp.get<gfx::material>("mat-cards" + std::to_string(_currentCardSet));
                create_markers(calc_card_size());
                _currentGame->restart();
                ev.Handled = true;
            }
        }
    }

    _currentGame->key_down(ev);
}

void field::on_mouse_motion(input::mouse::motion_event& ev)
{
    if (!_currentGame) { return; }

    if (_buttonDown) {
        if (_hovered.Pile) { drag_cards(ev); }
        if (_isDragging) { check_drop_pile(); }
    } else {
        check_hover_pile(ev.Position);
    }

    HoverChange(_currentGame->get_description(_hovered.Pile));
}

void field::on_mouse_button_down(input::mouse::button_event& ev)
{
    if (!_currentGame) { return; }

    if (ev.Button == input::mouse::button::Left) {
        _buttonDown = true;
        _currentGame->click(_hovered.Pile, ev.Clicks);

        if (ev.Clicks > 1) { check_hover_pile(ev.Position); }
        HoverChange(_currentGame->get_description(_hovered.Pile));
    }
}

void field::on_mouse_button_up(input::mouse::button_event& ev)
{
    if (!_currentGame) { return; }

    if (ev.Button == input::mouse::button::Left) {
        _buttonDown = false;
        if (_isDragging) {
            _currentGame->drop_cards(_hovered, _dropTarget);
            _isDragging = false;
        }

        check_hover_pile(ev.Position);
        HoverChange(_currentGame->get_description(_hovered.Pile));
    }
}

void field::drag_cards(input::mouse::motion_event const& ev)
{
    _isDragging = false;

    auto& cards {_hovered.Pile->Cards};
    if (!_hovered.Pile->is_playable() || cards.empty() || _hovered.Index < 0) {
        mark_dirty();
        return;
    }

    size_f const  zoom {(*_parentWindow->Camera).get_zoom()};
    point_f const off {ev.RelativeMotion.X / zoom.Width, ev.RelativeMotion.Y / zoom.Height};
    for (isize i {_hovered.Index}; i < std::ssize(cards); ++i) {
        auto& card {cards[i]};
        card.Bounds.move_by(off);
    }

    _dragRect   = cards[_hovered.Index].Bounds;
    _isDragging = true;
    mark_dirty();
}

void field::check_drop_pile()
{
    auto        oldPile {_dropTarget};
    auto const& card {_hovered.Pile->Cards[_hovered.Index]};
    isize const numCards {std::ssize(_hovered.Pile->Cards) - _hovered.Index};
    _dropTarget = _currentGame->drop_target_at(_dragRect, card, numCards);

    if (_dropTarget.Pile != oldPile.Pile || _dropTarget.Index != oldPile.Index) {
        if (oldPile.Pile) {
            oldPile.Pile->remove_color();
            mark_dirty();
        }
        if (_dropTarget.Pile) {
            _dropTarget.Pile->color_cards(COLOR_DROP, _dropTarget.Index);
            mark_dirty();
        }
    }
}

void field::check_hover_pile(point_i pos)
{
    auto oldPile {_hovered};
    _hovered = _currentGame->hover_at(point_i {(*_parentWindow->Camera).convert_screen_to_world(pos)});

    if (_hovered.Pile != oldPile.Pile || _hovered.Index != oldPile.Index) {
        if (oldPile.Pile) {
            oldPile.Pile->set_hovering(false, oldPile.Index);
            mark_dirty();
        }
        if (_hovered.Pile) {
            _hovered.Pile->set_hovering(true, _hovered.Index);
            mark_dirty();
        }
    }
}

auto field::calc_card_size() -> size_f
{
    size_f max {0, 2};
    for (auto const& [_, piles] : _currentGame->piles()) {
        for (auto const* pile : piles) {
            isize const countTrue {std::min<isize>(20, std::ranges::count(pile->Initial, true))};
            isize const countFalse {std::min<isize>(20, std::ranges::count(pile->Initial, false))};
            f32 const   c {(countFalse / FACE_DOWN_OFFSET) + (countTrue / FACE_UP_OFFSET)};

            if (pile->Layout == layout_type::Column) {
                max.Width  = std::max(pile->Position.X * CARD_PADDING, max.Width);
                max.Height = std::max((pile->Position.Y + c) * CARD_PADDING, max.Height);
            } else if (pile->Layout == layout_type::Row || pile->Layout == layout_type::Fan) {
                max.Width  = std::max((pile->Position.X + c) * CARD_PADDING, max.Width);
                max.Height = std::max(pile->Position.Y * CARD_PADDING, max.Height);
            } else {
                max.Width  = std::max(pile->Position.X * CARD_PADDING, max.Width);
                max.Height = std::max(pile->Position.Y * CARD_PADDING, max.Height);
            }
        }
    }

    max.Width++;
    max.Height++;

    auto const   tex {_mat->Texture};
    size_i const texSize {tex->get_size()};
    rect_f const uvRect {tex->get_region("card_back").UVRect};
    size_f const texRegSize {std::abs(uvRect.Width * texSize.Width), std::abs(uvRect.Height * texSize.Height)};
    f32 const    texAsp {texRegSize.Width / static_cast<f32>(texRegSize.Height)};

    auto const windowSize {_size};

    // TODO: get size from cardset and adjust zoom
    size_f retValue;
    retValue.Width  = windowSize.Width / max.Width;
    retValue.Height = retValue.Width / texAsp;

    if (retValue.Height * max.Height > windowSize.Height) {
        retValue.Width *= windowSize.Height / (retValue.Height * max.Height);
        retValue.Height = retValue.Width / texAsp;
    }

    return retValue;
}

}
