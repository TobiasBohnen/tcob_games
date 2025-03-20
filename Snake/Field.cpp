// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Field.hpp"

#include "Snake.hpp"

namespace Snake {

field::field(assets::asset_ptr<gfx::material> const& material, i32 windowHeight)
    : _windowHeight(windowHeight)
    , _map {{{
          {TS_FLOOR, {"metal"}},
          {TS_SNAKE_BODY, {"blockerBody"}},
          {TS_SNAKE_HEAD, {"blockerMad"}},
          {TS_SNAKE_DEAD, {"blockerSad"}},
          {TS_FOOD, {"coinGold"}},
          {TS_STAR, {"star"}},
          {TS_BOMB, {"bomb"}},
      }}}
{
    _map.Material = material;
}

void field::start()
{
    Score = 0;

    point_i const snake {_gridSize.Width / 2, _gridSize.Height / 2};
    _snake.reset(snake);

    _food.reset();
    _star.reset();
    _bomb.reset();

    _map.Grid = {.TileSize = get_tile_size()};

    _map.clear();

    gfx::tilemap_layer backLayer {.Tiles = {_gridSize, TS_FLOOR}};
    _layerBack = _map.add_layer(backLayer);

    gfx::tilemap_layer frontLayer {.Tiles = {_gridSize, TS_NONE}};
    frontLayer.Tiles[snake] = TS_SNAKE_HEAD;
    _layerFront             = _map.add_layer(frontLayer);

    _state = game_state::Running;
}

auto field::state() const -> game_state
{
    return _state;
}

void field::on_update(milliseconds deltaTime)
{
    if (_state != game_state::Running) {
        return;
    }

    _currentTime += deltaTime;
    if (_currentTime >= TICK) {
        tick();
        _currentTime = 0ms;
    }
    _map.update(deltaTime);
}

void field::on_fixed_update(milliseconds deltaTime)
{
}

void field::on_draw_to(gfx::render_target& target)
{
    _map.draw_to(target);
}

auto field::can_draw() const -> bool
{
    return true;
}

void field::on_key_down(input::keyboard::event const& ev)
{
    switch (ev.KeyCode) {
    case input::key_code::RIGHT: _snake.turn(direction::Right); break;
    case input::key_code::LEFT: _snake.turn(direction::Left); break;
    case input::key_code::DOWN: _snake.turn(direction::Down); break;
    case input::key_code::UP: _snake.turn(direction::Up); break;
    default: break;
    }
}

void field::on_controller_button_down(input::controller::button_event const& ev)
{
    switch (ev.Button) {
    case input::controller::button::DPadRight: _snake.turn(direction::Right); break;
    case input::controller::button::DPadLeft: _snake.turn(direction::Left); break;
    case input::controller::button::DPadDown: _snake.turn(direction::Down); break;
    case input::controller::button::DPadUp: _snake.turn(direction::Up); break;
    default: break;
    }
}

auto field::get_tile_size() const -> size_f
{
    f32 const max {std::max<f32>(_gridSize.Width, _gridSize.Height)};
    f32 const size {_windowHeight / max};
    return size_f {size, size};
}

void field::tick()
{
    if (!_snake.move_head()) { // out of bounds
        return;
    }

    // star
    _star.tick();
    _snake.try_eat(_star);

    // bomb
    _bomb.tick();
    if (_snake.try_eat(_bomb)) { // BOOM
        return;
    }

    // food
    _food.tick();
    if (_snake.try_eat(_food)) {
        _snake.eat_food();
    } else {
        _snake.move_body();
    }
}

void field::fail()
{
    _snake.die();

    auto& input {locate_service<input::system>()};
    if (input.InputMode == input::mode::Controller) {
        input.first_controller().rumble(0x7FFF, 0, 500ms);
    }

    _state = game_state::Failure;
}

void field::play_sound(audio::sound_wave const& wave)
{
    _sound = audio::sound_generator {}.create_sound(wave);
    _sound->play();
}

void field::set_tile(point_i pos, gfx::tile_index_t idx)
{
    _map.set_tile_index(_layerFront, pos, idx);
}

auto field::get_random_tile() -> point_i
{
    auto const gridSize {*_map.get_layer_size(_layerBack)};

    point_i retValue {};
    do {
        retValue = {_rng(0, gridSize.Width - 1), _rng(0, gridSize.Height - 1)};
    } while (*_map.get_tile_index(_layerFront, retValue) != TS_NONE);

    return retValue;
}

}
