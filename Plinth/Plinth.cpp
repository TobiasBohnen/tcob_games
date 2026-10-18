// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Plinth.hpp"

#include <numeric>
#include <unordered_map>

#include "Common.hpp"
#include "Level.hpp"
#include "Light.hpp"
#include "MapRenderer.hpp"
#include "Raycaster.hpp"
#include "Voxel.hpp"

auto static deathEffect {[](auto& screen) {
    static std::vector<usize> indices;
    static usize              currentIndex {0};
    static bool               active {false};

    if (!active) {
        indices.resize(screen.size());
        std::ranges::iota(indices, 0);
        random::shuffle {}(indices);
        currentIndex = 0;
        active       = true;
    }

    usize const pixelsPerFrame {500};
    usize const endIndex {std::min(indices.size(), currentIndex + pixelsPerFrame)};

    for (usize i {0}; i < endIndex; ++i) {
        usize const idx {indices[i]};
        screen[idx] = 0xFF000000u | (static_cast<u32>(0) << 16) | (static_cast<u32>(0) << 8) | static_cast<u32>(255);
    }

    currentIndex = endIndex;
}};

auto static cheaterEffect {[](auto& screen) {
    for (auto& col : screen) {
        color c {color::FromABGR(col)};
        std::swap(c.R, c.B);
        col = 0xFF000000u | (static_cast<u32>(c.B) << 16) | (static_cast<u32>(c.G) << 8) | static_cast<u32>(c.R);
    }
}};

auto static nightVisionEffect {[](auto& screen) {
    for (auto& col : screen) {
        color     c {color::FromABGR(col)};
        u32 const luminance {static_cast<u32>((c.R * 0.3f) + (c.G * 0.59f) + (c.B * 0.11f))};
        c.R = 0;
        c.B = 0;
        c.G = static_cast<u8>(std::min(255u, luminance + 50));
        col = 0xFF000000u | (static_cast<u32>(c.B) << 16) | (static_cast<u32>(c.G) << 8) | static_cast<u32>(c.R);
    }
}};

auto static staticNoiseEffect {[](auto& screen) {
    rng randomGenerator;
    for (auto& col : screen) {
        if (randomGenerator(0, 12) == 0) {
            u8 const noise {static_cast<u8>(randomGenerator(0, 255))};
            col = 0xFF000000u | (static_cast<u32>(noise) << 16) | (static_cast<u32>(noise) << 8) | static_cast<u32>(noise);
        }
    }
}};

auto static grayscaleEffect {[](auto& screen) {
    for (auto& col : screen) {
        color const c {color::FromABGR(col)};
        u8 const    gray {static_cast<u8>((c.R * 0.299f) + (c.G * 0.587f) + (c.B * 0.114f))};
        col = 0xFF000000u | (static_cast<u32>(gray) << 16) | (static_cast<u32>(gray) << 8) | static_cast<u32>(gray);
    }
}};

// PLACEHOLDER START

Plinth::Plinth(game& game)
    : scene {game}
    , _cache {std::make_unique<texture_cache>()}
{
    // window().SystemCursorEnabled = false;
    window().RelativeMouseMode = true;

    _material->first_pass().Texture = _texture;
    _engine.run("res/main.lua");

    _level = std::make_unique<level>();
    _engine.generate_level(*_level, _player);

    rng                         rng;
    std::unordered_set<point_i> used;
    auto const                  find_empty {[&]() {
        i32 x {0}, y {0};
        for (;;) {
            x = rng(0, MAP_WIDTH - 1);
            y = rng(0, MAP_HEIGHT - 1);
            if (!used.contains(point_i {x, y}) && _level->is_wall_clear(point_d {static_cast<f64>(x), static_cast<f64>(y)}, 1)) { break; }
        }
        used.insert(point_i {x, y});
        return point_d {static_cast<f64>(x) + 0.5f, static_cast<f64>(y) + 0.5f};
    }};

    static voxel_grid const testVoxelGrid {*voxel_grid::Load("res/ball1.vox")};

    for (i32 i {0}; i < 50; ++i) {
        f64       scale {rng(0.3, 1.0)};
        f64 const maxDim {static_cast<f64>(std::max({testVoxelGrid.Size.X, testVoxelGrid.Size.Y, testVoxelGrid.Size.Z}))};
        _level->spawn({.Position = find_empty(),
                       .Radius   = (std::max(testVoxelGrid.Size.X, testVoxelGrid.Size.Y) / 2.0) * scale / maxDim,
                       .Visual   = voxel_object {
                           .Scale = scale / maxDim,
                           .BaseZ = 0.0,
                           .Yaw   = degree_d {90.0 * i},
                           .Grid  = &testVoxelGrid,
                       }});
    }

    degree_d const angle {180};
    radian_d const rad {angle - degree_d {90}};
    _player.Direction = point_d::FromDirection(angle);
    _player.Plane     = {-rad.sin(), rad.cos()};

    _raycaster = std::make_unique<raycaster>(*_cache);
    set_quality(quality_level::High);
}

