// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GameInfo.hpp"

namespace solitaire {

game_rng::game_rng(rng::seed_type seed)
    : _seed {seed}
    , _gen {seed}
{
}

auto game_rng::gen() -> rng&
{
    return _gen;
}

auto game_rng::seed() const -> rng::seed_type const&
{
    return _seed;
}

} // namespace solitaire
