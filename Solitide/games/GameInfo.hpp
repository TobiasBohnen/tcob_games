// Copyright (c) 2024 Tobias Bohnen
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

enum class family {
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

enum class game_status {
    Initial,
    Running,
    Failure,
    Success
};

////////////////////////////////////////////////////////////

enum class objective {
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

    void static Serialize(game_state const& v, auto&& s)
    {
        s["Redeals"] = v.Redeals;
        s["Turns"]   = v.Turns;
        s["Score"]   = v.Score;
        s["Hints"]   = v.Hints;
        s["Undos"]   = v.Undos;
        s["Time"]    = v.Time.count();
    }

    auto static Deserialize(game_state& v, auto&& s) -> bool
    {
        return s.try_get(v.Redeals, "Redeals")
            && s.try_get(v.Turns, "Turns")
            && s.try_get(v.Score, "Score")
            && s.try_get(v.Hints, "Hints")
            && s.try_get(v.Undos, "Undos")
            && s.try_get(v.Time, "Time");
    }
};

////////////////////////////////////////////////////////////
using rng = random::shuffle<card, random::xoshiro_256_plus_plus>;

class game_rng {
public:
    game_rng(rng::seed_type seed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    auto gen() -> rng&;
    auto seed() const -> rng::seed_type const&;

    void static Serialize(game_rng const& v, auto&& s)
    {
        s["Seed"]  = v._seed;
        s["State"] = v._gen.get_state();
    }

    auto static Deserialize(game_rng& v, auto&& s) -> bool
    {
        rng::state_type state;
        if (s.try_get(v._seed, "Seed")
            && s.try_get(state, "State")) {
            v._gen = rng {state};
            return true;
        }

        return false;
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

    prop<std::string>          SelectedGame;
    prop<game_history>         SelectedHistory;
    prop<data::config::object> SelectedRules;

    theme_map    Themes;
    card_set_map CardSets;

    translator Translator;
};

}
