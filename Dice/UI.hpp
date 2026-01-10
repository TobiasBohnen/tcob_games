// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

class game_form : public ui::form<ui::grid_layout> {
public:
    game_form(rect_f const& bounds, assets::group const& grp, shared_state& state, event_bus& events);

protected:
    void on_update(milliseconds deltaTime) override;

private:
    void gen_styles(assets::group const& grp);

    shared_state& _sharedState;

    bool                          _updateDmd {true};
    asset_owner_ptr<gfx::texture> _dmdTexture;
    bool                          _updateSsd0 {true};
    bool                          _updateSsd1 {true};
};

////////////////////////////////////////////////////////////

class game_select_form : public ui::form<ui::manual_layout> {
public:
    signal<u32 const> StartGame;

    game_select_form(rect_f const& bounds, assets::group const& grp, std::map<u32, game_def> const& games);

private:
    void gen_styles(assets::group const& grp);
    auto make_tooltip(u32 id) -> std::shared_ptr<ui::tooltip>;

    std::map<u32, game_def> _games;
};