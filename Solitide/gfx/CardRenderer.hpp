// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"      // IWYU pragma: keep
#include "games/Piles.hpp" // IWYU pragma: keep
#include "gfx/CardSet.hpp"

namespace solitaire {

////////////////////////////////////////////////////////////

class quad_renderer final : public gfx::renderer {
public:
    explicit quad_renderer();

    void set_geometry(std::span<gfx::quad const> quads, gfx::pass const* pass);

private:
    void prepare(usize quadCount);
    void on_render_to_target(gfx::render_target& target) override;

    gfx::pass const* _pass {nullptr};
    usize            _numQuads {0};

    gfx::vertex_array _vertexArray;
};

////////////////////////////////////////////////////////////

class card_renderer {
public:
    card_renderer(card_table& parent);

    void start();

    void create_markers();

    void draw(gfx::render_target& target);
    void update(milliseconds deltaTime);

    void mark_dirty();

    void set_cardset(card_set const& cardset);
    auto get_card_size() const -> size_f;

private:
    void get_pile_quads(std::vector<gfx::quad>::iterator& quadIt, pile const* pile) const;
    void draw_cards(gfx::render_target& target);

    card_table& _parent;

    card_set const*        _cardSet {nullptr};
    gfx::shape_batch       _markerSprites;
    quad_renderer          _cardRenderer;
    std::vector<gfx::quad> _cardQuads;
    bool                   _renderDirty {true};
};

}
