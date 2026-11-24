// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ScreenEffect.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <numeric>
#include <optional>
#include <span>
#include <utility>
#include <vector>

#include "Common.hpp"

////////////////////////////////////////////////////////////

screen_effect::screen_effect(std::optional<milliseconds> duration)
    : _duration {duration}
{
}

void screen_effect::update(milliseconds deltaTime)
{
    _elapsed += deltaTime;
    if (_duration) { _elapsed = std::min(_elapsed, *_duration); }
}

void screen_effect::reset() { _elapsed = {}; }

auto screen_effect::elapsed() const -> milliseconds { return _elapsed; }
auto screen_effect::finished() const -> bool { return _duration && _elapsed >= *_duration; }

auto screen_effect::progress() const -> f64
{
    if (!_duration || _duration->count() <= 0) { return 0.0; }
    return std::min(static_cast<f64>(_elapsed.count()) / static_cast<f64>(_duration->count()), 1.0);
}

auto screen_effect::Pack(u8 r, u8 g, u8 b) -> u32
{
    return 0xFF000000u | (static_cast<u32>(b) << 16) | (static_cast<u32>(g) << 8) | static_cast<u32>(r);
}

////////////////////////////////////////////////////////////

reveal_effect::reveal_effect(std::vector<usize> order, u32 abgr, milliseconds duration)
    : screen_effect {duration}
    , _order {std::move(order)}
    , _abgr {abgr}
{
}

void reveal_effect::operator()(std::span<u32> screen) const
{
    if (screen.size() != _order.size()) { return; } // built for another resolution

    usize const gone {static_cast<usize>(progress() * static_cast<f64>(_order.size()))};
    for (usize i {0}; i < gone; ++i) { screen[_order[i]] = _abgr; }
}

auto MakeDeathEffect(size_i size, milliseconds duration) -> std::unique_ptr<screen_effect>
{
    std::vector<usize> order(static_cast<usize>(size.area()));
    std::ranges::iota(order, 0);
    random::shuffle {}(order);

    return std::make_unique<reveal_effect>(std::move(order), 0xFF0000FFu, duration);
}

auto MakeDissolveEffect(size_i size, milliseconds duration, f64 edge) -> std::unique_ptr<screen_effect>
{
    auto const hash {[](u32 x, u32 y) -> f64 {
        u32 h {(x * 73856093u) ^ (y * 19349663u)};
        h ^= h >> 16;
        h *= 0x7feb352du;
        h ^= h >> 15;
        h *= 0x846ca68bu;
        h ^= h >> 16;
        return static_cast<f64>(h) / 4294967296.0;
    }};

    f64 const cx {size.Width * 0.5};
    f64 const cy {size.Height * 0.5};
    f64 const rim {std::sqrt((cx * cx) + (cy * cy)) * edge};

    // when each pixel goes: its distance from the center plus some noise
    std::vector<f64> keys(static_cast<usize>(size.area()));
    for (i32 y {0}; y < size.Height; ++y) {
        for (i32 x {0}; x < size.Width; ++x) {
            f64 const dx {x + 0.5 - cx};
            f64 const dy {y + 0.5 - cy};
            keys[PixelIndex(size.Width, x, y)] = std::sqrt((dx * dx) + (dy * dy)) + (hash(static_cast<u32>(x), static_cast<u32>(y)) * rim);
        }
    }

    std::vector<usize> order(keys.size());
    std::ranges::iota(order, 0);
    std::ranges::sort(order, [&](usize a, usize b) { return keys[a] < keys[b]; });

    return std::make_unique<reveal_effect>(std::move(order), 0xFF000000u, duration);
}

////////////////////////////////////////////////////////////

void cheater_effect::operator()(std::span<u32> screen) const
{
    for (auto& col : screen) {
        color c {color::FromABGR(col)};
        std::swap(c.R, c.B);
        col = Pack(c.R, c.G, c.B);
    }
}

void night_vision_effect::operator()(std::span<u32> screen) const
{
    for (auto& col : screen) {
        color const c {color::FromABGR(col)};
        u32 const   luminance {static_cast<u32>((c.R * 0.3f) + (c.G * 0.59f) + (c.B * 0.11f))};
        col = Pack(0, static_cast<u8>(std::min(255u, luminance + 50)), 0);
    }
}

void static_noise_effect::operator()(std::span<u32> screen) const
{
    rng randomGenerator;
    for (auto& col : screen) {
        if (randomGenerator(0, 12) == 0) {
            u8 const noise {static_cast<u8>(randomGenerator(0, 255))};
            col = Pack(noise, noise, noise);
        }
    }
}

void grayscale_effect::operator()(std::span<u32> screen) const
{
    for (auto& col : screen) {
        color const c {color::FromABGR(col)};
        u8 const    gray {static_cast<u8>((c.R * 0.299f) + (c.G * 0.587f) + (c.B * 0.114f))};
        col = Pack(gray, gray, gray);
    }
}
