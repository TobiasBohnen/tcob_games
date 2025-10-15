// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "CardRenderer.hpp"

#include "CardTable.hpp"
#include "games/Games.hpp"

namespace solitaire {

card_renderer::card_renderer(card_table& parent)
    : _parent {parent}
    , _cardRenderer {gfx::buffer_usage_hint::DynamicDraw}
{
}

void card_renderer::start()
{
    _cardQuads.clear();
    _cardQuads.resize(_parent.game()->info().DeckCount * 52);
    create_markers();
}

void card_renderer::draw(gfx::render_target& target)
{
    _markerSprites.draw_to(target);
    draw_cards(target);
}

void card_renderer::update(milliseconds deltaTime)
{
    _markerSprites.update(deltaTime);
}

void card_renderer::mark_dirty()
{
    _renderDirty = true;
}

void card_renderer::set_cardset(card_set const& cardset)
{
    _cardSet = &cardset;
    if (_parent.game()) {
        if (!_markerSprites.is_empty()) { create_markers(); }
    }
}

void card_renderer::draw_cards(gfx::render_target& target)
{
    auto const& pass {_cardSet->get_material()->first_pass()};

    if (_renderDirty) {
        pile const* dragPile {nullptr};
        {
            auto quadIt {_cardQuads.begin()};
            for (auto const& [_, piles] : _parent.game()->piles()) {
                for (auto const* pile : piles) {
                    if (pile->IsDragging) {
                        dragPile = pile;
                    } else {
                        get_pile_quads(quadIt, pile);
                    }
                }
            }

            _cardRenderer.set_geometry({_cardQuads.begin(), quadIt}, &pass);
            _cardRenderer.render_to_target(target);
        }
        if (dragPile) {
            auto quadIt {_cardQuads.begin()};
            get_pile_quads(quadIt, dragPile);

            _cardRenderer.set_geometry({_cardQuads.begin(), quadIt}, &pass);
            _cardRenderer.render_to_target(target);
        }

        _renderDirty = false;
    } else {
        _cardRenderer.set_geometry(_cardQuads, &pass);
        _cardRenderer.render_to_target(target);
    }
}

void card_renderer::get_pile_quads(std::vector<gfx::quad>::iterator& quadIt, pile const* pile) const
{
    auto const mat {_cardSet->get_material()};
    for (auto const& card : pile->Cards) {
        auto& quad {*quadIt};
        gfx::geometry::set_color(quad, card.Color);
        gfx::geometry::set_texcoords(quad, mat->first_pass().Texture->regions()[card.get_texture_name()]);
        gfx::geometry::set_position(quad, card.Bounds);
        ++quadIt;
    }
}

void card_renderer::create_markers()
{
    auto const& cardSize {_cardSet->get_card_size()};
    _markerSprites.clear();
    for (auto const& [_, piles] : _parent.game()->piles()) {
        for (auto* pile : piles) {
            if (!pile->HasMarker) { continue; }

            pile->Marker                = &_markerSprites.create_shape<gfx::rect_shape>();
            pile->Marker->Material      = _cardSet->get_material();
            pile->Marker->TextureRegion = pile->get_marker_texture_name();
            pile->Marker->Bounds        = {multiply(pile->Position, cardSize), cardSize};
        }
    }
}

auto card_renderer::get_card_size() const -> size_f
{
    return _cardSet->get_card_size();
}

} // namespace solitaire
