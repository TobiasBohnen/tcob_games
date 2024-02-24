// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep
#include "Field.hpp"
#include "Piles.hpp"

namespace solitaire::games {
////////////////////////////////////////////////////////////

enum class type {
    // Closed
    SimpleBuilder,
    ReservedBuilder,
    SimplePacker,
    ReservedPacker,
    ClosedNonBuilder,
    // Half-Open
    Builder,
    Blockade,
    Planner,
    Packer,
    Spider,
    // Open
    OpenBuilder,
    OpenPacker,
    OpenNonBuilder
};

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
    type        Type {};
    family      Family {};

    i32 DeckCount {0};
    i32 CardDealCount {0};
    i32 Redeals {0};
};

////////////////////////////////////////////////////////////

class base_game {
public:
    using rng = random::rng_xoshiro_256_plus_plus;

    base_game(field& f, game_info info);
    virtual ~base_game() = default;

    std::vector<stock>      Stock;
    std::vector<waste>      Waste;
    std::vector<reserve>    Reserve;
    std::vector<freecell>   FreeCell;
    std::vector<tableau>    Tableau;
    std::vector<foundation> Foundation;

    auto get_name() const -> std::string;
    auto get_description(pile const* pile) const -> hover_info;

    auto info() const -> game_info;
    auto state() const -> game_state;
    auto piles() const -> std::unordered_map<pile_type, std::vector<pile*>> const&;

    void start(size_f cardSize, std::optional<data::config::object> const& loadObj);
    void restart();
    void save(data::config::object& saveObj);
    void undo();
    auto can_undo() const -> bool;

    auto hover_at(point_i pos) -> hit_test_result;
    auto drop_target_at(rect_f const& rect, card const& move, isize numCards) -> hit_test_result;
    void drop_cards(hit_test_result const& hovered, hit_test_result const& dropTarget);

    auto virtual can_drop(pile const& targetPile, isize targetIndex, card const& drop, isize numCards) const -> bool;

    void click(pile* srcPile, u8 clicks);
    void key_down(input::keyboard::event& ev);

protected:
    auto drop(pile& to, card& card) const -> bool;

    auto virtual do_redeal() -> bool;
    auto virtual do_deal() -> bool;

    auto virtual before_shuffle(card& card) -> bool;
    auto virtual shuffle(card& card, pile_type pileType) -> bool;
    void virtual after_shuffle();

    void virtual on_change();

    auto virtual check_state() const -> game_state;

    auto virtual check_movable(pile const& targetPile, isize idx) const -> bool;

    void add_pile(pile* p);
    void create_piles(auto&& piles, isize size, std::function<void(pile&, i32)> const& func);
    auto find_pile(card const& card) const -> pile*;

    void layout_piles();

    auto rand() -> rng&;
    auto redeals_left() const -> i32;

    void end_turn();

private:
    void new_game();
    auto load(std::optional<data::config::object> const& loadObj) -> bool;

    void init();
    void clear_pile_cards();
    auto get_pile_at(point_i pos, bool ignoreActivePile) -> hit_test_result;

    auto deal_cards() -> bool;
    void auto_deal(pile& from);
    void auto_move_to_foundation(pile& srcPile);

    std::unordered_map<pile_type, std::vector<pile*>> _piles;

    game_state _state {game_state::Initial};
    game_info  _gameInfo;
    size_f     _cardSize;
    field&     _field;
    i32        _turn {0};

    i32                              _remainingRedeals {};
    data::config::object             _currentState;
    std::stack<data::config::object> _undoStack;

    rng _rand {}; // TODO: custom state
};

inline void base_game::create_piles(auto&& piles, isize size, std::function<void(pile&, i32)> const& func)
{
    piles.reserve(size);
    for (i32 i {0}; i < size; ++i) {
        auto& pile {piles.emplace_back()};
        func(pile, i);
        add_pile(&pile);
    }
}

////////////////////////////////////////////////////////////

class script_game : public base_game {
public:
    script_game(field& f, game_info info, scripting::lua::table tab);

    void static CreateAPI(start_scene* scene, scripting::lua::script& script, std::vector<scripting::lua::native_closure_shared_ptr>& funcs);

    auto can_drop(pile const& targetPile, isize targetIndex, card const& drop, isize numCards) const -> bool override;

protected:
    auto do_redeal() -> bool override;
    auto do_deal() -> bool override;

    auto before_shuffle(card& card) -> bool override;
    auto shuffle(card& card, pile_type pileType) -> bool override;
    void after_shuffle() override;

    void on_change() override;

    auto check_state() const -> game_state override;

    auto check_movable(pile const& targetPile, isize idx) const -> bool override;

private:
    scripting::lua::table _table;
};

}
