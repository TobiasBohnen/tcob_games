// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Wizard.hpp"
#include "ui/Themes.hpp"

namespace solitaire {

class wizard_scene : public scene {
    struct ev_args {
        std::string Name;
        std::string Path;
    };

public:
    wizard_scene(game& game);

    signal<ev_args const> GameGenerated;

    void update_theme(color_themes const& currentTheme);

protected:
    void on_start() override;

    void on_draw_to(gfx::render_target& target) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event& ev) override;

private:
    std::shared_ptr<form_wizard> _formWizard {};
};

}
