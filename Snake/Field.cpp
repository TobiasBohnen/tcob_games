// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Field.hpp"

#include "Snake.hpp"

namespace Snake {

field::field(asset_ptr<gfx::material> const& material, i32 windowHeight)
    : _windowHeight(windowHeight)
    , _map {gfx::orthogonal_tilemap::set {
          {TS_FLOOR, {.TextureRegion = "metal"}},
          {TS_SNAKE_BODY, {.TextureRegion = "blockerBody"}},
          {TS_SNAKE_HEAD, {.TextureRegion = "blockerMad"}},
          {TS_SNAKE_DEAD, {.TextureRegion = "blockerSad"}},
          {TS_FOOD, {.TextureRegion = "coinGold"}},
          {TS_STAR, {.TextureRegion = "star"}},
          {TS_BOMB, {.TextureRegion = "bomb"}},
      }}
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

    _layerBack        = &_map.create_layer();
    _layerBack->Tiles = grid<gfx::tile_index_t> {_gridSize, TS_FLOOR};

    auto frontTiles {grid<gfx::tile_index_t> {_gridSize, TS_NONE}};
    frontTiles[snake]  = TS_SNAKE_HEAD;
    _layerFront        = &_map.create_layer();
    _layerFront->Tiles = frontTiles;

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
    case input::key_code::LEFT:  _snake.turn(direction::Left); break;
    case input::key_code::DOWN:  _snake.turn(direction::Down); break;
    case input::key_code::UP:    _snake.turn(direction::Up); break;
    default:                     break;
    }
}

void field::on_controller_button_down(input::controller::button_event const& ev)
{
    switch (ev.Button) {
    case input::controller::button::DPadRight: _snake.turn(direction::Right); break;
    case input::controller::button::DPadLeft:  _snake.turn(direction::Left); break;
    case input::controller::button::DPadDown:  _snake.turn(direction::Down); break;
    case input::controller::button::DPadUp:    _snake.turn(direction::Up); break;
    default:                                   break;
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
    if (!_snake.try_move_head()) { // out of bounds
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
    _sound = std::make_shared<audio::sound>(audio::sound_generator {}.create_buffer(wave));
    _sound->play();
}

void field::set_tile(point_i pos, gfx::tile_index_t idx)
{
    _layerFront->Tiles.mutate([&](auto& tiles) {
        tiles[pos] = idx;
    });
}

auto field::get_random_tile() -> point_i
{
    point_i retValue {};
    do {
        retValue = {_rng(0, _gridSize.Width - 1), _rng(0, _gridSize.Height - 1)};
    } while (_layerFront->Tiles[retValue] != TS_NONE);

    return retValue;
}

}
