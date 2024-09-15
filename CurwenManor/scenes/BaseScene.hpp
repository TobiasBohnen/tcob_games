// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp" // IWYU pragma: keep
#include "../GameCanvas.hpp"
#include "../assets/GameAssets.hpp"

namespace stn {

/////////////////////////////////////////////////////

class base_scene : public scene {
public:
    base_scene(game& game, std::shared_ptr<canvas> canvas, std::shared_ptr<assets> assets);

protected:
    auto get_canvas() -> canvas&;
    auto get_assets() -> assets&;
    auto is_fading() const -> bool;

    void on_draw_to(gfx::render_target& target) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event& ev) override;

    template <typename T>
    void push_scene(auto&&... args);

    void fade_out(bool toBlack, auto&& func);

private:
    std::shared_ptr<canvas> _canvas;
    std::shared_ptr<assets> _assets;

    std::unique_ptr<linear_tween<i32>> _fadeOut;
};

template <typename T>
inline void base_scene::push_scene(auto&&... args)
{
    get_game().push_scene(std::make_shared<T>(std::move(args)..., get_game(), _canvas, _assets));
    _canvas->request_draw();
}

inline void base_scene::fade_out(bool toBlack, auto&& func)
{
    if (_fadeOut) { return; }

    _fadeOut = make_unique_tween<linear_tween<i32>>(1000ms, 1, 4);
    _fadeOut->Value.Changed.connect([&, toBlack] {
        if (toBlack) {
            _canvas->fade_to_black();
        } else {
            _canvas->fade_to_white();
        }
    });
    _fadeOut->Finished.connect([&, func] {
        get_game().pop_current_scene();
        func();
    });
    _fadeOut->start();
}

}
