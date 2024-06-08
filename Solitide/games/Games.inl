// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Games.hpp"

#include <chrono>
#include <format>

namespace solitaire {
////////////////////////////////////////////////////////////

inline void base_game::create_piles(auto&& piles, isize size, std::function<void(pile&, i32)> const& func)
{
    piles.reserve(size);
    for (i32 i {0}; i < size; ++i) {
        auto& pile {piles.emplace_back()};
        pile.Index = i;
        func(pile, i);
        _pileMap[pile.Type].push_back(&pile);
    }
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline void script_game<Table, Function, IndexOffset>::CreateWrapper(auto&& script)
{
    using namespace scripting;

    // game
    auto& gameWrapper {*script.template create_wrapper<base_game>("script_game")};
    // properties
    gameWrapper["RedealsLeft"] = getter {[](base_game* game) { return game->state().Redeals; }};
    gameWrapper["DeckCount"]   = getter {[](base_game* game) { return game->info().DeckCount; }};

    auto static const returnPile {[](base_game* game, pile_type type) {
        auto const& piles {game->piles()};
        if (piles.contains(type)) { return game->piles().at(type); }
        return std::vector<pile*> {};
    }};

    gameWrapper["Stock"]      = getter {[](base_game* game) { return returnPile(game, pile_type::Stock); }};
    gameWrapper["Waste"]      = getter {[](base_game* game) { return returnPile(game, pile_type::Waste); }};
    gameWrapper["Foundation"] = getter {[](base_game* game) { return returnPile(game, pile_type::Foundation); }};
    gameWrapper["Tableau"]    = getter {[](base_game* game) { return returnPile(game, pile_type::Tableau); }};
    gameWrapper["Reserve"]    = getter {[](base_game* game) { return returnPile(game, pile_type::Reserve); }};
    gameWrapper["FreeCell"]   = getter {[](base_game* game) { return returnPile(game, pile_type::FreeCell); }};

    gameWrapper["Name"]    = getter {[](base_game* game) { return game->info().Name; }};
    gameWrapper["Storage"] = getter {[](base_game* game) { return game->storage(); }};

    // methods
    gameWrapper["shuffle_cards"] = [](base_game* game, std::vector<card> const& cards) {
        std::vector<card> shuffled {cards};
        game->rng().gen().template shuffle<card>(shuffled);
        return shuffled;
    };
    gameWrapper["find_pile"] = [](base_game* game, card& card) -> pile* {
        for (auto const& piles : game->piles()) {
            for (auto* pile : piles.second) {
                auto it {std::find(pile->Cards.begin(), pile->Cards.end(), card)};
                if (it != pile->Cards.end()) { return pile; }
            }
        }

        return nullptr;
    };
    gameWrapper["give_score"] = [](base_game* game, i32 score) {
        game->give_score(score);
    };
    gameWrapper["can_play"] = [](base_game* game, pile* targetPile, isize targetIndex, card const& card, isize numCards) {
        return game->base_game::can_play(*targetPile, targetIndex + IndexOffset, card, numCards);
    };
    gameWrapper["play_card"] = [](base_game* game, card& card, pile* to) {
        if (game->base_game::can_play(*to, to->size() - 1, card, 1)) { // skip script can_play here
            card.flip_face_up();
            to->Cards.emplace_back(card);
            return true;
        }

        return false;
    };
    gameWrapper["play_top_card"] = [](base_game* game, pile* from, pile* to) {
        if (from->empty()) { return false; }
        isize const toIdx {to->size() - 1};
        isize const pIdx {from->size() - 1};
        if (game->can_play(*to, toIdx, from->Cards[pIdx], 1)) {
            from->move_cards(*to, pIdx, 1, false);
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

    // getter
    pileWrapper["Type"]       = getter {[](pile* p) { return p->Type; }};
    pileWrapper["Index"]      = getter {[](pile* p) { return p->Index - IndexOffset; }};
    pileWrapper["IsEmpty"]    = getter {[](pile* p) { return p->empty(); }};
    pileWrapper["CardCount"]  = getter {[](pile* p) { return p->size(); }};
    pileWrapper["Cards"]      = getter {[](pile* p) { return p->Cards; }};
    pileWrapper["IsPlayable"] = getter {[](pile* p) { return p->Rule.IsPlayable(); }};

    // properties -- not saved
    pileWrapper["Position"] = property {[](pile* p) { return p->Position; }, [](pile* p, point_f pos) { p->Position = pos; }};

    // methods
    pileWrapper["flip_cards"]         = [](pile* p, std::vector<bool> const& val) { p->flip_cards(val); };
    pileWrapper["flip_up_cards"]      = [](pile* p) { p->flip_up_cards(); };
    pileWrapper["flip_up_top_card"]   = [](pile* p) { p->flip_up_top_card(); };
    pileWrapper["flip_down_cards"]    = [](pile* p) { p->flip_down_cards(); };
    pileWrapper["flip_down_top_card"] = [](pile* p) { p->flip_down_top_card(); };
    pileWrapper["clear_cards"]        = [](pile* p) { p->Cards.clear(); };

    pileWrapper["shift_rank_to_bottom"] = [](pile* p, rank r) { std::ranges::stable_partition(p->Cards, [r](card const& c) { return c.get_rank() == r; }); };
    pileWrapper["shift_card"]           = [](pile* p, isize from, isize to) {
        from += IndexOffset;
        to += IndexOffset;
        isize const count {p->size()};
        if (from < 0 || from >= count || to < 0 || to > count) { return false; }

        if (from > to) {
            std::rotate(p->Cards.rend() - from - 1, p->Cards.rend() - from, p->Cards.rend() - to);
        } else {
            std::rotate(p->Cards.begin() + from, p->Cards.begin() + from + 1, p->Cards.begin() + to + 1);
        }
        return true;
    };
    pileWrapper["move_cards"]     = [](pile* p, pile* to, isize startIndex, isize numCards, bool reverse) { p->move_cards(*to, startIndex + IndexOffset, numCards, reverse); };
    pileWrapper["get_card_index"] = [](pile* p, card const& c) -> isize {
        auto const& cards {p->Cards};
        auto const  it {std::find(cards.begin(), cards.end(), c)};
        if (it == cards.end()) { return -1 - IndexOffset; }
        return std::distance(cards.begin(), it) - IndexOffset;
    };

    // object
    auto& objWrap {*script.template create_wrapper<data::config::object>("object")};
    objWrap.UnknownGet.connect([](auto&& ev) {
        ev.Handled = true;

        if (ev.Instance) {
            auto const proxy {(*ev.Instance)[ev.Name]};
            if (i64 val {0}; proxy.try_get(val)) {
                ev.return_value(val);
                return;
            }
            if (f64 val {0}; proxy.try_get(val)) {
                ev.return_value(val);
                return;
            }
            if (bool val {}; proxy.try_get(val)) {
                ev.return_value(val);
                return;
            }
            if (std::string val {}; proxy.try_get(val)) {
                ev.return_value(val);
                return;
            }

            ev.return_value(nullptr);
        }
    });

    objWrap.UnknownSet.connect([](auto&& ev) {
        if (ev.Instance) {
            if (i64 val {0}; ev.get_value(val)) {
                (*ev.Instance)[ev.Name] = val;
                ev.Handled              = true;
                return;
            }
            if (f64 val {0}; ev.get_value(val)) {
                (*ev.Instance)[ev.Name] = val;
                ev.Handled              = true;
                return;
            }
            if (bool val {}; ev.get_value(val)) {
                (*ev.Instance)[ev.Name] = val;
                ev.Handled              = true;
                return;
            }
            if (std::string val {}; ev.get_value(val)) {
                (*ev.Instance)[ev.Name] = val;
                ev.Handled              = true;
                return;
            }
        }
    });
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
template <typename T>
inline void script_game<Table, Function, IndexOffset>::CreateGlobals(auto&& scene, auto&& script, auto&& globalTable, auto&& makeFunc, std::string const& ext)
{
    auto const now {std::chrono::system_clock::now()};
    globalTable["Sol"]["Date"] = std::format("{:%Y%m%d}", now);

    globalTable["Sol"]["register_game"] = makeFunc([scene](Table& tab) {
        Table infoTab;
        if (!tab.is_valid() || !tab.try_get(infoTab, "Info")) { return; } // TODO: error

        game_info info;

        infoTab.try_get(info.Name, "Name");
        infoTab.try_get(info.Family, "Family");
        infoTab.try_get(info.Objective, "Objective");
        infoTab.try_get(info.DeckCount, "DeckCount");
        infoTab.try_get(info.DeckRanks, "DeckRanks");
        infoTab.try_get(info.DeckSuits, "DeckSuits");
        infoTab.try_get(info.Redeals, "Redeals");
        infoTab.try_get(info.DisableHints, "DisableHints");
        infoTab.try_get(info.DisableAutoPlay, "DisableAutoPlay");

        auto func {[tab, info]() { return std::make_shared<T>(info, tab); }};
        scene->register_game(info, func);
    });

    globalTable["require"] = makeFunc([globalTable, &script, ext](std::string const& package) {
        if (globalTable.has("package", "loaded", package)) {
            return globalTable["package"]["loaded"][package].template as<Table>();
        }

        string file {package + "." + ext};
        if (!io::exists(file)) { file = "scripts/" + file; }

        auto pkg {script.template run_file<Table>(file).value()};
        globalTable["package"]["loaded"][package] = pkg;
        return pkg;
    });
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline script_game<Table, Function, IndexOffset>::script_game(game_info info, Table table)
    : base_game {info}
    , _table {std::move(table)}
{
    make_piles(_table);
    _table.try_get(_callbacks.Redeal, "redeal");
    _table.try_get(_callbacks.Deal, "deal");
    _table.try_get(_callbacks.OnBeforeShuffle, "on_before_shuffle");
    _table.try_get(_callbacks.OnShuffle, "on_shuffle");
    _table.try_get(_callbacks.OnAfterShuffle, "on_after_shuffle");
    _table.try_get(_callbacks.OnInit, "on_init");
    _table.try_get(_callbacks.OnDrop, "on_drop");
    _table.try_get(_callbacks.OnEndTurn, "on_end_turn");
    _table.try_get(_callbacks.CanPlay, "can_play");
    _table.try_get(_callbacks.GetStatus, "get_status");
    _table.try_get(_callbacks.GetShuffled, "get_shuffled");
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline auto script_game<Table, Function, IndexOffset>::can_play(pile const& targetPile, isize targetCardIndex, card const& card, isize numCards) const -> bool
{
    if (_callbacks.CanPlay) {
        return (*_callbacks.CanPlay)(static_cast<base_game const*>(this), &targetPile, targetCardIndex - IndexOffset, card, numCards);
    }
    return base_game::can_play(targetPile, targetCardIndex, card, numCards);
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline auto script_game<Table, Function, IndexOffset>::do_redeal() -> bool
{
    if (_callbacks.Redeal) {
        return (*_callbacks.Redeal)(static_cast<base_game const*>(this));
    }
    return false;
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline auto script_game<Table, Function, IndexOffset>::do_deal() -> bool
{
    if (_callbacks.Deal) {
        return (*_callbacks.Deal)(static_cast<base_game const*>(this));
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
inline auto script_game<Table, Function, IndexOffset>::get_status() const -> game_status
{
    if (_callbacks.GetStatus) {
        return (*_callbacks.GetStatus)(static_cast<base_game const*>(this));
    }
    return base_game::get_status();
}

template <typename Table, template <typename> typename Function, isize IndexOffset>
inline auto script_game<Table, Function, IndexOffset>::get_shuffled() -> std::vector<card>
{
    if (_callbacks.GetShuffled) {
        auto const retValue {(*_callbacks.GetShuffled)(static_cast<base_game const*>(this), rng().seed())};
        return retValue.empty() ? base_game::get_shuffled() : retValue;
    }
    return base_game::get_shuffled();
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
                    pile.Rule.Build = {[this, func](card const& base, card const& drop) {
                        return func(static_cast<base_game*>(this), base, drop);
                    }};
                }
            }

            if (Table moveTable; ruleTable.try_get(moveTable, "Move")) {
                moveTable.try_get(pile.Rule.MoveHint, "Hint");

                if (Function<bool> func; moveTable.try_get(func, "IsPlayable")) {
                    pile.Rule.IsPlayable = {[this, func]() {
                        return func(static_cast<base_game*>(this));
                    }};
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
            if (size == 0) { return; }
            if (size == 1 && !pileTypeTable.has("Pile")) {                              // pile table is definition
                create_piles(piles, 1, [&](auto& pile, i32) { createPile(pile, pileTypeTable); });
            } else if (Table createTable; pileTypeTable.try_get(createTable, "Pile")) { // use 'Pile' table
                create_piles(piles, size, [&](auto& pile, i32) { createPile(pile, createTable); });
            } else if (Function<Table> create; pileTypeTable.try_get(create, "Pile")) { // call 'Pile' function
                create_piles(piles, size, [&](auto& pile, i32 i) { createPile(pile, create(i)); });
            } else {
                assert(false);                                                          // TODO: log error
            }
        }
    }};

    createPiles(Stock, "Stock");
    createPiles(Waste, "Waste");
    createPiles(Reserve, "Reserve");
    createPiles(FreeCell, "FreeCell");
    createPiles(Foundation, "Foundation");
    createPiles(Tableau, "Tableau");

    if (!Foundation.empty()) {
        i32 const limit {static_cast<i32>(info().DeckCount * info().DeckRanks.size() * info().DeckSuits.size() / Foundation.size())};

        for (auto& foundation : Foundation) {
            if (foundation.Rule.Limit == AUTO_LIMIT) {
                foundation.Rule.Limit = limit;
            }
        }
    }
}
}
