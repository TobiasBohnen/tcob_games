// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Game.hpp"

class start_scene : public scene {

public:
    start_scene(game& game);
    ~start_scene() override;

protected:
    void on_start() override;

    void on_draw_to(gfx::render_target& target) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event const& ev) override;

private:
    void start_game(u32 id);

    void scan_games();
    auto scan_game(string const& ini) -> bool;

    std::shared_ptr<game_select_form> _selectForm {};

    std::map<u32, game_def>    _games;
    std::shared_ptr<dice_game> _currentGame {};
    u32                        _currentGameID {0};
    scene_node*                _gameNode {nullptr};
    u32                        _queuedGameID {0};
    bool                       _quitQueued {false};

    std::unique_ptr<event_bus> _events;

    std::vector<gfx::image_frame> _frames;
    milliseconds                  _frameTimer {0};
    bool                          _startRecord {false};
    std::future<bool>             _clipFtr;
};
