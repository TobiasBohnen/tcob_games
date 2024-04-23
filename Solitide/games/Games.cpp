// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Games.hpp"

#include "StartScene.hpp"

#include <ranges>
#include <utility>

namespace solitaire {

base_game::base_game(game_info info)
    : _info {std::move(info)}
{
    _info.RemainingRedeals = _info.Redeals;
}

void base_game::start(std::optional<data::config::object> const& loadObj)
{
    State = game_state::Initial;

    if (!load(loadObj)) { new_game(); }

    init();
}

void base_game::new_game()
{
    clear_piles();

    _info.InitialSeed = _rand.get_state();

    // create decks
    std::vector<card> cards {get_shuffled()};

    // on_before_shuffle
    for (isize i {std::ssize(cards) - 1}; i >= 0; --i) {
        auto& card {cards[i]};
        if (before_shuffle(card)) { // AKA moving specific cards to piles -> TODO: make it a pile property
            cards.erase(cards.begin() + i);
        }
    }

    // shuffle to piles
    static constexpr std::array<pile_type, 6> shuffleOrder {pile_type::Tableau, pile_type::Foundation, pile_type::Reserve, pile_type::FreeCell, pile_type::Waste, pile_type::Stock};

    for (auto const pileType : shuffleOrder) {
        if (!_piles.contains(pileType)) { continue; }

        auto const& piles {_piles.at(pileType)};
        for (auto* pile : piles) {
            for (isize i {0}; i < std::ssize(pile->Initial); ++i) {
                assert(!cards.empty());
                auto& card {cards.back()};
                if (!on_shuffle(card, pile)) {
                    if (pile->Initial[i]) {
                        card.flip_face_up();
                    } else {
                        card.flip_face_down();
                    }
                    pile->Cards.emplace_back(card);
                }
                cards.pop_back();
            }
        }
    }

    after_shuffle();
    assert(cards.empty());

    // deal cards if game contains Waste pile
    if (_piles.contains(pile_type::Waste)) { do_deal(); }
}

auto base_game::load(std::optional<data::config::object> const& loadObj) -> bool
{
    using namespace tcob::data::config;

    if (!loadObj) { return false; }

    clear_piles();

    if (!loadObj->has(_info.Name)) { return false; }

    object const obj {loadObj->get<object>(_info.Name).value()};
    if (!obj.has("Redeals") || !obj.has("Turn") || !obj.has("RNGSeed") || !obj.has("RNGState") || !obj.has("Time")) { return false; }

    _info.RemainingRedeals = obj["Redeals"].as<i32>();
    _info.Turn             = obj["Turn"].as<i32>();
    _info.Time             = milliseconds {obj["Time"].as<f64>()};
    _info.InitialSeed      = obj["RNGSeed"].as<rng::state_type>();
    _rand                  = rng {obj["RNGState"].as<rng::state_type>()};

    _storage.clear();
    if (object storage; obj.try_get(storage, "Storage")) { _storage = storage.clone(true); }

    auto const createCard {[&](entry const& entry) { return card::FromValue(entry.as<u16>()); }};
    for (auto const& [type, piles] : _piles) {
        auto const pileType {get_pile_type_name(type)};
        if (!obj.has(pileType)) { return false; }

        array const typeArray {obj[pileType].as<array>()};
        auto const  pileCount {std::ssize(piles)};

        if (typeArray.get_size() != pileCount) { return false; }

        for (isize i {0}; i < pileCount; ++i) {
            pile*       pile {piles[i]};
            array const pileArray {typeArray[i].as<array>()};
            for (auto const& cardEntry : pileArray) {
                auto card {createCard(cardEntry)};
                pile->Cards.emplace_back(card);
            }
        }
    }

    return true;
}

void base_game::save(tcob::data::config::object& saveObj)
{
    using namespace tcob::data::config;

    object obj;

    for (auto const& [type, piles] : _piles) {
        array pilesArr;
        for (auto const& pile : piles) {
            array pileArr;
            for (auto const& card : pile->Cards) {
                pileArr.add(card.to_value());
            }
            pilesArr.add(pileArr);
        }
        obj[get_pile_type_name(type)] = pilesArr;
    }

    obj["Redeals"]  = _info.RemainingRedeals;
    obj["RNGSeed"]  = _info.InitialSeed;
    obj["RNGState"] = _rand.get_state();
    obj["Turn"]     = _info.Turn;
    obj["Time"]     = _info.Time.count();
    if (!_storage.empty()) { obj["Storage"] = _storage.clone(true); }

    saveObj[_info.Name] = obj;
}

void base_game::init()
{
    on_init();
    refresh();

    _saveState = {};
    save(_saveState);
}

void base_game::refresh()
{
    _movableCache.clear();
    calc_hints();
    State = get_state();
    Layout();
}

void base_game::undo()
{
    if (can_undo()) {
        i32 const oldTurn {_info.Turn};
        f64 const oldTime {_info.Time.count()};

        load(_undoStack.top());
        _undoStack.pop();

        init();
        _info.Turn = oldTurn + 1;
        _info.Time = milliseconds {oldTime};
    }
}

auto base_game::can_undo() const -> bool
{
    return !_undoStack.empty();
}

void base_game::end_turn(bool deal)
{
    ++_info.Turn;

    on_end_turn();

    refresh();

    _undoStack.push(_saveState);
    _saveState = {};
    save(_saveState);

    if (deal // if all Waste piles are empty
        && !Waste.empty()
        && std::ranges::all_of(Waste, [](auto&& waste) { return waste.empty(); })) {
        deal_cards();
    }
}

void base_game::update(milliseconds delta)
{
    // TODO: pause
    if (State == game_state::Running) {
        _info.Time += delta;
    }
}

void base_game::auto_play_cards(pile& from)
{
    if (!from.empty()) {
        auto& card {from.Cards.back()};

        for (auto& fou : Foundation) {
            if (!can_play(fou, std::ssize(fou.Cards) - 1, card, 1)) { continue; }
            play_cards(from, fou, std::ssize(from.Cards) - 1, 1);
            return;
        }
    }
}

void base_game::play_cards(pile& from, pile& to, isize startIndex, isize numCards)
{
    from.move_cards(to, startIndex, numCards, false);
    on_drop(&to);
    end_turn(true);
}

void base_game::clear_piles()
{
    for (auto const& [_, piles] : _piles) {
        for (auto* pile : piles) {
            pile->Cards.clear();
            pile->IsHovering = false;
            pile->remove_tint();
        }
    }
}

auto base_game::can_play(pile const& targetPile, isize targetCardIndex, card const& card, isize numCards) const -> bool
{
    return targetPile.build(targetCardIndex, card, numCards);
}

auto base_game::check_movable(pile const& targetPile, isize idx) const -> bool
{
    std::pair<pile const*, isize> const key {&targetPile, idx};

    if (auto it {_movableCache.find(key)}; it != _movableCache.end()) {
        return it->second;
    }

    bool const retValue {targetPile.Rule.Move(&targetPile, idx)};
    _movableCache[key] = retValue;
    return retValue;
}

auto base_game::deal_cards() -> bool
{
    if (_info.RemainingRedeals != 0) {
        // e.g. Waste -> Stock
        if (do_redeal()) {
            if (_info.RemainingRedeals > 0) {
                --_info.RemainingRedeals;
            }

            end_turn(false);
        }
    }

    if (do_deal()) {
        end_turn(false);
        return true;
    }

    return false;
}

auto base_game::get_state() const -> game_state
{
    // success if cards only on foundation piles
    bool success {true};
    for (auto const& [type, piles] : _piles) {
        if (type != pile_type::Foundation) {
            for (auto const& pile : piles) {
                if (!pile->empty()) { success = false; }
            }
        }
    }
    if (success) { return game_state::Success; }

    if (Stock.empty() || (Stock[0].empty() && _info.RemainingRedeals == 0)) {
        if (get_available_hints().empty()) {
            return game_state::Failure;
        }
    }

    return game_state::Running;
}

auto base_game::get_shuffled() -> std::vector<card>
{
    return deck::GetShuffled(_rand, _info.DeckCount, _info.DeckSuits, _info.DeckRanks);
}

void base_game::calc_hints()
{
    if (_info.DisableHints) {
        _hints.clear();
        return;
    }

    std::vector<move> movable;
    for (auto const& [_, piles] : _piles) {
        isize srcIdx {0};
        for (auto* pile : piles) {
            if (!pile->is_playable()) { continue; }

            for (isize srcCardIdx {0}; srcCardIdx < std::ssize(pile->Cards); ++srcCardIdx) {
                if (!check_movable(*pile, srcCardIdx)) { continue; }

                auto& m {movable.emplace_back()};
                m.Src        = pile;
                m.SrcIdx     = srcIdx;
                m.SrcCardIdx = srcCardIdx;
            }
            srcIdx++;
        }
    }

    _hints.clear();
    _hints.reserve(movable.size());
    for (auto const& [_, piles] : _piles) {
        isize dstIdx {0};
        for (auto* dst : piles) {
            if (dst->Type == pile_type::Stock || dst->Type == pile_type::Waste || dst->Type == pile_type::Reserve) { continue; } // skip Stock/Waste/Reserve
            for (auto& src : movable) {
                if (src.Src == dst) { continue; }
                if (src.Src->Type == pile_type::Foundation && dst->Type == pile_type::Foundation) { continue; }                  // ignore Foundation to Foundation
                if (dst->Type == pile_type::FreeCell) { continue; }                                                              // ignore anything to FreeCell
                if (dst->Type == pile_type::Foundation && src.HasFoundation) { continue; }                                       // limit foundation destinations to 1

                if (can_play(*dst,
                             dst->empty() ? -1 : dst->Cards.size() - 1,
                             src.Src->Cards[src.SrcCardIdx],
                             std::ssize(src.Src->Cards) - src.SrcCardIdx)) {
                    if (dst->Type == pile_type::Foundation) { src.HasFoundation = true; }

                    auto& m {_hints.emplace_back()};
                    m.Src        = src.Src;
                    m.SrcIdx     = src.SrcIdx;
                    m.SrcCardIdx = src.SrcCardIdx;
                    m.Dst        = dst;
                    m.DstIdx     = dstIdx;
                    m.DstCardIdx = std::ssize(dst->Cards) - 1;
                }
            }
            dstIdx++;
        }
    }
}

auto base_game::get_available_hints() const -> std::vector<move> const&
{
    return _hints;
}

auto base_game::rand() -> rng&
{
    return _rand;
}

auto base_game::piles() const -> std::unordered_map<pile_type, std::vector<pile*>> const&
{
    return _piles;
}

auto base_game::info() const -> game_info const&
{
    return _info;
}

auto base_game::storage() -> data::config::object*
{
    return &_storage;
}

////////////////////////////////////////////////////////////

using namespace scripting;

lua_script_game::lua_script_game(game_info info, lua::table tab)
    : script_game {std::move(info), std::move(tab)}
{
}

void lua_script_game::CreateAPI(start_scene* scene, scripting::lua::script& script, std::vector<scripting::lua::native_closure_shared_ptr>& funcs)
{
    using namespace scripting::lua;
    script.open_libraries(library::Table, library::String, library::Math, library::Coroutine);
    auto& global {script.get_global_table()};

    table env {script.create_table()};
    env["table"]     = global["table"];
    env["string"]    = global["string"];
    env["math"]      = global["math"];
    env["coroutine"] = global["coroutine"];

    env["pairs"]        = global["pairs"];
    env["ipairs"]       = global["ipairs"];
    env["print"]        = global["print"];
    env["type"]         = global["type"];
    env["tonumber"]     = global["tonumber"];
    env["tostring"]     = global["tostring"];
    env["setmetatable"] = global["setmetatable"];
    env["getmetatable"] = global["getmetatable"];

    script.set_environment(env);

    auto make_func {[&](auto&& func) {
        auto ptr {make_shared_closure(std::function {func})};
        funcs.push_back(ptr);
        return ptr.get();
    }};

    CreateGlobals<lua_script_game>(scene, script, env, make_func, "lua");
    CreateWrapper(script);

    std::ignore = script.run_file("main.lua");

    global["Sol"]["Layout"] = env["Sol"]["Layout"];
}

////////////////////////////////////////////////////////////

squirrel_script_game::squirrel_script_game(game_info info, scripting::squirrel::table tab)
    : script_game {std::move(info), std::move(tab)}
{
}

void squirrel_script_game::CreateAPI(start_scene* scene, scripting::squirrel::script& script, std::vector<scripting::squirrel::native_closure_shared_ptr>& funcs)
{
    using namespace scripting::squirrel;

    script.open_libraries(library::Math, library::String);

    auto make_func {[&](auto&& func) {
        auto ptr {make_shared_closure(std::function {func})};
        funcs.push_back(ptr);
        return ptr.get();
    }};

    auto& root {script.get_root_table()};
    auto  view {script.get_view()};

    CreateGlobals<squirrel_script_game>(scene, script, root, make_func, "nut");
    CreateWrapper(script);

    // Lua interop
    table lua {view};

    table meta {view};
    meta["_get"]  = make_func([](stack_base& base, std::string const& func) {
        auto funcSig {base["_funcSig"].as<std::vector<std::string>>()};
        funcSig.push_back(func);
        base["_funcSig"] = funcSig;
        return base;
    });
    meta["_call"] = make_func([scene, view](stack_base const&, table& tab) {
        auto const types {view.get_stack_types()};
        lua_params args {};

        for (isize i {3}; i < std::ssize(types); ++i) {
            if (types[i - 1] == scripting::squirrel::type::Userdata) {
                base_game* val {};
                if (view.pull_convert_idx(i, val)) { args.Items.emplace_back(val); }
            } else {
                lua_value val {};
                if (view.pull_convert_idx(i, val)) {
                    std::visit([&args](auto&& item) { args.Items.emplace_back(item); }, val);
                }
            }
        }

        auto const funcSig {tab["_funcSig"].as<std::vector<std::string>>()};
        return scene->call_lua(funcSig, args);
    });

    table luaMeta {view};
    luaMeta["_get"] = make_func([meta, view](std::string const& func) {
        table tab {view};
        tab.set_delegate(meta);
        tab["_funcSig"] = std::vector<std::string> {func};
        return tab;
    });
    lua.set_delegate(luaMeta);

    root["Lua"] = lua;

    std::ignore = script.run_file("main.nut");
}

} // namespace solitaire
