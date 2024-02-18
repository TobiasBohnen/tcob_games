// Copyright (c) 2023 Tobias Bohnen
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
    my_game(char const* argv)
        : tcob::game {{.Path = argv, .Name = "GAME_NAME"}}
    {
        start();
    }

    void on_start() override
    {
        push_scene<GAME_NAME::start_scene>();
    }
};

auto main(int argc, char* argv[]) -> int
{
    my_game m {argv[0]};
    return 0;
}