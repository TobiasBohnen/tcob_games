// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../_common/Common.hpp" // IWYU pragma: keep

namespace BrickOut {

namespace physics {
    using namespace tcob::physics;
}

struct brick_def {
    enum class color {
        Blue,
        Green,
        Grey,
        Purple,
        Red,
        Yellow
    };

    enum class shape {
        Rectangle,
        Square
    };

    color   Color {};
    shape   Shape {};
    point_f Position {};
    i32     Size {1};
};

struct blue_rect : brick_def {
    blue_rect(point_f pos)
        : brick_def {.Color = color::Blue, .Shape = shape::Rectangle, .Position = pos}
    {
    }
};
struct blue_square : brick_def {
    blue_square(point_f pos)
        : brick_def {.Color = color::Blue, .Shape = shape::Square, .Position = pos}
    {
    }
};

struct green_rect : brick_def {
    green_rect(point_f pos)
        : brick_def {.Color = color::Green, .Shape = shape::Rectangle, .Position = pos}
    {
    }
};
struct green_square : brick_def {
    green_square(point_f pos)
        : brick_def {.Color = color::Green, .Shape = shape::Square, .Position = pos}
    {
    }
};

struct grey_rect : brick_def {
    grey_rect(point_f pos)
        : brick_def {.Color = color::Grey, .Shape = shape::Rectangle, .Position = pos}
    {
    }
};
struct grey_square : brick_def {
    grey_square(point_f pos)
        : brick_def {.Color = color::Grey, .Shape = shape::Square, .Position = pos}
    {
    }
};

struct purple_rect : brick_def {
    purple_rect(point_f pos)
        : brick_def {.Color = color::Purple, .Shape = shape::Rectangle, .Position = pos}
    {
    }
};
struct purple_square : brick_def {
    purple_square(point_f pos)
        : brick_def {.Color = color::Purple, .Shape = shape::Square, .Position = pos}
    {
    }
};

struct red_rect : brick_def {
    red_rect(point_f pos)
        : brick_def {.Color = color::Red, .Shape = shape::Rectangle, .Position = pos}
    {
    }
};
struct red_square : brick_def {
    red_square(point_f pos)
        : brick_def {.Color = color::Red, .Shape = shape::Square, .Position = pos}
    {
    }
};

struct yellow_rect : brick_def {
    yellow_rect(point_f pos)
        : brick_def {.Color = color::Yellow, .Shape = shape::Rectangle, .Position = pos}
    {
    }
};
struct yellow_square : brick_def {
    yellow_square(point_f pos)
        : brick_def {.Color = color::Yellow, .Shape = shape::Square, .Position = pos}
    {
    }
};

}