// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Games.hpp"

namespace solitaire::games {
////////////////////////////////////////////////////////////

inline void base_game::create_piles(auto&& piles, isize size, std::function<void(pile&, i32)> const& func)
{
    piles.reserve(size);
    for (i32 i {0}; i < size; ++i) {
        auto& pile {piles.emplace_back()};
        func(pile, i);
        add_pile(&pile);
    }
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline void script_game<Table, Function, IndexOffset>::CreateWrapper(auto&& script)
{
    using namespace scripting;

    // game
    auto& gameWrapper {*script.template create_wrapper<base_game>("script_game")};
    gameWrapper["RedealsLeft"]   = getter {[](base_game* game) { return game->redeals_left(); }};
    gameWrapper["CardDealCount"] = getter {[](base_game* game) { return game->info().CardDealCount; }};

    auto const returnPile {[](base_game* game, pile_type type) {
        auto const& piles {game->piles()};
        if (piles.contains(type)) { return game->piles().at(type); }
        return std::vector<pile*> {};
    }};

    // properties
    gameWrapper["Stock"]      = getter {[returnPile](base_game* game) { return returnPile(game, pile_type::Stock); }};
    gameWrapper["Waste"]      = getter {[returnPile](base_game* game) { return returnPile(game, pile_type::Waste); }};
    gameWrapper["Foundation"] = getter {[returnPile](base_game* game) { return returnPile(game, pile_type::Foundation); }};
    gameWrapper["Tableau"]    = getter {[returnPile](base_game* game) { return returnPile(game, pile_type::Tableau); }};
    gameWrapper["Reserve"]    = getter {[returnPile](base_game* game) { return returnPile(game, pile_type::Reserve); }};
    gameWrapper["FreeCell"]   = getter {[returnPile](base_game* game) { return returnPile(game, pile_type::FreeCell); }};

    // methods
    gameWrapper["shuffle_cards"] = [](base_game* game, std::vector<card> const& cards) {
        std::vector<card> shuffled {cards};
        game->rand().template shuffle<card>(shuffled);
        return shuffled;
    };
    gameWrapper["find_pile"] = [](base_game* game, card& card) { return game->find_pile(card); };
    gameWrapper["drop"]      = [](base_game* game, pile* to, card& card) { return game->drop(*to, card); };
    gameWrapper["can_drop"]  = [](base_game* game, pile* targetPile, isize targetIndex, card const& drop, isize numCards) {
        return game->base_game::can_drop(*targetPile, targetIndex + IndexOffset, drop, numCards);
    };

    // static methods
    auto placeCard {[](card& card, pile& to, bool ifEmpty, bool front) {
        bool const canPlace {ifEmpty ? to.Cards.empty() : true};
        if (canPlace) {
            card.flip_face_up();
            if (front) {
                to.Cards.emplace_front(card);
            } else {
                to.Cards.emplace_back(card);
            }
            return true;
        }
        return false;
    }};
    gameWrapper["PlaceTop"] = overload(
        [=](card& card, std::vector<pile*>& to, bool ifEmpty) {
            for (auto& pile : to) {
                if (placeCard(card, *pile, ifEmpty, false)) { return true; }
            }
            return false;
        },
        [=](card& card, std::vector<pile*>& to, i32 offset, usize size, bool ifEmpty) {
            auto const target {std::span<pile*>(to.data() + offset + IndexOffset, size)};
            for (auto& pile : target) {
                if (placeCard(card, *pile, ifEmpty, false)) { return true; }
            }
            return false;
        },
        [=](card& card, pile* to, bool ifEmpty) {
            return placeCard(card, *to, ifEmpty, false);
        });
    gameWrapper["PlaceBottom"] = overload(
        [=](card& card, std::vector<pile*>& to, bool ifEmpty) {
            for (auto& pile : to) {
                if (placeCard(card, *pile, ifEmpty, true)) { return true; }
            }
            return false;
        },
        [=](card& card, std::vector<pile*>& to, i32 offset, usize size, bool ifEmpty) {
            auto const target {std::span<pile*>(to.data() + offset + IndexOffset, size)};
            for (auto& pile : target) {
                if (placeCard(card, *pile, ifEmpty, true)) { return true; }
            }
            return false;
        },
        [=](card& card, pile* to, bool ifEmpty) {
            return placeCard(card, *to, ifEmpty, true);
        });

    // pile
    auto& pileWrapper {*script.template create_wrapper<pile>("pile")};

    // properties
    pileWrapper["Type"]      = getter {[](pile* p) { return p->Type; }};
    pileWrapper["IsEmpty"]   = getter {[](pile* p) { return p->empty(); }};
    pileWrapper["CardCount"] = getter {[](pile* p) { return p->Cards.size(); }};
    pileWrapper["Cards"]     = getter {[](pile* p) { return p->Cards; }};
    pileWrapper["Position"]  = property {[](pile* p) { return p->Position; }, [](pile* p, point_f pos) { p->Position = pos; }};

    // methods
    pileWrapper["flip_up_cards"]      = [](pile* p) { p->flip_up_cards(); };
    pileWrapper["flip_up_top_card"]   = [](pile* p) { p->flip_up_top_card(); };
    pileWrapper["flip_down_cards"]    = [](pile* p) { p->flip_down_cards(); };
    pileWrapper["flip_down_top_card"] = [](pile* p) { p->flip_down_top_card(); };
    pileWrapper["clear"]              = [](pile* p) { p->Cards.clear(); };

    pileWrapper["move_rank_to_bottom"] = [](pile* p, rank r) { std::ranges::stable_partition(p->Cards, [r](card const& c) { return c.get_rank() == r; }); };
    pileWrapper["move_cards"]          = [](pile* p, pile* to, isize startIndex, isize numCards, bool reverse) {
        p->move_cards(*to, startIndex + IndexOffset, numCards, reverse);
    };
    pileWrapper["redeal"] = [](pile* p, pile* to) {
        if (to->empty() && !p->empty()) {
            p->move_cards(*to, 0, std::ssize(p->Cards), true);
            to->flip_down_cards();
            return true;
        }

        return false;
    };
    pileWrapper["deal"] = [](pile* p, pile* to, i32 cardDealCount) {
        if (p->empty()) { return false; }

        for (i32 i {0}; i < cardDealCount; ++i) {
            p->move_cards(*to, std::ssize(p->Cards) - 1, 1, false);
        }
        to->flip_up_cards();

        return true;
    };
    pileWrapper["deal_to_group"] = [](pile* p, std::vector<pile*> const& to, bool ifEmpty) {
        if (p->empty()) { return false; }

        for (auto* toPile : to) {
            if (ifEmpty && !toPile->empty()) { continue; }
            if (p->empty()) { break; }

            p->move_cards(*toPile, std::ssize(p->Cards) - 1, 1, false);
            toPile->flip_up_top_card();
        }

        return true;
    };
    pileWrapper["check_bounds"] = [](pile* p, isize i, point_i pos) { return p->Cards[i - 1].Bounds.contains(pos); };
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
template <typename T>
inline void script_game<Table, Function, IndexOffset>::CreateGlobals(auto&& scene, auto&& script, auto&& globalTable, auto&& makeFunc, string const& ext)
{
    globalTable["Sol"]["register_game"] = makeFunc([scene](Table& tab) {
        auto             infoTab {tab["Info"].template as<Table>()};
        games::game_info info;

        infoTab.try_get(info.Name, "Name");
        infoTab.try_get(info.Family, "Family");
        infoTab.try_get(info.DeckCount, "DeckCount");
        infoTab.try_get(info.CardDealCount, "CardDealCount");
        infoTab.try_get(info.Redeals, "Redeals");

        auto func {[tab, info](auto& field) { return std::make_shared<T>(field, info, tab); }};
        scene->register_game(info, func);
    });

    globalTable["require"] = makeFunc([&globalTable, &script, ext](std::string const& package) {
        if (globalTable.has("package", "loaded", package)) {
            return globalTable["package"]["loaded"][package].template as<Table>();
        }

        auto pkg {script.template run_file<Table>(package + "." + ext).value()};
        globalTable["package"]["loaded"][package] = pkg;
        return pkg;
    });
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline script_game<Table, Function, IndexOffset>::script_game(field& f, game_info info, Table table)
    : base_game {f, info}
    , _table {std::move(table)}
{
    make_piles(_table);
    _table.try_get(_callbacks.CheckDrop, "check_drop");
    _table.try_get(_callbacks.OnRedeal, "on_redeal");
    _table.try_get(_callbacks.OnDeal, "on_deal");
    _table.try_get(_callbacks.OnBeforeShuffle, "on_before_shuffle");
    _table.try_get(_callbacks.OnShuffle, "on_shuffle");
    _table.try_get(_callbacks.OnAfterShuffle, "on_after_shuffle");
    _table.try_get(_callbacks.OnChange, "on_change");
    _table.try_get(_callbacks.CheckState, "check_state");
    _table.try_get(_callbacks.CheckMovable, "check_movable");
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline auto script_game<Table, Function, IndexOffset>::can_drop(pile const& targetPile, isize targetIndex, card const& drop, isize numCards) const -> bool
{
    if (_callbacks.CheckDrop) {
        return (*_callbacks.CheckDrop)(static_cast<base_game const*>(this), &targetPile, targetIndex + IndexOffset, drop, numCards);
    }
    return base_game::can_drop(targetPile, targetIndex, drop, numCards);
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline auto script_game<Table, Function, IndexOffset>::do_redeal() -> bool
{
    if (_callbacks.OnRedeal) {
        return (*_callbacks.OnRedeal)(static_cast<base_game const*>(this));
    }
    return false;
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline auto script_game<Table, Function, IndexOffset>::do_deal() -> bool
{
    if (_callbacks.OnDeal) {
        return (*_callbacks.OnDeal)(static_cast<base_game const*>(this));
    }
    return false;
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline auto script_game<Table, Function, IndexOffset>::before_shuffle(card& card) -> bool
{
    if (_callbacks.OnBeforeShuffle) {
        return (*_callbacks.OnBeforeShuffle)(static_cast<base_game const*>(this), card);
    }
    return false;
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline auto script_game<Table, Function, IndexOffset>::on_shuffle(card& card, pile_type pileType) -> bool
{
    if (_callbacks.OnShuffle) {
        return (*_callbacks.OnShuffle)(static_cast<base_game const*>(this), card, pileType);
    }
    return false;
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline void script_game<Table, Function, IndexOffset>::after_shuffle()
{
    if (_callbacks.OnAfterShuffle) {
        (*_callbacks.OnAfterShuffle)(static_cast<base_game const*>(this));
    }
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline void script_game<Table, Function, IndexOffset>::on_change()
{
    if (_callbacks.OnChange) {
        (*_callbacks.OnChange)(static_cast<base_game const*>(this));
    }
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline auto script_game<Table, Function, IndexOffset>::check_state() const -> game_state
{
    if (_callbacks.CheckState) {
        return (*_callbacks.CheckState)(static_cast<base_game const*>(this));
    }
    return base_game::check_state();
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline auto script_game<Table, Function, IndexOffset>::check_movable(pile const& targetPile, isize idx) -> bool
{
    if (_callbacks.CheckMovable) {
        return (*_callbacks.CheckMovable)(static_cast<base_game const*>(this), &targetPile, idx + IndexOffset);
    }
    return base_game::check_movable(targetPile, idx);
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline void script_game<Table, Function, IndexOffset>::make_piles(auto&& gameRef)
{
    auto const createPile {[this](pile& pile, Table const& pileTab) {
        pileTab.try_get(pile.Position, "Position");
        pileTab.try_get(pile.Initial, "Initial");
        pileTab.try_get(pile.Layout, "Layout");
        pileTab.try_get(pile.HasMarker, "HasMarker");

        if (Table ruleTable; pileTab.try_get(ruleTable, "Rule")) {
            ruleTable.try_get(pile.Rule.Interval, "Interval");
            ruleTable.try_get(pile.Rule.Wrap, "Wrap");
            ruleTable.try_get(pile.Rule.Limit, "Limit");

            if (Table buildTable; ruleTable.try_get(buildTable, "Build")) {
                buildTable.try_get(pile.Rule.BuildHint, "Hint");

                if (Function<bool> func; buildTable.try_get(func, "Build")) {
                    pile.Rule.Build = {[func](card const& target, card const& drop, i32 interval, bool wrap) {
                        return func(target, drop, interval, wrap);
                    }};
                }
            }

            if (Table moveTable; ruleTable.try_get(moveTable, "Move")) {
                if (!moveTable.try_get(pile.Rule.IsPlayable, "IsPlayable")) {
                    pile.Rule.IsPlayable = true;
                }
                if (!moveTable.try_get(pile.Rule.IsSequence, "IsSequence")) {
                    pile.Rule.IsSequence = true;
                }

                if (Function<bool> func; moveTable.try_get(func, "Move")) {
                    pile.Rule.Move = {[this, func](class pile const* target, isize idx) {
                        return func(static_cast<base_game*>(this), target, idx - IndexOffset);
                    }};
                }
            }

            if (Function<Function<bool>> emptyFunc; ruleTable.try_get(emptyFunc, "Base")) {
                auto func {emptyFunc(static_cast<base_game*>(this))};
                pile.Rule.Base = {[func](card const& card, isize numCards) {
                    return func(card, numCards);
                }};
            }
        }
    }};

    auto const createPiles {[&](auto&& piles, std::string const& name) {
        if (Table pileTypeTable; gameRef.try_get(pileTypeTable, name)) {
            isize size {1};
            pileTypeTable.try_get(size, "Size");
            if (size == 1 && !pileTypeTable.has("Create")) { // pile table is definition
                create_piles(piles, 1, [&](auto& pile, i32) {
                    createPile(pile, pileTypeTable);
                });
            } else if (Table createTable; pileTypeTable.try_get(createTable, "Create")) { // use 'Create' table
                create_piles(piles, size, [&](auto& pile, i32) {
                    createPile(pile, createTable);
                });
            } else { // call 'Create' function
                Function<Table> create {pileTypeTable["Create"].template as<Function<Table>>()};
                create_piles(piles, size, [&](auto& pile, i32 i) {
                    createPile(pile, create(i));
                });
            }
        }
    }};

    createPiles(Stock, "Stock");
    createPiles(Waste, "Waste");
    createPiles(Reserve, "Reserve");
    createPiles(FreeCell, "FreeCell");
    createPiles(Foundation, "Foundation");
    createPiles(Tableau, "Tableau");

    if (Function<void> func; gameRef.try_get(func, "on_created")) {
        func(static_cast<base_game*>(this));
    }
}

}