Plinth::~Plinth() = default;

void Plinth::on_start()
{
    _cache->load();
}

void Plinth::on_draw_to(gfx::render_target& target, transform const& xform)
{
    _texture->update_data(_raycaster->draw(*_level, _player), 0);
    if (_drawMap) {
        _texture->update_data(_mapRenderer->draw(*_level, _player), 0);
    }

    // aspect ratio correction
    size_i const size {*target.Size};

    f32 const srcAspect {static_cast<f32>(_quality.Resolution.Width) / static_cast<f32>(_quality.Resolution.Height)};
    f32 const dstAspect {static_cast<f32>(size.Width) / static_cast<f32>(size.Height)};

    f32 destWidth {static_cast<f32>(size.Width)};
    f32 destHeight {static_cast<f32>(size.Height)};

    if (dstAspect > srcAspect) {
        destWidth = destHeight * srcAspect;
    } else {
        destHeight = destWidth / srcAspect;
    }

    gfx::quad q {};
    gfx::geometry::set_color(q, colors::White);
    gfx::geometry::set_position(q, {0, 0, destWidth, destHeight});
    gfx::geometry::set_texcoords(q, {.UVRect = {0, 0, 1, 1}, .Level = 0});
    _renderer.set_geometry({.Vertices = q, .Indices = gfx::QuadIndicies, .Type = gfx::primitive_type::Triangles}, &_material->first_pass());
    _renderer.render_to_target(target, transform::Identity);
}

void Plinth::on_fixed_update(milliseconds deltaTime)
{
    auto const& stats {locate_service<gfx::render_system>().statistics()};
    auto const& mouse {locate_service<input::system>().mouse().get_position()};
    window().Title = std::format("Plinth | FPS avg:{:.2f} best:{:.2f} worst:{:.2f} | x:{} y:{} ",
                                 stats.average_FPS(), stats.best_FPS(), stats.worst_FPS(),
                                 mouse.X, mouse.Y);
}

void Plinth::on_update(milliseconds deltaTime)
{
    move_player(deltaTime);
    _player.bob(deltaTime);
    _level->update(deltaTime);

    // _frameTimer += deltaTime;
    // f64 const flicker {1.0 + (0.06 * std::sin(_frameTimer.count() / 1000.0 * 17.0)) + (0.04 * std::sin((_frameTimer.count() / 1000.0 * 29.3) + 1.7))};
    // _player.Settings.LightIntensity = 2 * flicker;

    for (auto& dl : _level->objects()) {
        if (!dl.sprite() && !dl.light()) { continue; }
        dl.set_position(dl.position() + _player.Direction * deltaTime.count() / 1000 * 10);
    }

    if (_startRecord) {
        if (_clipFtr.valid()) {
            if (_clipFtr.wait_for(0ms) == std::future_status::ready) {
                logger::Info("capture saved");
                _startRecord = false;
                _frames.clear();
                _clipFtr = {};
            }
            return;
        }

        _frameTimer += deltaTime;
        if (_frameTimer >= 50ms) {
            auto img {window().copy_to_image()};
            _frames.push_back({.Image = img, .Duration = _frameTimer});
            _frameTimer = 0ms;
        }
        if (_frames.size() == 200) {
            logger::Info("capture done. saving...");
            _clipFtr = gfx::save_animation_async("clip.gif", _frames);
        }
    }
}

