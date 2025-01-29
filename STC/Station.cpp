#include "Station.hpp"

#include <utility>

////////////////////////////////////////////////////////////

station::station(def const& def, std::shared_ptr<physics::body> body, std::shared_ptr<gfx::rect_shape> shape)
    : object {std::move(body), std::move(shape)}
{
}
