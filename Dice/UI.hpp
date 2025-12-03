// Copyright (c) 2025 Tobias Bohnen
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

    bool _updateDmd {true};
    bool _updateSsd0 {true};
    bool _updateSsd1 {true};
};