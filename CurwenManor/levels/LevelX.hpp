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

    void update(milliseconds deltaTime) override;

    void draw(canvas& canvas) override;

private:
    milliseconds _currentTime {0};
};

}
