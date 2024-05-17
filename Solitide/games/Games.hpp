// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Piles.hpp"

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
    AllCardsButFourToFoundation,
    ClearTableau
};

////////////////////////////////////////////////////////////

struct game_info {
    std::string Name;
    family      Family {};

    objective Objective {objective::AllCardsToFoundation};

    i32 DeckCount {0};
    i32 Redeals {0};

    std::unordered_set<rank> DeckRanks {rank::Ace, rank::Two, rank::Three, rank::Four, rank::Five, rank::Six, rank::Seven, rank::Eight, rank::Nine, rank::Ten, rank::Jack, rank::Queen, rank::King};
    std::unordered_set<suit> DeckSuits {suit::Clubs, suit::Diamonds, suit::Hearts, suit::Spades};

    bool DisableHints {false}; // TODO: replace with script function 'get_hints'
    bool DisableAutoPlay {false};
};

////////////////////////////////////////////////////////////

struct game_state {
    rng::state_type Seed {};

    i32          Redeals {};
    i32          Score {0};
    i32          Turns {0};
    milliseconds Time {0};
    i32          Undos {0};
    i32          Hints {0};

    void static Serialize(game_state const& v, auto&& s)
    {
        s["Redeals"] = v.Redeals;
        s["Seed"]    = v.Seed;
        s["Turns"]   = v.Turns;
        s["Score"]   = v.Score;
        s["Hints"]   = v.Hints;
        s["Undos"]   = v.Undos;
        s["Time"]    = v.Time.count();
    }

    auto static Deserialize(game_state& v, auto&& s) -> bool
    {
        return s.try_get(v.Redeals, "Redeals")
            && s.try_get(v.Seed, "Seed")
            && s.try_get(v.Turns, "Turns")
            && s.try_get(v.Score, "Score")
            && s.try_get(v.Hints, "Hints")
            && s.try_get(v.Undos, "Undos")
            && s.try_get(v.Time, "Time");
    }
};

////////////////////////////////////////////////////////////

struct game_history {
    isize Won {0};
    isize Lost {0};

    struct entry {
        i64  ID {0};
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

class base_game {
public:
    explicit base_game(game_info info);
    virtual ~base_game() = default;

    std::vector<stock>      Stock;
    std::vector<waste>      Waste;
    std::vector<reserve>    Reserve;
    std::vector<freecell>   FreeCell;
    std::vector<tableau>    Tableau;
    std::vector<foundation> Foundation;

    prop<game_status> Status;
    signal<>          Layout;

    auto state() const -> game_state const&;
    auto info() const -> game_info const&;

    auto piles() const -> std::unordered_map<pile_type, std::vector<pile*>> const&;
    auto rand() -> rng&;
    auto storage() -> data::config::object*;

    void start(std::optional<data::config::object> const& loadObj);
    void save(data::config::object& saveObj);

    void undo();
    auto can_undo() const -> bool;

    void hint();

    auto deal_cards() -> bool;
    void play_cards(pile& from, pile& to, isize startIndex, isize numCards);
    auto auto_play_cards(pile& from) -> bool;
    void collect_all();
    auto virtual can_play(pile const& targetPile, isize targetCardIndex, card const& card, isize numCards) const -> bool;

    auto get_available_hints() const -> std::vector<move> const&;
    auto check_movable(pile const& targetPile, isize idx) const -> bool;

    void update(milliseconds delta);

protected:
    auto virtual do_redeal() -> bool = 0;
    auto virtual do_deal() -> bool   = 0;

    auto virtual before_shuffle(card& card) -> bool         = 0;
    auto virtual on_shuffle(card& card, pile* pile) -> bool = 0;
    void virtual after_shuffle()                            = 0;

    void virtual on_init() = 0;

    void virtual on_drop(pile* pile) = 0;
    void virtual on_end_turn()       = 0;

    auto virtual get_status() const -> game_status;
    auto virtual get_shuffled() -> std::vector<card>;

    void create_piles(auto&& piles, isize size, std::function<void(pile&, i32)> const& func);

    void end_turn(bool deal);

private:
    void calc_hints();

    void new_game();
    auto load(std::optional<data::config::object> const& loadObj) -> bool;

    void init();
    void refresh();
    void clear_piles();

    std::unordered_map<pile_type, std::vector<pile*>>     _piles;
    mutable flat_map<std::pair<pile const*, isize>, bool> _movableCache;
    std::vector<move>                                     _hints;

    game_info  _info;
    game_state _state;

    data::config::object             _saveObj;
    std::stack<data::config::object> _undoStack;
    data::config::object             _storage;

    rng _rand {}; // TODO: custom state
};

////////////////////////////////////////////////////////////

template <typename Table, template <typename> typename Function, isize IndexOffset>
class script_game : public base_game {
public:
    script_game(game_info info, Table table);

    auto can_play(pile const& targetPile, isize targetCardIndex, card const& card, isize numCards) const -> bool override;

    void static CreateWrapper(auto&& script);
    template <typename T>
    void static CreateGlobals(auto&& scene, auto&& script, auto&& globalTable, auto&& makeFunc, string const& ext);

protected:
    auto do_redeal() -> bool override;
    auto do_deal() -> bool override;

    auto before_shuffle(card& card) -> bool override;
    auto on_shuffle(card& card, pile* pile) -> bool override;
    void after_shuffle() override;

    void on_init() override;

    void on_drop(pile* pile) override;
    void on_end_turn() override;

    auto get_status() const -> game_status override;
    auto get_shuffled() -> std::vector<card> override;

private:
    void make_piles(auto&& gameRef);

    struct callbacks {
        std::optional<Function<bool>>              Redeal;
        std::optional<Function<bool>>              Deal;
        std::optional<Function<bool>>              OnBeforeShuffle;
        std::optional<Function<bool>>              OnShuffle;
        std::optional<Function<void>>              OnAfterShuffle;
        std::optional<Function<void>>              OnInit;
        std::optional<Function<void>>              OnDrop;
        std::optional<Function<void>>              OnEndTurn;
        std::optional<Function<bool>>              CanPlay;
        std::optional<Function<game_status>>       GetStatus;
        std::optional<Function<std::vector<card>>> GetShuffled;
    };

    callbacks _callbacks;
    Table     _table;
};

////////////////////////////////////////////////////////////

class lua_script_game : public script_game<scripting::lua::table, scripting::lua::function, -1> {
public:
    lua_script_game(game_info info, scripting::lua::table tab);

    auto static CreateENV(scripting::lua::script& script) -> scripting::lua::table;
    void static CreateAPI(start_scene* scene, scripting::lua::script& script, std::vector<scripting::lua::native_closure_shared_ptr>& funcs);
};

////////////////////////////////////////////////////////////

class squirrel_script_game : public script_game<scripting::squirrel::table, scripting::squirrel::function, 0> {
public:
    squirrel_script_game(game_info info, scripting::squirrel::table tab);

    void static CreateAPI(start_scene* scene, scripting::squirrel::script& script, std::vector<scripting::squirrel::native_closure_shared_ptr>& funcs);
};

}

#include "Games.inl"
