// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Games.hpp"

#include "MainScene.hpp"

#include <ranges>
#include <utility>

namespace solitaire {

constexpr i32 SCORE_FOUNDATION = 10;
constexpr i32 SCORE_TABLEAU    = 1;

base_game::base_game(game_info info)
    : _info {std::move(info)}
{
}

void base_game::start(std::optional<data::config::object> const& loadObj, std::optional<u64> seed)
{
    Status         = game_status::Initial;
    _state.Redeals = _info.Redeals;

    if (!load(loadObj)) {
        if (seed) { _rng = {*seed}; }
        new_game();
    }

    init();
}

void base_game::new_game()
{
    reset_piles();

    // create decks
    std::vector<card> cards {get_shuffled()};

    // before setup
    for (isize i {std::ssize(cards) - 1}; i >= 0; --i) {
        auto& card {cards[i]};
        if (before_setup(card)) { // AKA moving specific cards to piles -> TODO: make it a pile property
            cards.erase(cards.begin() + i);
        }
    }

    // shuffle to piles
    static constexpr std::array<pile_type, 6> shuffleOrder {pile_type::Tableau, pile_type::Foundation, pile_type::Reserve, pile_type::FreeCell, pile_type::Waste, pile_type::Stock};

    for (auto const pileType : shuffleOrder) {
        if (!_pileMap.contains(pileType)) { continue; }

        auto const& piles {_pileMap.at(pileType)};
        for (auto* pile : piles) {
            for (isize i {0}; i < std::ssize(pile->Initial); ++i) {
                assert(!cards.empty()); // TODO: log error
                auto& card {cards.back()};
                if (!on_setup(card, pile)) {
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

    after_setup();
    assert(cards.empty()); // TODO: log error

    // deal cards if game contains Waste pile
    if (_pileMap.contains(pile_type::Waste)) { do_deal(); }
}

auto base_game::load(std::optional<data::config::object> const& loadObj) -> bool
{
    using namespace tcob::data::config;

    if (!loadObj) { return false; }

    reset_piles();

    if (!loadObj->has(_info.Name)) { return false; }

    object const obj {loadObj->get<object>(_info.Name).value()};
    if (!obj.has("State") || !obj.has("RNG")) { return false; }

    _state = obj["State"].as<game_state>();
    _rng   = obj["RNG"].as<game_rng>();

    _storage.clear();
    if (object storage; obj.try_get(storage, "Storage")) { _storage = storage.clone(true); }

    auto const createCard {[&](entry const& entry) { return card::FromValue(entry.as<u16>()); }};
    for (auto const& [type, piles] : _pileMap) {
        auto const pileType {get_pile_type_name(type)};
        if (!obj.has(pileType)) { return false; }

        array const typeArray {obj[pileType].as<array>()};
        auto const  pileCount {std::ssize(piles)};

        if (typeArray.size() != pileCount) { return false; }

        for (isize i {0}; i < pileCount; ++i) {
            pile*       pile {piles[i]};
            array const pileArray {typeArray[i].as<array>()};
            for (auto const& cardEntry : pileArray) {
                auto card {createCard(cardEntry)};
                pile->Cards.emplace_back(card);
            }
            pile->remove_tint();
        }
    }

    return true;
}

void base_game::save(tcob::data::config::object& saveObj)
{
    using namespace tcob::data::config;

    object obj;

    for (auto const& [type, piles] : _pileMap) {
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

    obj["State"] = _state;
    obj["RNG"]   = _rng;
    if (!_storage.empty()) { obj["Storage"] = _storage.clone(true); }

    saveObj[_info.Name] = obj;
}

void base_game::init()
{
    on_init();
    refresh();

    _saveObj = {};
    save(_saveObj);
}

void base_game::refresh()
{
    // update marker texture
    for (auto const& [_, piles] : _pileMap) {
        for (auto const* pile : piles) {
            if (pile->HasMarker) {
                if (pile->Marker) {
                    pile->Marker->TextureRegion = pile->get_marker_texture_name();
                } else {
                    // TODO: create marker
                }
            }
        }
    }

    _movableCache.clear();
    calc_hints();
    Status = get_status();
    Layout();
}

void base_game::undo()
{
    if (can_undo()) {
        i32 const oldTurns {_state.Turns};
        f64 const oldTime {_state.Time.count()};
        i32 const oldHints {_state.Hints};
        i32 const oldUndos {_state.Undos};

        load(_undoStack.top());
        _undoStack.pop();

        init();

        _state.Turns = oldTurns + 1;
        _state.Time  = milliseconds {oldTime};
        _state.Hints = oldHints;
        _state.Undos = oldUndos + 1;
    }
}

auto base_game::can_undo() const -> bool
{
    return !_undoStack.empty();
}

void base_game::use_hint()
{
    if (!_hints.empty()) {
        ++_state.Hints;
    }
}

void base_game::end_turn(bool deal)
{
    ++_state.Turns;

    on_end_turn();

    refresh();

    _undoStack.push(_saveObj);
    _saveObj = {};
    save(_saveObj);

    if (deal // if all Waste piles are empty
        && !Waste.empty()
        && std::ranges::all_of(Waste, [](auto&& waste) { return waste.empty(); })) {
        deal_cards();
    }
}

void base_game::update(milliseconds delta)
{
    // TODO: pause
    if (Status == game_status::Running) {
        _state.Time += delta;
    }
}

auto base_game::auto_play_cards(pile& from) -> bool
{
    if (from.Type == pile_type::Foundation || !from.is_playable() || _info.DisableAutoPlay) { return false; }

    auto& card {from.Cards.back()};

    for (auto& fou : Foundation) {
        if (!can_play(fou, fou.size() - 1, card, 1)) { continue; }
        play_cards(from, fou, from.size() - 1, 1);
        return true;
    }

    return false;
}

void base_game::collect_all()
{
    bool check {true};
    while (check) {
        check = false;
        for (auto const& [type, piles] : _pileMap) {
            if (type == pile_type::Foundation) { continue; }

            for (auto* pile : piles) {
                if (auto_play_cards(*pile)) { check = true; }
            }
        }
    }
}

void base_game::play_cards(pile& from, pile& to, isize startIndex, isize numCards)
{
    from.move_cards(to, startIndex, numCards, false);

    if (to.Type == pile_type::Foundation && from.Type != pile_type::Foundation) {
        give_score(SCORE_FOUNDATION);
    } else if (to.Type != pile_type::Foundation && from.Type == pile_type::Foundation) {
        give_score(-SCORE_FOUNDATION);
    } else if (to.Type == pile_type::Tableau && from.Type == pile_type::Waste) {
        give_score(SCORE_TABLEAU);
    }

    on_drop(&to);
    end_turn(true);
}

void base_game::reset_piles()
{
    for (auto const& [_, piles] : _pileMap) {
        for (auto* pile : piles) {
            pile->reset();
        }
    }
}

void base_game::give_score(i32 value)
{
    _state.Score += value;
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
    if (_state.Redeals != 0) {
        // e.g. Waste -> Stock
        if (do_redeal()) {
            if (_state.Redeals > 0) {
                --_state.Redeals;
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

auto base_game::get_status() const -> game_status
{
    isize foundationCards {0};
    isize tableauCards {0};
    for (auto const& [type, piles] : _pileMap) {
        switch (type) {
        case pile_type::Foundation:
            for (auto const& pile : piles) {
                foundationCards += pile->size();
            }
            break;
        case pile_type::Tableau:
            for (auto const& pile : piles) {
                tableauCards += pile->size();
            }
            break;
        case pile_type::Stock:
        case pile_type::Waste:
        case pile_type::Reserve:
        case pile_type::FreeCell: break;
        }
    }

    isize const maxCards {_info.DeckCount * std::ssize(_info.DeckRanks) * std::ssize(_info.DeckSuits)};
    switch (_info.Objective) {
    case objective::AllCardsToFoundation:
        if (foundationCards == maxCards) { return game_status::Success; }
        break;
    case objective::AllCardsButOneToFoundation:
        if (foundationCards == maxCards - 1) { return game_status::Success; }
        break;
    case objective::AllCardsButTwoToFoundation:
        if (foundationCards == maxCards - 2) { return game_status::Success; }
        break;
    case objective::AllCardsButFourToFoundation:
        if (foundationCards == maxCards - 4) { return game_status::Success; }
        break;
    case objective::ClearTableau:
        if (tableauCards == 0) { return game_status::Success; }
        break;
    case objective::AllCardsToTableau:
        if (tableauCards == maxCards) { return game_status::Success; }
        break;
    }

    if (Stock.empty() || (Stock[0].empty() && _state.Redeals == 0)) {
        if (_hints.empty() && !_info.DisableHints) {
            return game_status::Failure;
        }
    }

    return game_status::Running;
}

auto base_game::get_shuffled() -> std::vector<card>
{
    return deck::GetShuffled(_rng.gen(), _info.DeckCount, _info.DeckSuits, _info.DeckRanks);
}

void base_game::calc_hints()
{
    if (_info.DisableHints) {
        _hints.clear();
        return;
    }

    auto static const validHint {[](auto const& src, auto const& dst) {
        switch (src.Src->Type) {
        case pile_type::Foundation:
            // ignore Foundation to Foundation or to FreeCell
            if (dst->Type == pile_type::Foundation || dst->Type == pile_type::FreeCell) { return false; }
            break;
        case pile_type::FreeCell: {
            // ignore FreeCell to FreeCell type
            if (dst->Type == pile_type::FreeCell) { return false; }
        } break;
        case pile_type::Tableau: {
            // ignore first card Tableau to empty Tableau
            if (src.SrcCardIdx == 0 && dst->Type == pile_type::Tableau && dst->empty()) { return false; }
        } break;
        case pile_type::Stock:
        case pile_type::Waste:
        case pile_type::Reserve:
            break;
        }

        // limit Foundation/FreeCell destinations to 1
        if (dst->Type == pile_type::Foundation && src.HasFoundation) { return false; }
        if (dst->Type == pile_type::FreeCell && src.HasFreeCell) { return false; }
        // ignore markerless pile without cards
        if (!dst->HasMarker && dst->Cards.empty()) { return false; }
        return true;
    }};

    std::vector<hint> movable;
    for (auto const& [_, piles] : _pileMap) {
        for (auto* pile : piles) {
            if (!pile->is_playable()) { continue; }

            for (isize srcCardIdx {0}; srcCardIdx < pile->size(); ++srcCardIdx) {
                if (!check_movable(*pile, srcCardIdx)) { continue; }

                auto& m {movable.emplace_back()};
                m.Src        = pile;
                m.SrcPileIdx = pile->Index;
                m.SrcCardIdx = srcCardIdx;
            }
        }
    }

    _hints.clear();
    _hints.reserve(movable.size());
    for (auto const& [type, piles] : _pileMap) {
        if (type == pile_type::Stock || type == pile_type::Waste) { continue; } // skip Stock/Waste destination
        for (auto* dst : piles) {
            for (auto& src : movable) {
                if (src.Src == dst) { continue; }

                if (!validHint(src, dst)) { continue; }

                if (!can_play(*dst,
                              dst->size() - 1,
                              src.Src->Cards[src.SrcCardIdx],
                              src.Src->size() - src.SrcCardIdx)) { continue; }

                auto& m {_hints.emplace_back()};
                m.Src        = src.Src;
                m.SrcPileIdx = src.SrcPileIdx;
                m.SrcCardIdx = src.SrcCardIdx;
                m.Dst        = dst;
                m.DstPileIdx = dst->Index;
                m.DstCardIdx = dst->size() - 1;

                if (dst->Type == pile_type::Foundation) { src.HasFoundation = true; }
                if (dst->Type == pile_type::FreeCell) { src.HasFreeCell = true; }
            }
        }
    }
}

auto base_game::get_available_hints() const -> std::vector<hint> const&
{
    return _hints;
}

auto base_game::rng() -> game_rng&
{
    return _rng;
}

auto base_game::piles() const -> std::unordered_map<pile_type, std::vector<pile*>> const&
{
    return _pileMap;
}

auto base_game::state() const -> game_state const&
{
    return _state;
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

auto lua_script_game::CreateENV(scripting::lua::script& script) -> scripting::lua::table
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
    // env["collectgarbage"] = global["collectgarbage"];

    script.set_environment(env);
    return env;
}

void lua_script_game::CreateAPI(main_scene* scene, scripting::lua::script& script, std::vector<scripting::lua::native_closure_shared_ptr>& funcs)
{
    using namespace scripting::lua;
    auto env {CreateENV(script)};

    auto make_func {[&](auto&& func) {
        auto ptr {make_shared_closure(std::function {func})};
        funcs.push_back(ptr);
        return ptr.get();
    }};

    CreateGlobals<lua_script_game>(scene, script, env, make_func, "lua");
    CreateWrapper(script);

    std::ignore = script.run_file("scripts/main.lua");

    auto& global {script.get_global_table()};
    global["Sol"]["Layout"] = env["Sol"]["Layout"];
}

////////////////////////////////////////////////////////////

squirrel_script_game::squirrel_script_game(game_info info, scripting::squirrel::table tab)
    : script_game {std::move(info), std::move(tab)}
{
}

void squirrel_script_game::CreateAPI(main_scene* scene, scripting::squirrel::script& script, std::vector<scripting::squirrel::native_closure_shared_ptr>& funcs)
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

    std::ignore = script.run_file("scripts/main.nut");
}

} // namespace solitaire
