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

template <typename T>
inline void base_game::CreateWrapper(auto&& script, i32 indexOffset)
{
    using namespace scripting;

    // game
    auto& gameWrapper {*script.template create_wrapper<T>("script_game")};
    gameWrapper.template register_base<base_game>();
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
    gameWrapper["can_drop"]  = [indexOffset](base_game* game, pile* targetPile, isize targetIndex, card const& drop, isize numCards) {
        return game->base_game::can_drop(*targetPile, targetIndex + indexOffset, drop, numCards);
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
            auto const target {std::span<pile*>(to.data() + offset + indexOffset, size)};
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
            auto const target {std::span<pile*>(to.data() + offset + indexOffset, size)};
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
    pileWrapper["move_cards"]          = [indexOffset](pile* p, pile* to, isize startIndex, isize numCards, bool reverse) {
        p->move_cards(*to, startIndex + indexOffset, numCards, reverse);
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
        if (p->Cards.empty()) { return false; }

        for (auto* toPile : to) {
            if (ifEmpty && !toPile->Cards.empty()) { continue; }
            if (p->Cards.empty()) { break; }

            p->move_cards(*toPile, std::ssize(p->Cards) - 1, 1, false);
            toPile->flip_up_top_card();
        }

        return true;
    };
    pileWrapper["check_bounds"] = [](pile* p, isize i, point_i pos) { return p->Cards[i - 1].Bounds.contains(pos); };
}

template <typename T, typename Table, template <typename> typename Function>
inline void base_game::CreatePiles(T* game, auto&& gameRef)
{
    auto const createPile {[game](pile& pile, Table const& pileTab) {
        pile.Position  = pileTab["Position"].template get<point_f>().value_or(point_f::Zero);
        pile.Initial   = pileTab["Initial"].template get<std::vector<bool>>().value_or(std::vector<bool> {});
        pile.Layout    = pileTab["Layout"].template get<layout_type>().value_or(layout_type::Squared);
        pile.HasMarker = pileTab["HasMarker"].template get<bool>().value_or(true);

        if (Table ruleTable; pileTab.try_get(ruleTable, "Rule")) {
            pile.Rule.Build    = ruleTable["Build"].template get<build_type>().value_or(build_type::NoBuilding);
            pile.Rule.Interval = ruleTable["Interval"].template get<i32>().value_or(1);
            pile.Rule.Wrap     = ruleTable["Wrap"].template get<bool>().value_or(false);
            pile.Rule.Move     = ruleTable["Move"].template get<move_type>().value_or(move_type::Top);
            pile.Rule.Limit    = ruleTable["Limit"].template get<i32>().value_or(-1);

            if (Function<bool> emptyFunc; ruleTable.try_get(emptyFunc, "Empty")) {
                pile.Rule.Empty = empty::func {[game, emptyFunc](card const& card) { return emptyFunc(game, card); }};
            } else if (std::string empty; ruleTable.try_get(empty, "Empty")) {
                if (empty == "Ace") {
                    pile.Rule.Empty = empty::Ace();
                } else if (empty == "King") {
                    pile.Rule.Empty = empty::King();
                } else if (empty == "None") {
                    pile.Rule.Empty = empty::None();
                } else if (empty == "Any") {
                    pile.Rule.Empty = empty::Any();
                } else if (empty == "AnySingle") {
                    pile.Rule.Empty = {empty::Any(), true};
                } else if (empty == "FirstFoundation") {
                    pile.Rule.Empty = empty::First(game->Foundation[0]);
                }
            } else if (Table emptyTable; ruleTable.try_get(emptyTable, "Empty")) {
                if (std::string type; emptyTable.try_get(type, "Type")) {
                    if (type == "FirstFoundation") {
                        i32 const interval {emptyTable["Interval"].template get<i32>().value_or(0)};
                        pile.Rule.Empty = {empty::First(game->Foundation[0], interval)};
                    } else if (type == "Card") {
                        rank const       r {emptyTable["Rank"]};
                        suit_color const sc {emptyTable["Color"]};
                        pile.Rule.Empty = empty::Card(sc, r);
                    } else if (type == "Ranks") {
                        std::set<rank> const r {emptyTable["Ranks"].template as<std::set<rank>>()};
                        pile.Rule.Empty = empty::Ranks(r);
                    } else if (type == "Suits") {
                        std::set<suit> const s {emptyTable["Suits"].template as<std::set<suit>>()};
                        pile.Rule.Empty = empty::Suits(s);
                    }
                }
            }
        }
    }};

    auto const createPiles {[&](auto&& piles, std::string const& name) {
        if (Table pileTypeTable; gameRef.try_get(pileTypeTable, name)) {
            isize const size {pileTypeTable["Size"].template get<isize>().value_or(1)};
            if (size == 1 && !pileTypeTable.has("create")) { // table is definition
                game->create_piles(piles, 1, [&](auto& pile, i32) {
                    createPile(pile, pileTypeTable);
                });
            } else if (Table createTable; pileTypeTable.try_get(createTable, "create")) { // use 'create' table
                game->create_piles(piles, size, [&](auto& pile, i32) {
                    createPile(pile, createTable);
                });
            } else { // call 'create' function
                Function<Table> create {pileTypeTable["create"].template as<Function<Table>>()};
                game->create_piles(piles, size, [&](auto& pile, i32 i) {
                    createPile(pile, create(i));
                });
            }
        }
    }};

    createPiles(game->Stock, "Stock");
    createPiles(game->Waste, "Waste");
    createPiles(game->Reserve, "Reserve");
    createPiles(game->FreeCell, "FreeCell");
    createPiles(game->Foundation, "Foundation");
    createPiles(game->Tableau, "Tableau");

    if (Function<void> func; gameRef.try_get(func, "on_created")) {
        func(game);
    }
}

template <typename T, typename GameRef>
inline void base_game::CreateGlobals(auto&& scene, auto&& globalTable, auto&& makeFunc)
{
    globalTable["RegisterGame"] = makeFunc([scene](GameRef& tab) {
        games::game_info info;
        info.Name          = tab["Info"]["Name"].template as<std::string>();
        info.Type          = tab["Info"]["Type"];
        info.Family        = tab["Info"]["Family"];
        info.DeckCount     = tab["Info"]["DeckCount"];
        info.CardDealCount = tab["Info"]["CardDealCount"];
        info.Redeals       = tab["Info"]["Redeals"];

        auto func {[tab, info](auto& field) { return std::make_shared<T>(field, info, tab); }};
        scene->register_game(info, func);
    });
}

}
