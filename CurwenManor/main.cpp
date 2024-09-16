// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/tcob.hpp>

#include "GameCanvas.hpp"
#include "assets/GameAssets.hpp"
#include "scenes/StartScene.hpp"

#if defined(_MSC_VER) && !defined(TCOB_DEBUG)
    #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

#if defined(__EMSCRIPTEN__)
auto main(void) -> int
{
    tcob::data::config::object config;

    tcob::gfx::video_config video;
    video.FrameLimit           = 100;
    video.FullScreen           = false;
    video.UseDesktopResolution = false;
    video.VSync                = false;
    video.Resolution           = tcob::size_i {640, 576};
    video.RenderSystem         = "OPENGLES30";
    config["video"]            = video;

    tcob::game game {{.Path           = ".",
                      .Name           = "CurwenManor",
                      .LogFile        = "stdout",
                      .ConfigDefaults = config,
                      .WorkerThreads  = 8}};
#else

auto main(int /* argc */, char* argv[]) -> int
{
    tcob::data::config::object config;

    tcob::gfx::video_config video;
    video.FrameLimit           = 1000;
    video.FullScreen           = false;
    video.UseDesktopResolution = false;
    video.VSync                = false;
    video.Resolution           = tcob::size_i {640, 576};
    video.RenderSystem         = "OPENGL45";
    config["video"]            = video;

    tcob::game game {{.Path           = ".",
                      .Name           = "CurwenManor",
                      .LogFile        = "stdout",
                      .ConfigDefaults = config,
                      .WorkerThreads  = 8}};
#endif

    locate_service<gfx::render_system>().get_window().ClearColor = color::FromRGB(0x9bbc0f);

    auto assets {std::make_shared<stn::assets>(game)};
    auto canvas {std::make_shared<stn::canvas>(*assets)};
    game.push_scene(std::make_shared<stn::start_scene>(game, canvas, assets));
    game.start();
    return 0;
}