// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GameScene.hpp"

#include "CutScene.hpp"

#include <utility>

namespace stn {

game_scene::game_scene(game& game, std::shared_ptr<canvas> canvas, std::shared_ptr<assets> assets)
    : base_scene {game, std::move(canvas), std::move(assets)}
{
}

game_scene::~game_scene()
{
}

void game_scene::on_start()
{
    push_scene<cut_scene>(0);
}

void game_scene::on_wake_up()
{
    auto& canvas {get_canvas()};
    canvas.Draw.connect([&](gfx::canvas& canvas) { on_canvas_draw(canvas); });
    canvas.request_draw();
}

void game_scene::on_controller_button_down(input::controller::button_event&)
{
}

void game_scene::on_controller_button_up(input::controller::button_event&)
{
}

void game_scene::on_canvas_draw(gfx::canvas& canvas)
{
    canvas.set_fill_style(colors::White);
    point_f p {};
    for (i32 i {0}; i < 90; ++i) {
        canvas.draw_image(get_assets().get_texture("tiles0"), "tile0", {p, TILE_SIZE_F});
        p.X += TILE_SIZE_F.Width;
        if (p.X >= CANVAS_SIZE_F.Width) {
            p.X = 0;
            p.Y += TILE_SIZE_F.Height;
        }
    }
}

}
