// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

#include <tcob/tcob.hpp>

#if defined(_MSC_VER) && !defined(TCOB_DEBUG)
    #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

#include "games/spider/Spider.hpp"

class my_game : public tcob::game {
public:
    my_game(char const* argv)
        : tcob::game {{.Path = argv, .Name = "Solitaire"}}
    {
        //  get_window().ClearColor = colors::DarkGreen;
        start();
    }

    template <typename... Ts>
    void register_games(solitaire::start_scene& scene)
    {
        (scene.register_game(Ts::Info, [](auto& field) { return std::make_shared<Ts>(field); }), ...);
    }

    void on_start() override
    {
        using namespace solitaire::games;
        auto scene {std::make_shared<solitaire::start_scene>(*this)};
        register_games<
            // Spider
            spider, arachnida>

            (*scene);
        push_scene(scene);
    }
};

auto main(int /* argc */, char* argv[]) -> int
{
    my_game m {argv[0]};
    return 0;
}
