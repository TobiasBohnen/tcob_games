// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Snake.hpp"

#include <deque>

namespace Snake {

class field;

class field_item {
public:
    explicit field_item(field& parent);

    virtual ~field_item() = default;

    void reset();

    auto is_hidden() const -> bool;

    virtual auto eat_check(point_i pos) -> bool = 0;
    virtual void tick()                         = 0;

protected:
    auto is_pos(point_i pos) const -> bool;
    void hide();

    auto rand(i32 max) -> bool;

    void spawn_at_random(gfx::tile_index_t idx);
    void give_score(i32 score);
    void play_sound(audio::sound_wave const& wave);
    void fail();

private:
    field&  _parent;
    rng     _rng {};
    point_i _pos {HIDDEN};
};

class food : public field_item {
public:
    using field_item::field_item;

    auto eat_check(point_i pos) -> bool override;
    void tick() override;
};

class star : public field_item {
public:
    using field_item::field_item;

    auto eat_check(point_i pos) -> bool override;
    void tick() override;
};

class bomb : public field_item {
public:
    using field_item::field_item;

    auto eat_check(point_i pos) -> bool override;
    void tick() override;
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class snake {
public:
    explicit snake(field& parent, size_i size);

    void reset(point_i pos);

    void turn(direction dir);
    auto try_move_head() -> bool;
    void move_body();
    void eat_food();

    void die();

    auto try_eat(field_item& item) const -> bool;

private:
    auto enforce_bounds() const -> bool;

    field& _parent;
    size_i _gridSize;

    direction           _dir {};
    point_i             _headPos {};
    point_i             _oldHeadPos {};
    std::deque<point_i> _tail;
};

}