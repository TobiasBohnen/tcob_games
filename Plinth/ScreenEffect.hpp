// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <span>
#include <vector>

#include "Common.hpp"

////////////////////////////////////////////////////////////

class screen_effect {
public:
    explicit screen_effect(std::optional<milliseconds> duration = std::nullopt);
    virtual ~screen_effect() = default;

    void update(milliseconds deltaTime);
    void reset();

    auto elapsed() const -> milliseconds;
    auto finished() const -> bool;

    auto progress() const -> f64;

    virtual void operator()(std::span<u32> screen) const = 0;

private:
    std::optional<milliseconds> _duration;
    milliseconds                _elapsed {};
};

////////////////////////////////////////////////////////////

// pixels are painted in a fixed order, more of them as time goes by
class reveal_effect final : public screen_effect {
public:
    reveal_effect(std::vector<usize> order, u32 abgr, milliseconds duration);

    void operator()(std::span<u32> screen) const override;

private:
    std::vector<usize> _order;
    u32                _abgr;
};

// red pixels everywhere, in random order
auto MakeDeathEffect(size_i size, milliseconds duration = 1500ms) -> std::unique_ptr<screen_effect>;

// black from the center outwards; the rim is noisy, so it crumbles instead of cutting a hard circle
auto MakeDissolveEffect(size_i size, milliseconds duration = 1500ms, f64 edge = 0.25) -> std::unique_ptr<screen_effect>;

////////////////////////////////////////////////////////////

class cheater_effect final : public screen_effect {
public:
    void operator()(std::span<u32> screen) const override;
};

class night_vision_effect final : public screen_effect {
public:
    void operator()(std::span<u32> screen) const override;
};

class static_noise_effect final : public screen_effect {
public:
    void operator()(std::span<u32> screen) const override;
};

class grayscale_effect final : public screen_effect {
public:
    void operator()(std::span<u32> screen) const override;
};
