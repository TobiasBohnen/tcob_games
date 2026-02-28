// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Game.hpp"

#include <utility>

using namespace scripting;

////////////////////////////////////////////////////////////

dice_game::dice_game(init init)
    : gfx::entity {update_mode::Both}
    , _init {std::move(init)}
    , _sprites {_init.Events}
    , _sockets {_init.RealWindowSize / DICE_REF_SIZE}
    , _dice {_init.Group, _diceBatch, _init.RealWindowSize / DICE_REF_SIZE}
    , _engine {engine::init {
          .UIState   = _uiState,
          .Events    = _init.Events,
          .Rng       = _rng,
          .SpriteMgr = _sprites,
          .Sockets   = _sockets}}
{
    // TODO: enforce int scaling for background
    // TODO: 16:10 support
    auto const [w, h] {_init.RealWindowSize};
    rect_f const bgBounds {0, 0, h / 3.0f * 4.0f, h};
    rect_f const uiBounds {bgBounds.width(), 0.0f, w - bgBounds.width(), h};

    _form0 = std::make_unique<game_form>(uiBounds, _init.Group, _uiState, _init.Events);

    _screenTexture->Size      = size_i {VIRTUAL_SCREEN_SIZE};
    _screenTexture->Filtering = gfx::texture::filtering::NearestNeighbor;

    auto& firstPass {_screenMaterial->create_pass()};
    firstPass.Shader  = _init.Group.get<gfx::shader>("CRT");
    firstPass.Texture = _screenTexture;

    gfx::quad q {};
    gfx::geometry::set_color(q, colors::White);
    gfx::geometry::set_position(q, bgBounds);
    gfx::geometry::set_texcoords(q, {.UVRect = gfx::render_texture::UVRect(), .Level = 0});
    _screenRenderer.set_geometry({.Vertices = q, .Indices = gfx::QuadIndicies, .Type = gfx::primitive_type::Triangles}, &_screenMaterial->first_pass());

    // dice
    u32 totalDiceCount {0};
    for (auto const& die : _init.Dice) {
        if (PALETTE_MAP.contains(die.Color)) {
            totalDiceCount += die.Amount;
        }
    }

    u32 idx {0};
    for (auto const& die : _init.Dice) {
        if (!PALETTE_MAP.contains(die.Color)) { continue; }

        std::vector<die_face> vec;
        vec.reserve(die.Values.size());
        for (u8 value : die.Values) {
            vec.emplace_back(value, PALETTE_MAP.at(die.Color));
        }
        if (vec.empty()) { return; }

        for (u32 i {0}; i < die.Amount; ++i) {
            _dice.add(get_die_position(totalDiceCount, idx), _rng, vec[0], vec);
            idx++;
        }
    }

    _init.Events.TurnStart.connect([this]() { _sockets.lock(); });
    _init.Events.TurnFinish.connect([this]() {
        _dice.roll();
        for (usize i {0}; i < _dice.count(); ++i) {
            _dice.move(i, get_die_position(_dice.count(), i));
        }
    });
}

dice_game::~dice_game()
{
    // remove sprites before lua gets destroyed
    _sprites.clear();
}

void dice_game::on_update(milliseconds deltaTime)
{
    _form0->update(deltaTime);

    _dice.update(deltaTime);
    _diceBatch.update(deltaTime);
}

void dice_game::on_fixed_update(milliseconds deltaTime)
{
    if (_engine.update(deltaTime)) {
        _sprites.wrap_and_collide();
    }
    _sprites.update(deltaTime);
}

void dice_game::on_draw_to(gfx::render_target& target, transform const& xform)
{
    _sprites.draw_to(*_screenTexture);
    _screenRenderer.render_to_target(target, transform::Identity);

    _form0->draw_to(target, xform);
    _diceBatch.draw_to(target, xform);
}

auto dice_game::can_draw() const -> bool
{
    return true;
}

void dice_game::on_key_down(input::keyboard::event const& ev)
{
    switch (ev.ScanCode) {
    case input::scan_code::SPACE: _init.Events.TurnStart(); break;
    case input::scan_code::R:     _init.Events.GameRestart(); break;
    case input::scan_code::Q:     _init.Events.GameQuit(); break;
    default:                      break;
    }
}

void dice_game::on_mouse_button_up(input::mouse::button_event const& ev)
{
    switch (ev.Button) {
    case input::mouse::button::Left:
        if (auto* socket {_sockets.try_insert_die(_hoverDie)}) {
            _diceBatch.send_to_back(*_hoverDie->shape());
            _init.Events.DieInsert(socket);
        }
        break;
    case input::mouse::button::Right: break;
    default:                          break;
    }

    if (!_hoverDie) {
        static_cast<input::receiver*>(_form0.get())->on_mouse_button_up(ev);
    }
}

void dice_game::on_mouse_button_down(input::mouse::button_event const& ev)
{
    if (!_hoverDie) {
        static_cast<input::receiver*>(_form0.get())->on_mouse_button_down(ev);
    }
}

void dice_game::on_mouse_motion(input::mouse::motion_event const& ev)
{
    bool const isButtonDown {ev.Mouse->is_button_down(input::mouse::button::Left)};
    auto const mp {point_f {ev.Position}};

    if (!isButtonDown) { _hoverDie = _dice.on_hover(mp); }

    if (isButtonDown && _hoverDie) {
        _sockets.on_drag(_hoverDie);

        if (auto* socket {_sockets.try_remove_die(_hoverDie)}) {
            _init.Events.DieRemove(socket);
        }

        _dice.on_drag(mp, _uiState.DiceArea);

        _init.Events.DieMotion(_hoverDie);
    }

    static_cast<input::receiver*>(_form0.get())->on_mouse_motion(ev);
}

void dice_game::run(string const& file)
{
    _engine.run(file);
    _dice.roll();

    _sprites.wrap_and_update();
}

auto dice_game::get_die_position(usize count, usize idx) const -> point_f
{
    f32 const    scale {_init.RealWindowSize.Width / DICE_REF_SIZE.Width};
    size_f const scaledDiceSize {DICE_SIZE * scale};
    f32 const    colPadding {3.0f * scale};
    f32 const    rowPadding {5.0f * scale};

    rect_f area {_uiState.DiceArea};
    area.Size.Height -= DICE_SIZE.Height;

    f32 const   diceWidth {scaledDiceSize.Width + colPadding};
    f32 const   maxDicePerRow {std::floor((area.width() + colPadding) / diceWidth)};
    usize const dicePerRow {std::max(1u, static_cast<u32>(maxDicePerRow))};

    usize const row {idx / dicePerRow};
    usize const col {idx % dicePerRow};

    usize const diceInCurrentRow {std::min(dicePerRow, count - (row * dicePerRow))};

    f32 const rowWidth {(diceInCurrentRow * scaledDiceSize.Width) + ((diceInCurrentRow - 1) * colPadding)};

    f32 const rowStartX {(area.width() - rowWidth) / 2.0f};

    f32 const x {rowStartX + (col * diceWidth)};
    f32 const y {area.bottom() - (row * (scaledDiceSize.Height + rowPadding))};
    return {area.left() + x, y};
}
