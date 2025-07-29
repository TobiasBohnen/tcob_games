#include "FieldItems.hpp"

#include <algorithm>

#include "Field.hpp"

namespace Snake {

////////////////////////////////////////////////////////////

field_item::field_item(field& parent)
    : _parent {parent}
{
}

void field_item::reset()
{
    _pos = HIDDEN;
}

auto field_item::rand(i32 max) -> bool
{
    return _rng(0, max) == 0;
}

void field_item::spawn_at_random(gfx::tile_index_t idx)
{
    _pos = _parent.get_random_tile();
    _parent.set_tile(_pos, idx);
}

void field_item::give_score(i32 score)
{
    _parent.Score += score;
}

void field_item::play_sound(audio::sound_wave const& wave)
{
    _parent.play_sound(wave);
}

void field_item::fail()
{
    _parent.fail();
}

auto field_item::is_hidden() const -> bool
{
    return _pos == HIDDEN;
}

auto field_item::is_pos(point_i pos) const -> bool
{
    return _pos == pos;
}

void field_item::hide()
{
    if (_pos != HIDDEN) {
        _parent.set_tile(_pos, TS_NONE);
        _pos = HIDDEN;
    }
}

////////////////////////////////////////////////////////////

constexpr i32 STAR_CHANCE {50};
constexpr i32 BOMB_CHANCE {20};
constexpr i32 STAR_HIDE_CHANCE {15};
constexpr i32 BOMB_HIDE_CHANCE {15};
constexpr i32 STAR_SCORE {1000};
constexpr i32 FOOD_SCORE {100};

auto star::eat_check(point_i pos) -> bool
{
    if (is_pos(pos)) { // eat star
        give_score(STAR_SCORE);
        play_sound(audio::sound_generator {}.generate_powerup());
        hide();
        return true;
    }

    return false;
}

void star::tick()
{
    if (is_hidden()) {
        if (rand(STAR_CHANCE)) {
            spawn_at_random(TS_STAR);
        }
    } else if (rand(STAR_HIDE_CHANCE)) {
        hide();
    }
}

auto bomb::eat_check(point_i pos) -> bool
{
    if (is_pos(pos)) { // eat bomb
        fail();
        play_sound(audio::sound_generator {}.generate_explosion());
        return true;
    }

    return false;
}

void bomb::tick()
{
    if (is_hidden()) {
        if (rand(BOMB_CHANCE)) {
            spawn_at_random(TS_BOMB);
        }
    } else if (rand(BOMB_HIDE_CHANCE)) { // hide bomb
        hide();
    }
}

auto food::eat_check(point_i pos) -> bool
{
    if (is_pos(pos)) { // eat food
        give_score(FOOD_SCORE);
        hide();
        play_sound(audio::sound_generator {}.generate_pickup_coin());
        return true;
    }

    return false;
}

void food::tick()
{
    if (is_hidden()) {
        spawn_at_random(TS_FOOD);
    }
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

snake::snake(field& parent, size_i size)
    : _parent {parent}
    , _gridSize {size}
{
}

void snake::reset(point_i pos)
{
    _dir        = direction::Left;
    _headPos    = pos;
    _oldHeadPos = HIDDEN;
    _tail.clear();
}

void snake::turn(direction dir)
{
    _dir = dir;
}

auto snake::check_bounds() const -> bool
{
    if (!_gridSize.contains(_headPos)) {
        _parent.fail();
        _parent.play_sound(audio::sound_generator {}.generate_hit_hurt());
        return false;
    }

    return true;
}

auto snake::move_head() -> bool
{
    _oldHeadPos = _headPos;

    switch (_dir) {
    case direction::Left:  _headPos = {_oldHeadPos.X - 1, _oldHeadPos.Y}; break;
    case direction::Right: _headPos = {_oldHeadPos.X + 1, _oldHeadPos.Y}; break;
    case direction::Up:    _headPos = {_oldHeadPos.X, _oldHeadPos.Y - 1}; break;
    case direction::Down:  _headPos = {_oldHeadPos.X, _oldHeadPos.Y + 1}; break;
    case direction::None:  break;
    }

    return check_bounds();
}

void snake::move_body()
{
    _parent.set_tile(_oldHeadPos, TS_NONE);

    if (!_tail.empty()) { // draw body
        // check hit tail
        if (std::ranges::find(_tail, _headPos) != _tail.end()) {
            _parent.fail();
            _parent.play_sound(audio::sound_generator {}.generate_hit_hurt());
            return;
        }

        _parent.set_tile(_tail.back(), TS_NONE);
        _tail.pop_back();
        _parent.set_tile(_oldHeadPos, TS_SNAKE_BODY);
        _tail.push_front(_oldHeadPos);
    }

    // draw head
    _parent.set_tile(_headPos, TS_SNAKE_HEAD);
}

void snake::eat_food()
{
    _parent.set_tile(_oldHeadPos, TS_SNAKE_BODY);
    _tail.push_front(_oldHeadPos);

    // draw head
    _parent.set_tile(_headPos, TS_SNAKE_HEAD);
}

void snake::die()
{
    _parent.set_tile(_oldHeadPos, TS_SNAKE_DEAD);
}

auto snake::try_eat(field_item& item) const -> bool
{
    return item.eat_check(_headPos);
}

////////////////////////////////////////////////////////////

}
