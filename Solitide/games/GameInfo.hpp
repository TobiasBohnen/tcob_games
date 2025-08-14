// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include <unordered_set>

#include "Cards.hpp"
#include "Translator.hpp"

namespace solitaire {

////////////////////////////////////////////////////////////

enum class family : u8 {
    Other,
    BakersDozen,
    BeleagueredCastle,
    Canfield,
    Fan,
    FlowerGarden,
    FortyThieves,
    FreeCell,
    Golf,
    Gypsy,
    Klondike,
    Montana,
    Numerica,
    Pairing,
    PictureGallery,
    Spider,
    Terrace,
    Yukon
};

////////////////////////////////////////////////////////////

enum class game_status : u8 {
    Initial,
    Running,
    Failure,
    Success
};

////////////////////////////////////////////////////////////

enum class objective : u8 {
    AllCardsToFoundation,
    AllCardsButOneToFoundation,
    AllCardsButTwoToFoundation,
    AllCardsButFourToFoundation,
    AllCardsToTableau,
    ClearTableau
};

////////////////////////////////////////////////////////////

struct game_info {
    std::string                Name;
    std::unordered_set<family> Family;

    objective Objective {objective::AllCardsToFoundation};

    i32 DeckCount {0};
    i32 Redeals {0};

    std::unordered_set<rank> DeckRanks {};
    std::unordered_set<suit> DeckSuits {};

    bool DisableHints {false}; // TODO: replace with script function 'get_hints'
    bool DisableAutoPlay {false};
};

////////////////////////////////////////////////////////////

struct game_state {
    i32          Redeals {};
    i32          Score {0};
    i32          Turns {0};
    milliseconds Time {0};
    i32          Undos {0};
    i32          Hints {0};

    auto static constexpr Members()
    {
        return std::tuple {
            member<&game_state::Redeals> {"Redeals"},
            member<&game_state::Turns> {"Turns"},
            member<&game_state::Score> {"Score"},
            member<&game_state::Hints> {"Hints"},
            member<&game_state::Undos> {"Undos"},
            member<&game_state::Time> {"Time"}};
    }
};

////////////////////////////////////////////////////////////
using rng = random::shuffle<card, random::xoshiro_256_plus_plus>;

inline auto get_time_seed() -> rng::seed_type
{
    u64 const                seed {static_cast<u64>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())};
    random::rng_split_mix_64 rng {seed};
    return static_cast<rng::seed_type>(rng.next());
}

class game_rng {
public:
    game_rng(rng::seed_type seed);

    auto gen() -> rng&;
    auto seed() const -> rng::seed_type const&;

    auto static constexpr Members()
    {
        return std::tuple {
            member<&game_rng::_seed> {"Seed"},
            computed_member<
                [](auto&& val) { return val._gen.state(); },
                [](auto&& val, auto&& state) { val._gen = rng {state}; }> {"State"}};
    }

private:
    rng::seed_type _seed {};
    rng            _gen {};
};

////////////////////////////////////////////////////////////

struct game_history {
    isize Won {0};
    isize Lost {0};

    struct entry {
        i64  ID {0};
        u64  Seed {0};
        i64  Turns {0};
        i64  Score {0};
        i64  Undos {0};
        i64  Hints {0};
        i64  Time {0};
        bool Won {false};
    };
    std::vector<entry> Entries;
};

////////////////////////////////////////////////////////////

struct game_rule {
    std::string Build;
    std::string Move;
    std::string Base;

    auto operator==(game_rule const& right) const -> bool = default;
};

////////////////////////////////////////////////////////////

struct menu_sources {
    settings Settings;

    game_map Games;
    signal<> GameAdded; // wizard

    prop<std::string>  SelectedGame;
    prop<game_history> SelectedHistory;
    prop<data::object> SelectedRules;

    theme_map    Themes;
    card_set_map CardSets;

    translator Translator;
};

}

struct pair_hash {
    template <class T1, class T2>
    auto operator()(std::pair<T1, T2> const& p) const -> std::size_t
    {
        return std::hash<T1> {}(p.first) ^ std::hash<T2> {}(p.second);
    }
};

namespace std {
template <>
struct hash<solitaire::game_rule> {
    auto operator()(solitaire::game_rule const& rule) const -> std::size_t
    {
        std::size_t h1 = std::hash<std::string> {}(rule.Build);
        std::size_t h2 = std::hash<std::string> {}(rule.Move);
        std::size_t h3 = std::hash<std::string> {}(rule.Base);
        return h1 ^ (h2 << 1) ^ (h3 << 1);
    }
};
}