void Plinth::move_player(milliseconds deltaTime)
{
    constexpr f64 keyboardRotateSpeed {3.0};   // radians/second
    constexpr f64 mouseSensitivity {0.002};    // radians/pixel
    constexpr f64 controllerRotateSpeed {3.0}; // radians/second
    constexpr f64 controllerDeadzone {5000.0};
    constexpr f64 maxAxisValue {32767.0};

    auto const& input {locate_service<input::system>()};

    f64 const deltaSeconds {deltaTime.count() / 1000.0};
    f64 const moveSpeed {deltaSeconds * _player.Settings.MoveSpeed};

    f64 forwardAmount {0.0};
    f64 strafeAmount {0.0};
    f64 rotateAmount {0.0};

    if (input.InputMode == input::mode::KeyboardMouse) {
        auto const& keyboard {input.keyboard()};

        auto const isDown {[&keyboard, this](game_action action) {
            auto const it {_inputMapping.KeyboardBindings.find(action)};
            return it != _inputMapping.KeyboardBindings.end() && keyboard.is_key_down(it->second);
        }};

        if (isDown(game_action::MoveForward)) { forwardAmount += moveSpeed; }
        if (isDown(game_action::MoveBackward)) { forwardAmount -= moveSpeed; }

        if (isDown(game_action::StrafeLeft)) { strafeAmount += moveSpeed; }
        if (isDown(game_action::StrafeRight)) { strafeAmount -= moveSpeed; }

        if (isDown(game_action::RotateRight)) { rotateAmount += deltaSeconds * keyboardRotateSpeed; }
        if (isDown(game_action::RotateLeft)) { rotateAmount -= deltaSeconds * keyboardRotateSpeed; }

        rotateAmount += static_cast<f64>(_mouseX) * mouseSensitivity;
    } else {
        static auto const normalizeAxis {[](auto value) -> f64 {
            f64 const axis {static_cast<f64>(value)};
            f64 const magnitude {std::abs(axis)};

            if (magnitude <= controllerDeadzone) { return 0.0; }

            f64 const normalizedMagnitude {std::clamp((magnitude - controllerDeadzone) / (maxAxisValue - controllerDeadzone), 0.0, 1.0)};
            return std::copysign(normalizedMagnitude, axis);
        }};

        auto const& controller {input.first_controller()};

        if (controller.is_button_pressed(input::controller::button::DPadUp)) { forwardAmount += moveSpeed; }
        if (controller.is_button_pressed(input::controller::button::DPadDown)) { forwardAmount -= moveSpeed; }

        if (controller.is_button_pressed(input::controller::button::DPadLeft)) { strafeAmount += moveSpeed; }
        if (controller.is_button_pressed(input::controller::button::DPadRight)) { strafeAmount -= moveSpeed; }

        forwardAmount -= normalizeAxis(controller.get_axis_value(input::controller::axis::LeftY)) * moveSpeed;
        strafeAmount -= normalizeAxis(controller.get_axis_value(input::controller::axis::LeftX)) * moveSpeed;
        rotateAmount += normalizeAxis(controller.get_axis_value(input::controller::axis::RightX)) * deltaSeconds * controllerRotateSpeed;
    }

    if (f64 const movementLength {std::hypot(forwardAmount, strafeAmount)};
        movementLength > moveSpeed && movementLength > 0.0) {
        f64 const scale {moveSpeed / movementLength};
        forwardAmount *= scale;
        strafeAmount *= scale;
    }

    _mouseX = 0.0;
    _player.move(*_level, forwardAmount, strafeAmount, rotateAmount);
}

void Plinth::action_toggle_wall()
{
    _level->toggle_wall(point_i {_player.Position + _player.Direction});
}

void Plinth::action_toggle_map()
{
    _drawMap = !_drawMap;
}

void Plinth::action_fire()
{
    sprite_object const sprite {.BaseZ     = 0.25,
                                .Size      = {0.05, 0.15},
                                .Texture   = ballTexture,
                                .LightMask = LIGHT_LAYER_SPELL};

    dynamic_light light {.Range     = 0.5,
                         .Color     = colors::Red,
                         .Intensity = 1.5,
                         .Layers    = LIGHT_LAYER_WORLD | LIGHT_LAYER_SPELL};
    light.Z = sprite.BaseZ + (sprite.Size.Height * 0.5);

    _level->spawn({.Position = _player.Position,
                   .Visual   = sprite,
                   .Light    = light});
}

void Plinth::handle_action(game_action action)
{
    switch (action) {
    case game_action::Fire:       action_fire(); break;
    case game_action::ToggleWall: action_toggle_wall(); break;
    case game_action::ToggleMap:  action_toggle_map(); break;
    default:
        break;
    }
}

