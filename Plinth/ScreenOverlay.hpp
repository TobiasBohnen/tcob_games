// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include <functional>
#include <span>

class screen_overlay {
public:
    explicit screen_overlay(texture_cache& cache);

    std::function<void(std::span<u32>)> ScreenEffect;

    void update(milliseconds deltaTime);

    auto draw(level const& level, player const& player, std::span<u32> screen, size_i screenSize) -> std::span<u32>;

    void show_message(string const& msg);

private:
    void draw_screen_effect() const;

    void draw_target(level const& level) const;
    void draw_weapon(player const& player) const;
    void draw_compass(player const& player) const;
    void draw_message() const;

    void draw_text(string_view text, point_i pos, i32 textScale, bool shadow) const;

    std::span<u32> _screen;
    size_i         _screenSize;

    texture_cache& _cache;

    milliseconds _messageTimer {};
    string       _message;
};
