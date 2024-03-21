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

auto field::get_hover_color() const -> color
{
    return colors::LightBlue;
}

auto field::get_drop_color() const -> color
{
    return colors::LightGreen;
}

void field::start(std::shared_ptr<games::base_game> const& game, bool cont)
{
    _text.hide();

    if (_currentGame) {
        _currentGame->save(_saveGame);
    }

    _dropTarget   = {};
    _hovered      = {};
    _manualCamera = false;

    _currentGame = game;

    _cardQuads.clear();
    _cardQuads.resize(_currentGame->info().DeckCount * 52);

    auto const cardSize {get_card_size()};
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

                    if (_isDragging && pile->is_active()) {
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

        _cardQuadsDirty = false;
        move_camera(bounds);
    } else {
        _cardRenderer.set_geometry(_cardQuads);
        _cardRenderer.render_to_target(target);
    }
}

void field::move_camera(size_f bounds)
{
    if (!_manualCamera) {
        auto&      camera {*_parentWindow->Camera};
        auto const winSize {_parentWindow->Size()};

        f32 const     off {static_cast<f32>(winSize.Height - _size.Height)};
        f32 const     zoom {std::min(winSize.Width / bounds.Width, (winSize.Height - off) / bounds.Height)};
        point_f const pos {bounds.Width / 2, (bounds.Height + (off / zoom)) / 2};
        camera.look_at(pos);
        camera.set_zoom({zoom, zoom});
    }
}

void field::get_pile_quads(std::vector<gfx::quad>::iterator& quadIt, pile const* pile) const
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
    using namespace tcob::enum_ops;

    if (!_currentGame) { return; }

    if (!ev.Repeat) {
        if (ev.KeyCode == input::key_code::c) {
            if (_currentGame) {
                _currentCardSet++;
                if (_currentCardSet > _maxCardSet) { _currentCardSet = 1; }
                _mat = _resGrp.get<gfx::material>("mat-cards" + std::to_string(_currentCardSet));
                create_markers(get_card_size());
                _currentGame->restart();
                ev.Handled = true;
            }
        }
    }

    _currentGame->key_down(ev);
}

void field::on_mouse_motion(input::mouse::motion_event& ev)
{
    if (input::system::IsMouseButtonDown(input::mouse::button::Right)) {
        auto&         camera {*_parentWindow->Camera};
        size_f const  zoom {camera.get_zoom()};
        point_f const off {-ev.RelativeMotion.X / zoom.Width, -ev.RelativeMotion.Y / zoom.Height};
        camera.move_by(off);
        _manualCamera = true;
    }

    if (!_currentGame) { return; }

    if (_buttonDown) {
        if (_hovered.Pile) { drag_cards(ev); }
        if (_isDragging) { check_drop_pile(); }
    } else {
        check_hover_pile(ev.Position);
    }

    HoverChange(_currentGame->get_description(_hovered.Pile));
    ev.Handled = true;
}

void field::on_mouse_wheel(input::mouse::wheel_event& ev)
{
    auto& camera {*_parentWindow->Camera};
    if (ev.Scroll.Y > 0) {
        camera.zoom_by({1.1f, 1.1f});
    } else {
        camera.zoom_by({1 / 1.1f, 1 / 1.1f});
    }
    _manualCamera = true;
    ev.Handled    = true;
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
        cards[i].Bounds.move_by(off);
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
            oldPile.Pile->remove_tint();
            mark_dirty();
        }
        if (_dropTarget.Pile) {
            _dropTarget.Pile->tint_cards(get_drop_color(), _dropTarget.Index);
            mark_dirty();
        }
    }
}

void field::check_hover_pile(point_i pos)
{
    auto oldPile {_hovered};
    _hovered = _currentGame->hover_at(point_i {(*_parentWindow->Camera).convert_screen_to_world(pos)});

    if (oldPile.Pile) {
        oldPile.Pile->set_active(false, oldPile.Index, get_hover_color());
        mark_dirty();
    }
    if (_hovered.Pile) {
        _hovered.Pile->set_active(true, _hovered.Index, get_hover_color());
        mark_dirty();
    }
}

auto field::get_card_size() const -> size_f
{
    return _cardSets[_currentCardSet]->get_card_size();
}

}
