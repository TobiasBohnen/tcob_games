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
        _piles[pile.Type].push_back(&pile);
    }
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline void script_game<Table, Function, IndexOffset>::CreateWrapper(auto&& script)
{
    using namespace scripting;

    // game
    auto& gameWrapper {*script.template create_wrapper<base_game>("script_game")};
    gameWrapper["RedealsLeft"]   = getter {[](base_game* game) { return game->info().RemainingRedeals; }};
    gameWrapper["CardDealCount"] = getter {[](base_game* game) { return game->info().CardDealCount; }};
    gameWrapper["DeckCount"]     = getter {[](base_game* game) { return game->info().DeckCount; }};

    auto static const returnPile {[](base_game* game, pile_type type) {
        auto const& piles {game->piles()};
        if (piles.contains(type)) { return game->piles().at(type); }
        return std::vector<pile*> {};
    }};

    // properties
    gameWrapper["Stock"]      = getter {[](base_game* game) { return returnPile(game, pile_type::Stock); }};
    gameWrapper["Waste"]      = getter {[](base_game* game) { return returnPile(game, pile_type::Waste); }};
    gameWrapper["Foundation"] = getter {[](base_game* game) { return returnPile(game, pile_type::Foundation); }};
    gameWrapper["Tableau"]    = getter {[](base_game* game) { return returnPile(game, pile_type::Tableau); }};
    gameWrapper["Reserve"]    = getter {[](base_game* game) { return returnPile(game, pile_type::Reserve); }};
    gameWrapper["FreeCell"]   = getter {[](base_game* game) { return returnPile(game, pile_type::FreeCell); }};

    // methods
    gameWrapper["shuffle_cards"] = [](base_game* game, std::vector<card> const& cards) {
        std::vector<card> shuffled {cards};
        game->rand().template shuffle<card>(shuffled);
        return shuffled;
    };
    gameWrapper["get_pile_index"] = [](base_game* game, pile* p) -> isize {
        auto const& piles {game->piles().at(p->Type)};
        return std::distance(piles.begin(), std::find(piles.begin(), piles.end(), p)) - IndexOffset;
    };
    gameWrapper["find_pile"] = [](base_game* game, card& card) -> pile* {
        for (auto const& piles : game->piles()) {
            for (auto* pile : piles.second) {
                for (auto const& c : pile->Cards) {
                    if (c == card) { return pile; }
                }
            }
        }

        return nullptr;
    };
    gameWrapper["can_play"] = [](base_game* game, pile* targetPile, isize targetIndex, card const& card, isize numCards) {
        return game->base_game::can_play(*targetPile, targetIndex + IndexOffset, card, numCards);
    };
    gameWrapper["play_card"] = [](base_game* game, pile* to, card& card) {
        if (game->base_game::can_play(*to, std::ssize(to->Cards) - 1, card, 1)) { // skip script check_playable here
            card.flip_face_up();
            to->Cards.emplace_back(card);
            return true;
        }

        return false;
    };

    // static methods
    auto static const placeCard {[](card& card, pile& to, bool ifEmpty, bool front) {
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
        [](card& card, std::vector<pile*>& to, bool ifEmpty) {
            for (auto& pile : to) {
                if (placeCard(card, *pile, ifEmpty, false)) { return true; }
            }
            return false;
        },
        [](card& card, std::vector<pile*>& to, i32 offset, usize size, bool ifEmpty) {
            auto const target {std::span<pile*>(to.data() + offset + IndexOffset, size)};
            for (auto& pile : target) {
                if (placeCard(card, *pile, ifEmpty, false)) { return true; }
            }
            return false;
        },
        [](card& card, pile* to, bool ifEmpty) {
            return placeCard(card, *to, ifEmpty, false);
        });
    gameWrapper["PlaceBottom"] = overload(
        [](card& card, std::vector<pile*>& to, bool ifEmpty) {
            for (auto& pile : to) {
                if (placeCard(card, *pile, ifEmpty, true)) { return true; }
            }
            return false;
        },
        [](card& card, std::vector<pile*>& to, i32 offset, usize size, bool ifEmpty) {
            auto const target {std::span<pile*>(to.data() + offset + IndexOffset, size)};
            for (auto& pile : target) {
                if (placeCard(card, *pile, ifEmpty, true)) { return true; }
            }
            return false;
        },
        [](card& card, pile* to, bool ifEmpty) {
            return placeCard(card, *to, ifEmpty, true);
        });

    // pile
    auto& pileWrapper {*script.template create_wrapper<pile>("pile")};

    // properties
    pileWrapper["Type"]      = getter {[](pile* p) { return p->Type; }};
    pileWrapper["IsEmpty"]   = getter {[](pile* p) { return p->empty(); }};
    pileWrapper["CardCount"] = getter {[](pile* p) { return p->Cards.size(); }};
    pileWrapper["Cards"]     = getter {[](pile* p) { return p->Cards; }};

    pileWrapper["Position"]   = property {[](pile* p) { return p->Position; }, [](pile* p, point_f pos) { p->Position = pos; }};
    pileWrapper["IsPlayable"] = property {[](pile* p) { return p->Rule.IsPlayable; }, [](pile* p, bool val) { p->Rule.IsPlayable = val; }};

    // methods
    pileWrapper["flip_cards"]         = [](pile* p, std::vector<bool> const& val) { p->flip_cards(val); };
    pileWrapper["flip_up_cards"]      = [](pile* p) { p->flip_up_cards(); };
    pileWrapper["flip_up_top_card"]   = [](pile* p) { p->flip_up_top_card(); };
    pileWrapper["flip_down_cards"]    = [](pile* p) { p->flip_down_cards(); };
    pileWrapper["flip_down_top_card"] = [](pile* p) { p->flip_down_top_card(); };
    pileWrapper["clear"]              = [](pile* p) { p->Cards.clear(); };

    pileWrapper["move_rank_to_bottom"] = [](pile* p, rank r) { std::ranges::stable_partition(p->Cards, [r](card const& c) { return c.get_rank() == r; }); };
    pileWrapper["move_cards"]          = [](pile* p, pile* to, isize startIndex, isize numCards, bool reverse) { p->move_cards(*to, startIndex + IndexOffset, numCards, reverse); };
    pileWrapper["play_card"]           = [](pile* p, pile* to, base_game* game) {
        if (p->empty()) { return false; }
        isize const toIdx {std::ssize(to->Cards) - 1};
        isize const pIdx {std::ssize(p->Cards) - 1};
        if (game->can_play(*to, toIdx, p->Cards[pIdx], 1)) {
            p->move_cards(*to, pIdx, 1, false);
            return true;
        }
        return false;
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
        infoTab.try_get(info.DeckRanks, "DeckRanks");
        infoTab.try_get(info.DeckSuits, "DeckSuits");
        infoTab.try_get(info.CardDealCount, "CardDealCount");
        infoTab.try_get(info.Redeals, "Redeals");
        infoTab.try_get(info.DisableHints, "DisableHints");

        auto func {[tab, info](auto& cardTable) { return std::make_shared<T>(cardTable, info, tab); }};
        scene->register_game(info, func);
    });

    globalTable["require"] = makeFunc([globalTable, &script, ext](std::string const& package) {
        if (globalTable.has("package", "loaded", package)) {
            return globalTable["package"]["loaded"][package].template as<Table>();
        }

        auto pkg {script.template run_file<Table>(package + "." + ext).value()};
        globalTable["package"]["loaded"][package] = pkg;
        return pkg;
    });
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline script_game<Table, Function, IndexOffset>::script_game(card_table& f, game_info info, Table table)
    : base_game {f, info}
    , _table {std::move(table)}
{
    make_piles(_table);
    _table.try_get(_callbacks.DoRedeal, "do_redeal");
    _table.try_get(_callbacks.DoDeal, "do_deal");
    _table.try_get(_callbacks.OnBeforeShuffle, "on_before_shuffle");
    _table.try_get(_callbacks.OnShuffle, "on_shuffle");
    _table.try_get(_callbacks.OnAfterShuffle, "on_after_shuffle");
    _table.try_get(_callbacks.OnInit, "on_init");
    _table.try_get(_callbacks.OnDrop, "on_drop");
    _table.try_get(_callbacks.OnEndTurn, "on_end_turn");
    _table.try_get(_callbacks.CheckPlayable, "check_playable");
    _table.try_get(_callbacks.GetState, "get_state");
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline auto script_game<Table, Function, IndexOffset>::can_play(pile const& targetPile, isize targetCardIndex, card const& card, isize numCards) const -> bool
{
    if (_callbacks.CheckPlayable) {
        return (*_callbacks.CheckPlayable)(static_cast<base_game const*>(this), &targetPile, targetCardIndex - IndexOffset, card, numCards);
    }
    return base_game::can_play(targetPile, targetCardIndex, card, numCards);
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline auto script_game<Table, Function, IndexOffset>::do_redeal() -> bool
{
    if (_callbacks.DoRedeal) {
        return (*_callbacks.DoRedeal)(static_cast<base_game const*>(this));
    }
    return false;
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline auto script_game<Table, Function, IndexOffset>::do_deal() -> bool
{
    if (_callbacks.DoDeal) {
        return (*_callbacks.DoDeal)(static_cast<base_game const*>(this));
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
inline auto script_game<Table, Function, IndexOffset>::on_shuffle(card& card, pile* pile) -> bool
{
    if (_callbacks.OnShuffle) {
        return (*_callbacks.OnShuffle)(static_cast<base_game const*>(this), card, pile);
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
inline void script_game<Table, Function, IndexOffset>::on_init()
{
    if (_callbacks.OnInit) {
        (*_callbacks.OnInit)(static_cast<base_game const*>(this));
    }
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline void script_game<Table, Function, IndexOffset>::on_drop(pile* pile)
{
    if (_callbacks.OnDrop) {
        (*_callbacks.OnDrop)(static_cast<base_game const*>(this), pile);
    }
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline void script_game<Table, Function, IndexOffset>::on_end_turn()
{
    if (_callbacks.OnEndTurn) {
        (*_callbacks.OnEndTurn)(static_cast<base_game const*>(this));
    }
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline auto script_game<Table, Function, IndexOffset>::get_state() const -> game_state
{
    if (_callbacks.GetState) {
        return (*_callbacks.GetState)(static_cast<base_game const*>(this));
    }
    return base_game::get_state();
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
            ruleTable.try_get(pile.Rule.Limit, "Limit");

            if (Table buildTable; ruleTable.try_get(buildTable, "Build")) {
                buildTable.try_get(pile.Rule.BuildHint, "Hint");

                if (Function<bool> func; buildTable.try_get(func, "Func")) {
                    pile.Rule.Build = {[this, func](card const& target, card const& card) {
                        return func(static_cast<base_game*>(this), target, card);
                    }};
                }
            }

            if (Table moveTable; ruleTable.try_get(moveTable, "Move")) {
                moveTable.try_get(pile.Rule.MoveHint, "Hint");

                if (!moveTable.try_get(pile.Rule.IsPlayable, "IsPlayable")) {
                    pile.Rule.IsPlayable = true;
                }
                if (!moveTable.try_get(pile.Rule.IsSequence, "IsSequence")) {
                    pile.Rule.IsSequence = true;
                }

                if (Function<bool> func; moveTable.try_get(func, "Func")) {
                    pile.Rule.Move = {[this, func](class pile const* target, isize idx) {
                        return func(static_cast<base_game*>(this), target, idx - IndexOffset);
                    }};
                }
            }

            if (Table baseTable; ruleTable.try_get(baseTable, "Base")) {
                baseTable.try_get(pile.Rule.BaseHint, "Hint");

                if (Function<bool> func; baseTable.try_get(func, "Func")) {
                    pile.Rule.Base = {[this, func](card const& card, isize numCards) {
                        return func(static_cast<base_game*>(this), card, numCards);
                    }};
                }
            }
        }
    }};

    auto const createPiles {[&](auto&& piles, std::string const& name) {
        if (Table pileTypeTable; gameRef.try_get(pileTypeTable, name)) {
            isize size {1};
            pileTypeTable.try_get(size, "Size");
            if (size == 1 && !pileTypeTable.has("Pile")) { // pile table is definition
                create_piles(piles, 1, [&](auto& pile, i32) {
                    createPile(pile, pileTypeTable);
                });
            } else if (Table createTable; pileTypeTable.try_get(createTable, "Pile")) { // use 'Pile' table
                create_piles(piles, size, [&](auto& pile, i32) {
                    createPile(pile, createTable);
                });
            } else { // call 'Pile' function
                Function<Table> create {pileTypeTable["Pile"].template as<Function<Table>>()};
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
}

}
