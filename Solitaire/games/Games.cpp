// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Games.hpp"

#include "Field.hpp"
#include "StartScene.hpp"

#include <ranges>
#include <utility>

namespace solitaire::games {

base_game::base_game(field& f, game_info info)
    : _gameInfo {std::move(info)}
    , _field {f}
    , _remainingRedeals {info.Redeals}
{
}

auto base_game::get_name() const -> std::string
{
    return _gameInfo.Name;
}

auto base_game::get_description(pile const* pile) -> hover_info
{
    if (!pile) { return {}; }

    if (auto it {_descriptionCache.find(pile)}; it != _descriptionCache.end()) {
        return it->second;
    }

    auto const retValue {pile->get_description(*this)};
    _descriptionCache[pile] = retValue;
    return retValue;
}

void base_game::start(size_f cardSize, std::optional<data::config::object> const& loadObj)
{
    _cardSize = cardSize;
    _state    = game_state::Initial;

    if (!load(loadObj)) { new_game(); }

    init();
}

void base_game::restart()
{
    start(_cardSize, _currentState);
}

void base_game::new_game()
{
    clear_piles();

    std::vector<card> cards;
    for (i32 i {0}; i < _gameInfo.DeckCount; ++i) {
        deck const deck {deck::GetShuffled(_rand, static_cast<u8>(i), _gameInfo.DeckSuits, _gameInfo.DeckRanks)};
        cards.insert(cards.end(), deck.Cards.begin(), deck.Cards.end());
    }

    for (isize i {std::ssize(cards) - 1}; i >= 0; --i) {
        auto& card {cards[i]};
        if (before_shuffle(card)) { // AKA moving specific cards to piles -> TODO: make it a pile property
            cards.erase(cards.begin() + i);
        }
    }

    for (auto& [_, piles] : _piles) {
        for (auto* pile : piles) {
            for (isize i {0}; i < std::ssize(pile->Initial); ++i) {
                assert(!cards.empty());
                auto& card {cards.back()};
                if (!on_shuffle(card, pile->Type)) {
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

    if (!loadObj->has(_gameInfo.Name)) { return false; }

    object const obj {loadObj->get<object>(_gameInfo.Name).value()};
    if (!obj.has("Redeals") || !obj.has("Turn")) { return false; }
    _remainingRedeals = obj["Redeals"].as<i32>();
    _turn             = obj["Turn"].as<i32>();

    auto const createCard {[&](entry const& entry) { return card::FromValue(entry.as<u16>()); }};
    for (auto& [type, piles] : _piles) {
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

    for (auto const& kvp : _piles) {
        array pilesArr;
        for (auto const& pile : kvp.second) {
            array pileArr;
            for (auto const& card : pile->Cards) {
                pileArr.add(card.to_value());
            }
            pilesArr.add(pileArr);
        }
        obj[get_pile_type_name(kvp.first)] = pilesArr;
    }

    obj["Redeals"] = _remainingRedeals;
    obj["Turn"]    = _turn;

    saveObj[_gameInfo.Name] = obj;
}

void base_game::init()
{
    layout_piles();

    _currentState = {};
    save(_currentState);

    _movableCache.clear();
    _descriptionCache.clear();
    calc_available_moves();
    _state = check_state();
}

void base_game::undo()
{
    if (can_undo()) {
        i32 const oldTurn {_turn};
        load(_undoStack.top());
        _undoStack.pop();

        init();
        _turn = oldTurn + 1;
    }
}

auto base_game::can_undo() const -> bool
{
    return !_undoStack.empty();
}

void base_game::end_turn(bool deal)
{
    ++_turn;

    on_end_turn();
    layout_piles();

    _undoStack.push(_currentState);
    _currentState = {};
    save(_currentState);

    // deal if all Waste piles are empty
    if (deal
        && !Waste.empty()
        && std::ranges::all_of(Waste, [](auto&& waste) { return waste.empty(); })) {
        deal_cards();
    }

    _movableCache.clear();
    _descriptionCache.clear();
    calc_available_moves();
    _state = check_state();
}

void base_game::layout_piles()
{
    for (auto& [_, piles] : _piles) {
        for (auto* pile : piles) {
            point_f pos {multiply(pile->Position, _cardSize)};

            switch (pile->Layout) {
            case layout_type::Squared: {
                for (auto& card : pile->Cards) {
                    card.Bounds = {pos, _cardSize};
                }
            } break;
            case layout_type::Column: { // TODO: break large columns
                for (auto& card : pile->Cards) {
                    card.Bounds = {pos, _cardSize};
                    if (card.is_face_down()) {
                        pos.Y += _cardSize.Height / FACE_DOWN_OFFSET;
                    } else {
                        pos.Y += _cardSize.Height / FACE_UP_OFFSET;
                    }
                }
            } break;
            case layout_type::Row: {
                for (auto& card : pile->Cards) {
                    card.Bounds = {pos, _cardSize};
                    if (card.is_face_down()) {
                        pos.X += _cardSize.Width / FACE_DOWN_OFFSET;
                    } else {
                        pos.X += _cardSize.Height / FACE_UP_OFFSET;
                    }
                }
            } break;
            case layout_type::Fan: {
                if (pile->empty()) { break; }
                for (isize i {0}; i < std::ssize(pile->Cards); ++i) {
                    auto& card {pile->Cards[i]};
                    if (i < std::ssize(pile->Cards) - 3) {
                        card.Bounds = {pos, _cardSize};
                    } else {
                        card.Bounds = {pos, _cardSize};
                        pos.X += _cardSize.Height / FACE_UP_OFFSET;
                    }
                }
            } break;
            }
        }
    }

    _field.mark_dirty();
}

auto base_game::get_pile_at(point_i pos, bool ignoreActivePile) -> hit_test_result
{
    auto const checkPile {[&](pile const& p) -> isize {
        if (ignoreActivePile && p.is_active()) { return INDEX_INVALID; }
        if (p.empty() && p.Marker && p.Marker->Bounds->contains(pos)) { return INDEX_MARKER; }

        for (isize i {std::ssize(p.Cards) - 1}; i >= 0; --i) {
            if (p.Cards[i].Bounds.contains(pos) && check_movable(p, i)) { return i; }
        }

        return INDEX_INVALID;
    }};

    for (auto const& entry : _piles) {
        for (auto* pile : entry.second | std::views::reverse) {
            isize const idx {checkPile(*pile)};
            if (idx != INDEX_INVALID) { return {pile, idx}; }
        }
    }

    return {};
}

auto base_game::drop_target_at(rect_f const& rect, card const& move, isize numCards) -> hit_test_result
{
    std::array<point_i, 4> points {
        point_i {rect.top_left()}, point_i {rect.top_right()},
        point_i {rect.bottom_left()}, point_i {rect.bottom_right()}};

    std::vector<hit_test_result> candidates;
    for (auto const& point : points) {
        if (auto target {get_pile_at(point, true)};
            target.Pile
            && target.Index == std::ssize(target.Pile->Cards) - 1
            && can_play(*target.Pile, target.Index, move, numCards)) {
            candidates.push_back(target);
        }
    }

    if (candidates.empty()) { return {}; }
    if (candidates.size() == 1) { return candidates[0]; }

    f32             maxArea {0};
    hit_test_result retValue;
    for (auto const& candidate : candidates) {
        auto const interSect {rect.as_intersected(candidate.Index == INDEX_MARKER
                                                      ? candidate.Pile->Marker->Bounds
                                                      : candidate.Pile->Cards[candidate.Index].Bounds)};
        if (interSect.Width * interSect.Height > maxArea) {
            maxArea  = interSect.Width * interSect.Height;
            retValue = candidate;
        }
    }

    return retValue;
}

auto base_game::hover_at(point_i pos) -> hit_test_result
{
    return get_pile_at(pos, false);
}

void base_game::key_down(input::keyboard::event& ev)
{
    using namespace tcob::enum_ops;

    if (!ev.Repeat) {
        if (ev.KeyCode == input::key_code::SPACE) {
            deal_cards();
            ev.Handled = true;
        } else if (ev.KeyCode == input::key_code::z && (ev.KeyMods & input::key_mod::LeftControl) == input::key_mod::LeftControl) {
            undo();
            ev.Handled = true;
        }
    }
}

void base_game::click(pile* srcPile, u8 clicks)
{
    if (!srcPile || srcPile->Type == pile_type::Foundation) { return; }

    if (srcPile->Type == pile_type::Stock) {
        // deal card
        srcPile->remove_tint();
        deal_cards();
    } else if (clicks > 1) {
        // try move to foundation
        auto_move_to_foundation(*srcPile);
    }
}

void base_game::auto_move_to_foundation(pile& srcPile)
{
    if (!srcPile.empty()) {
        auto& card {srcPile.Cards.back()};

        for (auto& fou : Foundation) {
            if (!can_play(fou, std::ssize(fou.Cards) - 1, card, 1)) { continue; }

            srcPile.move_cards(fou, std::ssize(srcPile.Cards) - 1, 1, false);
            end_turn(true);
            srcPile.remove_tint();
            fou.remove_tint();
            return;
        }
    }
}

void base_game::drop_cards(hit_test_result const& hovered, hit_test_result const& dropTarget)
{
    if (hovered.Pile) { hovered.Pile->remove_tint(); }
    if (dropTarget.Pile) { dropTarget.Pile->remove_tint(); }

    if (dropTarget.Pile && hovered.Pile) {
        hovered.Pile->move_cards(*dropTarget.Pile, hovered.Index, std::ssize(hovered.Pile->Cards) - hovered.Index, false);
        end_turn(true);
    } else {
        layout_piles();
    }
}

void base_game::clear_piles()
{
    for (auto& kvp : _piles) {
        for (auto* pile : kvp.second) {
            pile->Cards.clear();
            pile->set_active(false, -2, colors::Transparent);
        }
    }
}

auto base_game::can_play(pile const& targetPile, isize targetIndex, card const& drop, isize numCards) const -> bool
{
    return targetPile.build(targetIndex, drop, numCards);
}

auto base_game::check_movable(pile const& targetPile, isize idx) const -> bool
{
    std::pair<pile const*, isize> const key {&targetPile, idx};

    if (auto it {_movableCache.find(key)}; it != _movableCache.end()) {
        return it->second;
    }

    if (!targetPile.is_playable()) { // non-playable -> hover top
        return move_top(&targetPile, idx);
    }

    bool const retValue {targetPile.Rule.Move(&targetPile, idx)};
    _movableCache[key] = retValue;
    return retValue;
}

auto base_game::deal_cards() -> bool
{
    if (_remainingRedeals != 0) {
        // e.g. Waste -> Stock
        if (do_redeal()) {
            if (_remainingRedeals > 0) {
                --_remainingRedeals;
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

auto base_game::check_state() const -> game_state
{
    // success if cards only on foundation piles
    bool success {true};
    for (auto const& kvp : _piles) {
        if (kvp.first != pile_type::Foundation) {
            for (auto const& pile : kvp.second) {
                if (!pile->empty()) { success = false; }
            }
        }
    }
    if (success) { return game_state::Success; }

    if (Stock.empty() || (Stock[0].empty() && _remainingRedeals == 0)) {
        if (get_available_moves().empty()) {
            return game_state::Failure;
        }
    }

    return game_state::Running;
}

void base_game::calc_available_moves()
{
    if (_gameInfo.DisableHints) {
        _availableMoves.clear();
        return;
    }

    std::vector<move> movable;
    for (auto const& kvp : _piles) {
        isize srcIdx {0};
        for (auto* pile : kvp.second) {
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

    _availableMoves.clear();
    _availableMoves.reserve(movable.size());
    for (auto const& kvp : _piles) {
        isize dstIdx {0};
        for (auto* dst : kvp.second) {
            if (dst->Type == pile_type::Stock || dst->Type == pile_type::Waste || dst->Type == pile_type::Reserve) { continue; }
            for (auto& src : movable) {
                if (src.Src == dst) { continue; }
                if (src.Src->Type == pile_type::Foundation && dst->Type == pile_type::Foundation) { continue; } // ignore Foundation to Foundation
                if (src.Src->Type == pile_type::Foundation && dst->Type == pile_type::FreeCell) { continue; }   // ignore Foundation to FreeCell
                if (src.Src->Type == pile_type::FreeCell && dst->Type == pile_type::FreeCell) { continue; }     // ignore FreeCell to FreeCell
                if (dst->Type == pile_type::Foundation && src.HasFoundation) { continue; }                      // limit foundation/freecell destinations to 1
                if (dst->Type == pile_type::FreeCell && src.HasFreeCell) { continue; }

                if (can_play(*dst,
                             dst->empty() ? -1 : dst->Cards.size() - 1,
                             src.Src->Cards[src.SrcCardIdx],
                             std::ssize(src.Src->Cards) - src.SrcCardIdx)) {
                    if (dst->Type == pile_type::Foundation) {
                        src.HasFoundation = true;
                    } else if (dst->Type == pile_type::FreeCell) {
                        src.HasFreeCell = true;
                    }

                    auto& m {_availableMoves.emplace_back()};
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

auto base_game::get_available_moves() const -> std::vector<move> const&
{
    return _availableMoves;
}

auto base_game::rand() -> rng&
{
    return _rand;
}

auto base_game::piles() const -> std::unordered_map<pile_type, std::vector<pile*>> const&
{
    return _piles;
}

auto base_game::info() const -> game_info
{
    return _gameInfo;
}

auto base_game::state() const -> game_state
{
    return _state;
}

auto base_game::redeals_left() const -> i32
{
    return _remainingRedeals;
}

////////////////////////////////////////////////////////////

using namespace scripting;

lua_script_game::lua_script_game(field& f, game_info info, lua::table tab)
    : script_game {f, std::move(info), std::move(tab)}
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

    (void)script.run_file("main.lua");
    global["Sol"]["Layout"] = env["Sol"]["Layout"];
}

////////////////////////////////////////////////////////////

squirrel_script_game::squirrel_script_game(field& f, game_info info, scripting::squirrel::table tab)
    : script_game {f, std::move(info), std::move(tab)}
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

    (void)script.run_file("main.nut");
}

} // namespace solitaire
