// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <tcob/tcob.hpp>

using namespace tcob;
using namespace tcob::gfx::ui;
using namespace std::chrono_literals;

enum class game_state {
    Initial,
    Running,
    Failure,
    Success
};
