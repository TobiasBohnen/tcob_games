// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

template <typename T = void>
using callback = std::optional<scripting::function<T>>;

struct callbacks {
    callback<>     OnCollision;
    callback<>     OnDieChange;
    callback<>     OnHoverChange;
    callback<>     OnSetup;
    callback<>     OnTeardown;
    callback<i32>  OnTurnUpdate;
    callback<>     OnTurnFinish;
    callback<bool> CanStartTurn;
    callback<>     OnTurnStart;
};

////////////////////////////////////////////////////////////

struct bg_def {
    string Bitmap;
    static auto constexpr Members()
    {
        return std::tuple {member<&bg_def::Bitmap> {"bitmap"}};
    }
};

////////////////////////////////////////////////////////////

struct tex_def {
    u32                ID {0};
    size_i             Size {size_i::Zero};
    string             Bitmap;
    std::optional<u32> Transparent;
    static auto constexpr Members()
    {
        return std::tuple {member<&tex_def::Size> {"size"}, member<&tex_def::Bitmap> {"bitmap"}, member<&tex_def::Transparent, std::nullopt> {"transparent"}};
    }
};

////////////////////////////////////////////////////////////

class dmd_proxy {
public:
    dmd_proxy(prop<grid<u8>>& dmd);

    void clear();

    void blit(rect_i const& rect, string const& dotStr);

    void print(point_i pos, string_view text, color color);

private:
    prop<grid<u8>>& _dmd;
};

////////////////////////////////////////////////////////////

class sfx_proxy {
public:
    auto pickup_coin(u64 seed) -> audio::sound_wave;
    auto laser_shoot(u64 seed) -> audio::sound_wave;
    auto explosion(u64 seed) -> audio::sound_wave;
    auto powerup(u64 seed) -> audio::sound_wave;
    auto hit_hurt(u64 seed) -> audio::sound_wave;
    auto jump(u64 seed) -> audio::sound_wave;
    auto blip_select(u64 seed) -> audio::sound_wave;
    auto random(u64 seed) -> audio::sound_wave;
};
