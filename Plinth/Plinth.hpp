
// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <memory>

#include "Common.hpp"

#include "Engine.hpp"
#include "MapRenderer.hpp"
#include "Player.hpp"
#include "Raycaster.hpp"
#include "TextureCache.hpp"

enum class game_action : u8 {
    MoveForward,
    MoveBackward,
    StrafeLeft,
    StrafeRight,
    RotateLeft,
    RotateRight,
    Fire,
    ToggleWall,
    ToggleMap
};

struct input_mapping {
    std::unordered_map<game_action, input::scan_code> KeyboardBindings {
        {game_action::MoveForward, input::scan_code::W},
        {game_action::MoveBackward, input::scan_code::S},
        {game_action::StrafeLeft, input::scan_code::A},
        {game_action::StrafeRight, input::scan_code::D},
        {game_action::RotateLeft, input::scan_code::LEFT},
        {game_action::RotateRight, input::scan_code::RIGHT},
        {game_action::Fire, input::scan_code::F},
        {game_action::ToggleWall, input::scan_code::SPACE},
        {game_action::ToggleMap, input::scan_code::TAB}};
};

class Plinth final : public scene {
public:
    explicit Plinth(game& game);
    ~Plinth() override;

    void on_start() override;

    void on_draw_to(gfx::render_target& target, transform const& xform) override;
    void on_fixed_update(milliseconds deltaTime) override;
    void on_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_controller_axis_motion(input::controller::axis_event const& ev) override;
    void on_controller_button_down(input::controller::button_event const& ev) override;
    void on_mouse_motion(input::mouse::motion_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;

    void on_text_input(input::keyboard::text_input_event const& ev) override;

private:
    void handle_action(game_action action);

    void move_player(milliseconds deltaTime);

    void action_toggle_wall();
    void action_toggle_map();
    void action_fire();

    void set_quality(quality_level level);

    std::unique_ptr<texture_cache> _cache;
    std::unique_ptr<level>         _level;
    player                         _player;
    std::unique_ptr<raycaster>     _raycaster;
    std::unique_ptr<map_renderer>  _mapRenderer;

    asset_owner_ptr<gfx::material> _material;
    asset_owner_ptr<gfx::texture>  _texture;
    gfx::renderer                  _renderer {gfx::buffer_usage_hint::StaticDraw};

    bool _drawMap {false};
    f32  _mouseX {0};
    bool _controllerFireDown {false};

    utf8_string _keyCache;

    std::vector<gfx::image_frame> _frames;
    milliseconds                  _frameTimer {0};
    bool                          _startRecord {false};
    std::future<bool>             _clipFtr;

    quality_settings _quality;
    input_mapping    _inputMapping {};
    engine           _engine;
};
