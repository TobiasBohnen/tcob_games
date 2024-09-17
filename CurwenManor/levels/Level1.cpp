// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Level1.hpp"

#include "../scenes/GameScene.hpp"

namespace stn {

level1::level1(game_scene* parent)
    : base_level {parent}
{
    auto& player {parent->get_player()};
    player.reset_turn();
    player.set_position({3, 3});
    player.set_direction(direction::Down, 0);

    auto& map {parent->get_map()};

    lighting l;
    l.Off = true;
    map.set_lighting(l);

    map.make_basic_layout("09", "04");

    std::vector<std::vector<std::string>> room0 {
        {"48", "44", "44", "44", "49"},
        {"46", "14", "14", "14", "45"},
        {"46", "14", "14", "14", "21"},
        {"50", "47", "47", "47", "51"},
    };
    map.set_tiles(room0, {1, 1});
    std::vector<std::vector<std::string>> room1 {
        {"48", "44", "44", "44", "49"},
        {"46", "05", "05", "05", "45"},
        {"21", "05", "05", "05", "45"},
        {"50", "47", "47", "47", "51"},
    };
    map.set_tiles(room1, {7, 1});

    on_event_finished(0);
}

void level1::on_event_finished(i32 numEvents)
{
    switch (numEvents) {
    case 0:
        set_event([](auto& scene, i32 step) { return step < 5 ? level_event_status::Visible : level_event_status::Finished; },
                  [&](auto& canvas, i32 step) { draw_text(canvas, 1, step / 2); });
        break;
    case 1:
        set_event(
            [&](auto& scene, i32 step) {
                if (step > 4) { return level_event_status::Finished; }
                if (step == 0 && get_parent().get_player().get_turn() > 5 && _rand(0, 2) == 2) {
                    auto& map {get_parent().get_map()};

                    lighting l;
                    l.Falloff                       = {25, 50, 60};
                    l.VisionCones[direction::Left]  = {190, 350};
                    l.VisionCones[direction::Right] = {10, 170};
                    l.VisionCones[direction::Up]    = {280, 80};
                    l.VisionCones[direction::Down]  = {100, 260};

                    map.set_lighting(l);
                    return level_event_status::Visible;
                }
                return step > 0 ? level_event_status::Visible : level_event_status::Hidden;
            },
            [&](auto& canvas, i32) {
                draw_text(canvas, 2, 0);
            });
        break;
    case 2:
        set_event(
            [&](auto& scene, i32 step) {
                if (step >= 4) { return level_event_status::Finished; }
                if (_rand(0, 4) == 4) {
                    scene.get_player().take_sanity_damage();
                    return level_event_status::Visible;
                }
                return level_event_status::Hidden;
            },
            [&](auto& canvas, i32 step) {
                draw_text(canvas, 3, step);
            });
        break;
    }
}
}
