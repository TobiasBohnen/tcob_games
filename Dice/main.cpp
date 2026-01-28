// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

#include <tcob/tcob.hpp>

#if defined(_MSC_VER) && !defined(TCOB_DEBUG)
    #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

#if defined(__EMSCRIPTEN__)

auto get_config() -> tcob::data::object
{
    tcob::data::object config;

    tcob::gfx::video_config video;
    video.FrameLimit           = 60;
    video.FullScreen           = false;
    video.UseDesktopResolution = false;
    video.VSync                = true;
    video.Resolution           = tcob::size_i {800, 450};
    video.RenderSystem         = "OPENGLES30";
    config["video"]            = video;
    return config;
}

class my_game : public tcob::game {
public:
    my_game(char const* /* argv */)
        : tcob::game {
              {.Name           = "Dice",
               .LogFile        = "stdout",
               .ConfigDefaults = get_config(),
               .WorkerThreads  = 8,
               .FixedStep      = milliseconds {1000.f / 100.f}}}
    {
#else
class my_game : public tcob::game {
public:
    my_game(char const* /* argv */)
        : tcob::game {{.Name = "Dice",
    #if defined(TCOB_DEBUG)
                       .LogFile = "stdout",
    #else
                       .LogFile = "dice.log",
    #endif
                       .FixedStep = milliseconds {1000.f / 100.f}}}
    {
#endif

#if !defined(__EMSCRIPTEN__)
        auto& plt {locate_service<platform>()};
        plt.FrameLimit = static_cast<i32>(plt.displays().begin()->second.DesktopMode.RefreshRate);
#endif
        auto& rs {locate_service<gfx::render_system>()};
        rs.window().VSync = false;

        start();
    }

    void on_start() override
    {
        push_scene<start_scene>();
    }
};

auto main(int /* argc */, char* argv[]) -> int
{
    my_game m {argv[0]};
    return 0;
}