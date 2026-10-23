// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include "Level.hpp"

class screen_overlay {
public:
    explicit screen_overlay(texture_cache& cache);

    std::function<void(std::span<u32>)> ScreenEffect;

    auto draw(level& level, player const& player, std::span<u32> screen, size_i screenSize) -> std::span<u32>
    {
        _screen     = screen;
        _screenSize = screenSize;

        draw_weapon(player);

        draw_screen_effect();

        draw_hud(player);
        draw_message(level);

        return _screen;
    }

private:
    void draw_screen_effect();
    void draw_weapon(player const& player);
    void draw_hud(player const& player);
    void draw_message(level const& level);

    std::span<u32> _screen;
    size_i         _screenSize;

    texture_cache& _cache;

    task_manager& _taskManager;
};
