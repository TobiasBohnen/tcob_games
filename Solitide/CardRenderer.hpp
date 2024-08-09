// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "CardSet.hpp"
#include "Common.hpp" // IWYU pragma: keep
#include "Piles.hpp"

namespace solitaire {

////////////////////////////////////////////////////////////

class card_renderer {
public:
    card_renderer(card_table& parent);

    void start();

    void create_markers();

    void draw(gfx::render_target& target);
    void update(milliseconds deltaTime);

    void mark_dirty();

    void set_card_set(std::shared_ptr<card_set> cardset);

private:
    void get_pile_quads(std::vector<gfx::quad>::iterator& quadIt, pile const* pile) const;
    void draw_cards(gfx::render_target& target);

    card_table& _parent;

    std::shared_ptr<card_set> _cardSet;
    gfx::mesh_batch           _markerSprites;
    gfx::quad_renderer        _cardRenderer;
    std::vector<gfx::quad>    _cardQuads;
    bool                      _renderDirty {true};
};

}
