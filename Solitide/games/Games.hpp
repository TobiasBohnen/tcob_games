// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "CardTable.hpp"
#include "Piles.hpp"

namespace solitaire::games {
////////////////////////////////////////////////////////////

enum class family {
    Other,
    BakersDozen,
    BeleagueredCastle,
    Canfield,
    Fan,
    FortyThieves,
    FreeCell,
    Golf,
    Gypsy,
    Klondike,
    Montana,
    Raglan,
    Spider,
    Yukon
};

////////////////////////////////////////////////////////////

struct game_info {
    std::string Name;
    family      Family {};

    i32 DeckCount {0};
    i32 CardDealCount {0};
    i32 Redeals {0};

    std::unordered_set<rank> DeckRanks {rank::Ace, rank::Two, rank::Three, rank::Four, rank::Five, rank::Six, rank::Seven, rank::Eight, rank::Nine, rank::Ten, rank::Jack, rank::Queen, rank::King};
    std::unordered_set<suit> DeckSuits {suit::Clubs, suit::Diamonds, suit::Hearts, suit::Spades};

    // TODO: replace with script function 'get_hints'
    bool DisableHints {false};

    // load/save
    std::string        InitialSeed;
    i32                RemainingRedeals {};
    i32                Turn {0};
    tcob::milliseconds Time {0};
};

////////////////////////////////////////////////////////////

struct move {
    pile* Src {nullptr};
    isize SrcIdx {0};
    isize SrcCardIdx {0};

    pile* Dst {nullptr};
    isize DstIdx {0};
    isize DstCardIdx {0};

    bool HasFoundation {false};
    bool HasFreeCell {false};
};

class base_game {
public:
    using rng = random::rng_xoshiro_256_plus_plus;

    base_game(card_table& f, game_info info);
    virtual ~base_game() = default;

    std::vector<stock>      Stock;
    std::vector<waste>      Waste;
    std::vector<reserve>    Reserve;
    std::vector<freecell>   FreeCell;
    std::vector<tableau>    Tableau;
    std::vector<foundation> Foundation;

    prop<game_state> State;

    auto get_name() const -> std::string;
    auto get_description(pile const* pile) -> pile_description;

    auto info() const -> game_info const&;
    auto piles() const -> std::unordered_map<pile_type, std::vector<pile*>> const&;
    auto rand() -> rng&;

    void start(size_f cardSize, std::optional<data::config::object> const& loadObj);
    void restart();
    void save(data::config::object& saveObj);
    void undo();
    auto can_undo() const -> bool;

    auto hover_at(point_i pos) -> hit_test_result;
    auto drop_target_at(rect_f const& rect, card const& move, isize numCards) -> hit_test_result;
    void drop_cards(hit_test_result const& hovered, hit_test_result const& dropTarget);
    auto get_available_hints() const -> std::vector<move> const&;

    auto virtual can_play(pile const& targetPile, isize targetIndex, card const& drop, isize numCards) const -> bool;

    void click(pile* srcPile, u8 clicks);
    void key_down(input::keyboard::event& ev);

    void update(milliseconds delta);

protected:
    auto virtual do_redeal() -> bool = 0;
    auto virtual do_deal() -> bool   = 0;

    auto virtual before_shuffle(card& card) -> bool                 = 0;
    auto virtual on_shuffle(card& card, pile_type pileType) -> bool = 0;
    void virtual after_shuffle()                                    = 0;

    void virtual on_end_turn() = 0;

    auto virtual check_state() const -> game_state;

    void create_piles(auto&& piles, isize size, std::function<void(pile&, i32)> const& func);

    void layout_piles();

    void end_turn(bool deal);

private:
    auto check_movable(pile const& targetPile, isize idx) const -> bool;
    void calc_available_moves();

    void new_game();
    auto load(std::optional<data::config::object> const& loadObj) -> bool;

    void init();
    void clear_piles();
    auto get_pile_at(point_i pos, bool ignoreActivePile) -> hit_test_result;

    auto deal_cards() -> bool;
    void auto_move_to_foundation(pile& srcPile);

    std::unordered_map<pile_type, std::vector<pile*>>         _piles;
    mutable flat_map<std::pair<pile const*, isize>, bool>     _movableCache;
    mutable std::unordered_map<pile const*, pile_description> _descriptionCache;
    std::vector<move>                                         _availableMoves;

    game_info   _info;
    size_f      _cardSize;
    card_table& _cardTable;

    data::config::object             _currentState;
    std::stack<data::config::object> _undoStack;

    rng _rand {}; // TODO: custom state
};

////////////////////////////////////////////////////////////

template <typename Table, template <typename> typename Function, isize IndexOffset>
class script_game : public base_game {
public:
    script_game(card_table& f, game_info info, Table table);

    auto can_play(pile const& targetPile, isize targetIndex, card const& drop, isize numCards) const -> bool override;

    void static CreateWrapper(auto&& script);
    template <typename T>
    void static CreateGlobals(auto&& scene, auto&& script, auto&& globalTable, auto&& makeFunc, string const& ext);

protected:
    auto do_redeal() -> bool override;
    auto do_deal() -> bool override;

    auto before_shuffle(card& card) -> bool override;
    auto on_shuffle(card& card, pile_type pileType) -> bool override;
    void after_shuffle() override;

    void on_end_turn() override;

    auto check_state() const -> game_state override;

private:
    void make_piles(auto&& gameRef);

    struct callbacks {
        std::optional<Function<bool>>       OnRedeal;
        std::optional<Function<bool>>       OnDeal;
        std::optional<Function<bool>>       OnBeforeShuffle;
        std::optional<Function<bool>>       OnShuffle;
        std::optional<Function<void>>       OnAfterShuffle;
        std::optional<Function<void>>       OnEndTurn;
        std::optional<Function<bool>>       CheckPlayable;
        std::optional<Function<game_state>> CheckState;
    };

    callbacks _callbacks;
    Table     _table;
};

////////////////////////////////////////////////////////////

class lua_script_game : public script_game<scripting::lua::table, scripting::lua::function, -1> {
public:
    lua_script_game(card_table& f, game_info info, scripting::lua::table tab);

    void static CreateAPI(start_scene* scene, scripting::lua::script& script, std::vector<scripting::lua::native_closure_shared_ptr>& funcs);
};

////////////////////////////////////////////////////////////

class squirrel_script_game : public script_game<scripting::squirrel::table, scripting::squirrel::function, 0> {
public:
    squirrel_script_game(card_table& f, game_info info, scripting::squirrel::table tab);

    void static CreateAPI(start_scene* scene, scripting::squirrel::script& script, std::vector<scripting::squirrel::native_closure_shared_ptr>& funcs);
};

}

#include "Games.inl"
