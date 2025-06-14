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
    my_game()
        : tcob::game {{.Name = "Solitide"}}
    {
        start();
    }

    void on_start() override
    {
        push_scene<solitaire::start_scene>();
    }
};

auto main(int /* argc */, char*[] /* argv*/) -> int
{
    tcob::single_instance inst {"Solitide"};
    if (inst) { my_game m {}; }
    return 0;
}