constexpr auto make_quality(quality_level level) -> quality_settings
{
    switch (level) {
    case quality_level::Low:    return {.Resolution = {320, 180}, .VoxelPixelBudget = 8000, .LightLosSamples = 1, .VoxelAo = false, .Dithering = false, .ContactShadows = false};
    case quality_level::Medium: return {.Resolution = {480, 270}, .VoxelPixelBudget = 14000, .LightLosSamples = 3, .VoxelAo = true, .Dithering = true, .ContactShadows = true};
    case quality_level::High:   return {.Resolution = {640, 360}, .VoxelPixelBudget = 20000, .LightLosSamples = 5, .VoxelAo = true, .Dithering = true, .ContactShadows = true};
    case quality_level::Ultra:  return {.Resolution = {960, 540}, .VoxelPixelBudget = 40000, .LightLosSamples = 5, .VoxelAo = true, .Dithering = true, .ContactShadows = true};
    case quality_level::Custom: break;
    }

    return {};
}

void Plinth::set_quality(quality_level level)
{
    _quality = make_quality(level);

    f64 const fov {FOV * TAU / 360.0};
    _player.Plane = _player.Plane * (std::tan(fov / 2.0) / _player.Plane.length());

    _texture->resize(_quality.Resolution, 1, gfx::texture::format::RGBA8);
    _texture->Filtering = gfx::texture::filtering::NearestNeighbor;

    _player.Settings.BobHeight = _quality.Resolution.Height / 60.0;

    _raycaster->set_quality(_quality, (_quality.Resolution.Width / 2.0) / std::tan(fov / 2.0));
    _mapRenderer = std::make_unique<map_renderer>(*_cache, _quality.Resolution);
}

void Plinth::on_key_down(input::keyboard::event const& ev)
{
    if (ev.Repeat) { return; }

    switch (ev.ScanCode) { // NOLINT
#ifdef TCOB_DEBUG
    case input::scan_code::BACKSPACE: {
        parent().pop_current_scene();
    } break;
    case input::scan_code::F11: {
        auto const fileName {[]() {
            for (i32 i {0};; ++i) {
                auto const name {std::format("screen{:02}.png", i)};
                if (!io::exists(name)) { return name; }
            }
        }()};
        std::ignore = window().copy_to_image().save(fileName);
    } break;
    case input::scan_code::F10:
        _startRecord = true;
        break;
    case input::scan_code::Q: {
        set_quality(quality_level::Low);
    } break;
    case input::scan_code::K: {
        if (!_raycaster->ScreenEffect) {
            _raycaster->ScreenEffect = grayscaleEffect;
        } else {
            _raycaster->ScreenEffect = nullptr;
        }

    } break;
#endif
    default: break;
    }

    auto const it {std::ranges::find_if(_inputMapping.KeyboardBindings, [&ev](auto const& binding) {
        return binding.second == ev.ScanCode;
    })};

    if (it != _inputMapping.KeyboardBindings.end()) {
        handle_action(it->first);
    }
}

void Plinth::on_controller_axis_motion(input::controller::axis_event const& ev)
{
    if (ev.Axis != input::controller::axis::RightTrigger) { return; }

    constexpr f64 pressThreshold {0.5};
    constexpr f64 releaseThreshold {0.4};

    f64 const value {static_cast<f64>(ev.Value) / static_cast<f64>(std::numeric_limits<decltype(ev.Value)>::max())};

    if (!_controllerFireDown && value >= pressThreshold) {
        _controllerFireDown = true;
        handle_action(game_action::Fire);
    } else if (_controllerFireDown && value <= releaseThreshold) {
        _controllerFireDown = false;
    }
}

void Plinth::on_controller_button_down(input::controller::button_event const& ev)
{
    switch (ev.Button) {
    case input::controller::button::A:    handle_action(game_action::Fire); break;
    case input::controller::button::X:    handle_action(game_action::ToggleWall); break;
    case input::controller::button::Back: handle_action(game_action::ToggleMap); break;
    default:                              break;
    }
}

void Plinth::on_mouse_motion(input::mouse::motion_event const& ev)
{
    _mouseX += ev.RelativeMotion.X;
}

void Plinth::on_mouse_button_down(input::mouse::button_event const& ev)
{
    if (ev.Button == input::mouse::button::Left) {
        handle_action(game_action::Fire);
    }
}

void Plinth::on_text_input(input::keyboard::text_input_event const& ev)
{
    _keyCache += ev.Text;
    while (utf8::length(_keyCache) > 10) {
        _keyCache = utf8::remove(_keyCache, 0);
    }

    if (_keyCache.ends_with("givemap")) {
        _level->mark_all_seen();
        _level->show_message("MAP REVEALED");
        _keyCache.clear();
        _player.Cheater          = true;
        _raycaster->ScreenEffect = cheaterEffect;
    }
}

// PLACEHOLDER END
