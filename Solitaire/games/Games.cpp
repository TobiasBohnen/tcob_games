#include "Games.hpp"

#include "Field.hpp"
#include "StartScene.hpp"

#include <ranges>
#include <utility>

namespace solitaire::games {

using namespace tcob::data::config;

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

auto base_game::get_description(pile const* pile) const -> hover_info
{
    if (!pile) { return {}; }

    return pile->get_description(_remainingRedeals);
}

void base_game::start(size_f cardSize, std::optional<data::config::object> const& loadObj)
{
    _cardSize = cardSize;

    _state = game_state::Initial;

    if (!load(loadObj)) {
        new_game();
    }

    init();
}

void base_game::restart()
{
    start(_cardSize, _currentState);
}

void base_game::new_game()
{
    clear_pile_cards();

    std::vector<card> cards;
    for (i32 i {0}; i < _gameInfo.DeckCount; ++i) {
        deck const deck {deck::GetShuffled(_rand, static_cast<u8>(i))};
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
                if (!shuffle(card, pile->Type)) {
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
    if (!loadObj) { return false; }

    clear_pile_cards();

    if (!loadObj->has(_gameInfo.Name)) { return false; }

    object const obj {loadObj->get<object>(_gameInfo.Name).value()};
    if (!obj.has("Redeals") || !obj.has("Turn")) { return false; }
    _remainingRedeals = obj["Redeals"];
    _turn             = obj["Turn"];

    auto const createCard {[&](entry const& entry) { return card::FromValue(entry.as<u16>()); }};
    for (auto& [type, piles] : _piles) {
        auto const pileType {get_pile_type_name(type)};
        if (!obj.has(pileType)) { return false; }

        array const typeArray {obj[pileType]};
        auto const  pileCount {std::ssize(piles)};

        if (typeArray.get_size() != pileCount) { return false; }

        for (isize i {0}; i < pileCount; ++i) {
            pile*       pile {piles[i]};
            array const pileArray {typeArray[i]};
            for (auto const& cardEntry : pileArray) {
                auto card {createCard(cardEntry)};
                pile->Cards.emplace_back(card);
            }
        }
    }

    return true;
}

void base_game::save(object& saveObj)
{
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

void base_game::layout_piles()
{
    on_change();

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
    _state = check_state();
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
    std::array<point_i, 4> points {point_i {rect.top_left()}, point_i {rect.top_right()}, point_i {rect.bottom_left()}, point_i {rect.bottom_right()}};

    std::vector<hit_test_result> candidates;
    for (auto const& point : points) {
        if (auto target {get_pile_at(point, true)};
            target.Pile
            && target.Index == std::ssize(target.Pile->Cards) - 1
            && can_drop(*target.Pile, target.Index, move, numCards)) {
            candidates.push_back(target);
        }
    }

    if (candidates.empty()) { return {}; }
    if (candidates.size() == 1) { return candidates[0]; }

    f32             maxArea {0};
    hit_test_result retValue;
    for (auto const& candidate : candidates) {
        auto const interSect {rect.as_intersection(candidate.Index == INDEX_MARKER ? candidate.Pile->Marker->Bounds : candidate.Pile->Cards[candidate.Index].Bounds)};
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

auto base_game::drop(pile& to, card& card) const -> bool
{
    if (can_drop(to, std::ssize(to.Cards) - 1, card, 1)) {
        card.flip_face_up();
        to.Cards.emplace_back(card);
        return true;
    }

    return false;
}

void base_game::click(pile* srcPile, u8 clicks)
{
    if (!srcPile || srcPile->Type == pile_type::Foundation) { return; }

    if (srcPile->Type == pile_type::Stock) {
        // deal card
        srcPile->remove_tint();
        deal_cards();
        srcPile->tint_cards(COLOR_HOVER, std::ssize(srcPile->Cards) - 1);
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
            if (!can_drop(fou, std::ssize(fou.Cards) - 1, card, 1)) { continue; }

            srcPile.move_cards(fou, std::ssize(srcPile.Cards) - 1, 1, false);
            end_turn();
            auto_deal(srcPile);
            srcPile.remove_tint();
            fou.remove_tint();
            return;
        }
    }
}

void base_game::auto_deal(pile& from)
{
    if (from.Type == pile_type::Waste && from.empty()) {
        deal_cards();
    }
}

void base_game::drop_cards(hit_test_result const& hovered, hit_test_result const& dropTarget)
{
    if (hovered.Pile) { hovered.Pile->remove_tint(); }
    if (dropTarget.Pile) { dropTarget.Pile->remove_tint(); }

    if (dropTarget.Pile && hovered.Pile) {
        hovered.Pile->move_cards(*dropTarget.Pile, hovered.Index, std::ssize(hovered.Pile->Cards) - hovered.Index, false);
        end_turn();
        auto_deal(*hovered.Pile);
    } else {
        layout_piles();
    }
}

void base_game::clear_pile_cards()
{
    for (auto& kvp : _piles) {
        for (auto* pile : kvp.second) {
            pile->Cards.clear();
        }
    }
}

auto base_game::before_shuffle(card& /* card */) -> bool
{
    return false;
}

auto base_game::shuffle(card& /* card */, pile_type /*pile*/) -> bool
{
    return false;
}

void base_game::after_shuffle()
{
}

auto base_game::can_drop(pile const& targetPile, isize targetIndex, card const& drop, isize numCards) const -> bool
{
    return rules::build(targetPile, targetIndex, drop, numCards);
}

auto base_game::check_movable(pile const& targetPile, isize idx) const -> bool
{
    switch (targetPile.Rule.Move) {
    case move_type::None:
    case move_type::Top: {
        if (rules::stack::top(targetPile, idx)) { return true; }
    } break;
    case move_type::TopOrPile: {
        if (rules::stack::top_or_pile(targetPile, idx)) { return true; }
    } break;
    case move_type::FaceUp: {
        if (rules::stack::face_up(targetPile, idx)) { return true; }
    } break;
    case move_type::InSequence: {
        if (rules::stack::in_seq(this, targetPile, idx)) { return true; }
    } break;
    case move_type::InSequenceInSuit: {
        if (rules::stack::in_seq_in_suit(this, targetPile, idx)) { return true; }
    } break;
    case move_type::InSequenceInSuitOrSameRank: {
        if (rules::stack::in_seq_in_suit_same_rank(this, targetPile, idx)) { return true; }
    } break;
    case move_type::SuperMove: {
        if (rules::stack::super_move(this, targetPile, idx)) { return true; }
    } break;
    }

    return false;
}

auto base_game::deal_cards() -> bool
{
    if (_remainingRedeals != 0) {
        // e.g. Waste -> Stock
        if (do_redeal()) {
            if (_remainingRedeals > 0) {
                --_remainingRedeals;
            }

            end_turn();
        }
    }

    if (do_deal()) {
        end_turn();
        return true;
    }

    return false;
}

auto base_game::do_redeal() -> bool
{
    return false;
}

auto base_game::do_deal() -> bool
{
    return false;
}

void base_game::on_change()
{
}

auto base_game::check_state() const -> game_state
{
    // success if cards only on foundation piles
    for (auto const& kvp : _piles) {
        if (kvp.first != pile_type::Foundation) {
            for (auto const& pile : kvp.second) {
                if (!pile->empty()) { return game_state::Running; }
            }
        }
    }

    // TODO: detect dead game
    return game_state::Success;
}

void base_game::add_pile(pile* p)
{
    _piles[p->Type].push_back(p);
}

auto base_game::find_pile(card const& card) const -> pile*
{
    for (auto const& kvp : _piles) {
        for (auto* pile : kvp.second) {
            for (auto const& c : pile->Cards) {
                if (c == card) { return pile; }
            }
        }
    }

    return nullptr;
}

void base_game::end_turn()
{
    ++_turn;

    layout_piles();

    _undoStack.push(_currentState);
    _currentState = {};
    save(_currentState);
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
using namespace scripting::lua;

script_game::script_game(field& f, game_info info, scripting::lua::table tab)
    : base_game {f, std::move(info)}
    , _table {std::move(tab)}
{
    // TODO: check table for unknown members

    auto const createPile {[this](pile& pile, table const& pileTab) {
        pile.Position  = pileTab["Position"].get<point_f>().value_or(point_f::Zero);
        pile.Initial   = pileTab["Initial"].get<std::vector<bool>>().value_or(std::vector<bool> {});
        pile.Layout    = pileTab["Layout"].get<layout_type>().value_or(layout_type::Squared);
        pile.HasMarker = pileTab["HasMarker"].get<bool>().value_or(true);

        if (table ruleTable; pileTab.try_get(ruleTable, "Rule")) {
            pile.Rule.Build    = ruleTable["Build"].get<build_type>().value_or(build_type::NoBuilding);
            pile.Rule.Interval = ruleTable["Interval"].get<i32>().value_or(1);
            pile.Rule.Wrap     = ruleTable["Wrap"].get<bool>().value_or(false);
            pile.Rule.Move     = ruleTable["Move"].get<move_type>().value_or(move_type::Top);
            pile.Rule.Limit    = ruleTable["Limit"].get<i32>().value_or(-1);

            if (function<bool> emptyFunc; ruleTable.try_get(emptyFunc, "Empty")) {
                pile.Rule.Empty = empty::func {[this, emptyFunc](card const& card) { return emptyFunc(this, card); }};
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
                    pile.Rule.Empty = empty::First(Foundation[0]);
                }
            } else if (table emptyTable; ruleTable.try_get(emptyTable, "Empty")) {
                if (std::string type; emptyTable.try_get(type, "Type")) {
                    if (type == "FirstFoundation") {
                        i32 const interval {emptyTable["Interval"].get<i32>().value_or(0)};
                        pile.Rule.Empty = {empty::First(Foundation[0], interval)};
                    } else if (type == "Card") {
                        rank const       r {emptyTable["Rank"]};
                        suit_color const sc {emptyTable["Color"]};
                        pile.Rule.Empty = empty::Card(sc, r);
                    } else if (type == "Ranks") {
                        std::set<rank> const r {emptyTable["Ranks"].as<std::set<rank>>()};
                        pile.Rule.Empty = empty::Ranks(r);
                    } else if (type == "Suits") {
                        std::set<suit> const s {emptyTable["Suits"].as<std::set<suit>>()};
                        pile.Rule.Empty = empty::Suits(s);
                    }
                }
            }
        }
    }};

    auto const createPiles {[&](auto&& piles, std::string const& name) {
        if (table pileTypeTable; _table.try_get(pileTypeTable, name)) {
            isize const size {pileTypeTable["Size"].get<isize>().value_or(1)};
            if (size == 1 && !pileTypeTable.has("create")) { // table is definition
                create_piles(piles, 1, [&](auto& pile, i32) {
                    createPile(pile, pileTypeTable);
                });
            } else if (table createTable; pileTypeTable.try_get(createTable, "create")) { // use 'create' table
                create_piles(piles, size, [&](auto& pile, i32) {
                    createPile(pile, createTable);
                });
            } else { // call 'create' function
                function<table> create {pileTypeTable["create"].as<function<table>>()};
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

    if (function<void> func; _table.try_get(func, "on_created")) {
        func(this);
    }
}

auto script_game::can_drop(pile const& targetPile, isize targetIndex, card const& drop, isize numCards) const -> bool
{
    if (function<bool> func; _table.try_get(func, "check_drop")) {
        return func(this, &targetPile, targetIndex + 1, drop, numCards);
    }
    return base_game::can_drop(targetPile, targetIndex, drop, numCards);
}

auto script_game::do_redeal() -> bool
{
    if (function<bool> func; _table.try_get(func, "on_redeal")) {
        return func(this);
    }
    return base_game::do_redeal();
}

auto script_game::do_deal() -> bool
{
    if (function<bool> func; _table.try_get(func, "on_deal")) {
        return func(this);
    }
    return base_game::do_deal();
}

auto script_game::before_shuffle(card& card) -> bool
{
    if (function<bool> func; _table.try_get(func, "on_before_shuffle")) {
        return func(this, card);
    }
    return base_game::before_shuffle(card);
}

auto script_game::shuffle(card& card, pile_type pileType) -> bool
{
    if (function<bool> func; _table.try_get(func, "on_shuffle")) {
        return func(this, card, pileType);
    }
    return base_game::shuffle(card, pileType);
}

void script_game::after_shuffle()
{
    if (function<void> func; _table.try_get(func, "on_after_shuffle")) {
        func(this);
    } else {
        base_game::after_shuffle();
    }
}

void script_game::on_change()
{
    if (function<void> func; _table.try_get(func, "on_change")) {
        func(this);
    } else {
        base_game::on_change();
    }
}

auto script_game::check_state() const -> game_state
{
    if (function<game_state> func; _table.try_get(func, "check_state")) {
        return func(this);
    }
    return base_game::check_state();
}

auto script_game::check_movable(pile const& targetPile, isize idx) const -> bool
{
    if (function<bool> func; _table.try_get(func, "check_movable")) {
        return func(this, &targetPile, idx + 1);
    }
    return base_game::check_movable(targetPile, idx);
}

void script_game::CreateAPI(start_scene* scene, scripting::lua::script& script, std::vector<scripting::lua::native_closure_shared_ptr>& funcs)
{
    script.open_libraries();

    auto make_func {[&](auto&& func) {
        auto ptr {make_shared_closure(std::function {func})};
        funcs.push_back(ptr);
        return ptr.get();
    }};

    auto global {script.get_global_table()};
    global["RegisterGame"] = make_func([scene](table& tab) {
        games::game_info info;
        info.Name          = tab["Info"]["Name"].as<std::string>();
        info.Type          = tab["Info"]["Type"];
        info.Family        = tab["Info"]["Family"];
        info.DeckCount     = tab["Info"]["DeckCount"];
        info.CardDealCount = tab["Info"]["CardDealCount"];
        info.Redeals       = tab["Info"]["Redeals"];

        auto func {[tab, info](auto& field) { return std::make_shared<games::script_game>(field, info, tab); }};
        scene->register_game(info, func);
    });

    // game
    auto& gameWrapper {*script.create_wrapper<script_game>("script_game")};
    gameWrapper["RedealsLeft"]   = getter {[](script_game* game) { return game->redeals_left(); }};
    gameWrapper["CardDealCount"] = getter {[](script_game* game) { return game->info().CardDealCount; }};

    auto const returnPile {[](script_game* game, pile_type type) {
        auto const& piles {game->piles()};
        if (piles.contains(type)) { return game->piles().at(type); }
        return std::vector<pile*> {};
    }};

    // properties
    gameWrapper["Stock"]      = getter {[returnPile](script_game* game) { return returnPile(game, pile_type::Stock); }};
    gameWrapper["Waste"]      = getter {[returnPile](script_game* game) { return returnPile(game, pile_type::Waste); }};
    gameWrapper["Foundation"] = getter {[returnPile](script_game* game) { return returnPile(game, pile_type::Foundation); }};
    gameWrapper["Tableau"]    = getter {[returnPile](script_game* game) { return returnPile(game, pile_type::Tableau); }};
    gameWrapper["Reserve"]    = getter {[returnPile](script_game* game) { return returnPile(game, pile_type::Reserve); }};
    gameWrapper["FreeCell"]   = getter {[returnPile](script_game* game) { return returnPile(game, pile_type::FreeCell); }};

    // methods
    gameWrapper["shuffle_cards"] = [](script_game* game, std::vector<card> const& cards) {
        std::vector<card> shuffled {cards};
        game->rand().shuffle<card>(shuffled);
        return shuffled;
    };
    gameWrapper["find_pile"] = [](script_game* game, card& card) { return game->find_pile(card); };
    gameWrapper["drop"]      = [](script_game* game, pile* to, card& card) { return game->drop(*to, card); };
    gameWrapper["can_drop"]  = [](script_game* game, pile* targetPile, isize targetIndex, card const& drop, isize numCards) {
        return game->base_game::can_drop(*targetPile, targetIndex - 1, drop, numCards);
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
            auto const target {std::span<pile*>(to.data() + offset - 1, size)};
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
            auto const target {std::span<pile*>(to.data() + offset - 1, size)};
            for (auto& pile : target) {
                if (placeCard(card, *pile, ifEmpty, true)) { return true; }
            }
            return false;
        },
        [=](card& card, pile* to, bool ifEmpty) {
            return placeCard(card, *to, ifEmpty, true);
        });

    // pile
    auto& pileWrapper {*script.create_wrapper<pile>("pile")};

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
        p->move_cards(*to, startIndex - 1, numCards, reverse);
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

            if (!p->Cards.empty()) {
                p->move_cards(*toPile, std::ssize(p->Cards) - 1, 1, false);
            } else {
                break;
            }
            toPile->flip_up_top_card();
        }

        return true;
    };
    pileWrapper["check_bounds"] = [](pile* p, isize i, point_i pos) { return p->Cards[i - 1].Bounds.contains(pos); };
}

} // namespace solitaire
