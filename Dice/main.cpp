// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

#include <tcob/tcob.hpp>

#if defined(_MSC_VER) && !defined(TCOB_DEBUG)
    #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

class my_game : public tcob::game {
public:
    my_game(char const* /* argv */)
        : tcob::game {{.Name      = "Dice",
                       .LogFile   = "stdout",
                       .FixedStep = milliseconds {1000.f / 100.f}}}
    {
        auto& plt {locate_service<platform>()};
        plt.FrameLimit = static_cast<i32>(plt.displays().begin()->second.DesktopMode.RefreshRate);

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