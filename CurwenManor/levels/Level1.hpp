// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp" // IWYU pragma: keep
#include "BaseLevel.hpp"

namespace stn {

/////////////////////////////////////////////////////

class level1 : public base_level {
public:
    level1(game_scene* parent);

protected:
    void on_event_finished(i32 numEvents) override;

private:
    rng _rand {};
};

}
