-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local function golf_check_state(game)
    local foundation = game.Foundation
    if foundation[1].IsEmpty then return "Running" end

    local dead    = true -- detect dead game
    local success = true -- detect win state
    for _, tableau in ipairs(game.Tableau) do
        if not tableau.IsEmpty then
            success = false
            if game:can_play(foundation[1], #foundation[1].Cards - 1, tableau.Cards[#tableau.Cards], 1) then
                dead = false
                break
            end
        end
    end


    if success then return "Success" end
    if not game.Stock[1].IsEmpty then return "Running" end
    if dead then return "Failure" end

    return "Running"
end

------

local golf                       = {
    Info             = {
        Name          = "Golf",
        Family        = "Golf",
        DeckCount     = 1,
        CardDealCount = 1,
        Redeals       = 0
    },
    Stock            = {
        Initial = ops.Initial.face_down(16)
    },
    Foundation       = {
        Initial = ops.Initial.face_up(1),
        Layout  = "Squared",
        Rule    = { Base = rules.Base.None, Build = rules.Build.UpOrDownByRank(), Move = rules.Move.None() }
    },
    Tableau          = {
        Size = 7,
        Pile = {
            Initial = ops.Initial.top_face_up(5),
            Layout = "Column",
            Rule = { Base = rules.Base.None, Build = rules.Build.None(), Move = rules.Move.Top() }
        }
    },
    on_deal          = function(game)
        return ops.Deal.to_pile(game.Stock[1], game.Foundation[1], game.CardDealCount)
    end,
    check_state      = golf_check_state,
    on_piles_created = Sol.Layout.golf
}

------

local double_golf                = Sol.copy(golf)
double_golf.Info.Name            = "Double Golf"
double_golf.Info.DeckCount       = 2
double_golf.Stock.Initial        = ops.Initial.face_down(40)
double_golf.Tableau.Size         = 9
double_golf.Tableau.Pile.Initial = ops.Initial.top_face_up(7)

------

local putt_putt                  = Sol.copy(golf)
putt_putt.Info.Name              = "Putt Putt"
putt_putt.Foundation             = {
    Initial = ops.Initial.face_up(1),
    Layout  = "Squared",
    Rule    = { Base = rules.Base.None, Build = rules.Build.UpOrDownByRank(true), Move = rules.Move.None() }
}

------

local double_putt                = Sol.copy(putt_putt)
double_putt.Info.Name            = "Double Putt"
double_putt.Info.DeckCount       = 2
double_putt.Stock.Initial        = ops.Initial.face_down(40)
double_putt.Tableau.Size         = 9
double_putt.Tableau.Pile.Initial = ops.Initial.top_face_up(7)

------
local black_hole_pos             = {
    { 0, 0 }, { 2, 0 }, { 4, 0 }, { 6, 0 }, { 8, 0 },
    { 0, 1 }, { 2, 1 }, --[[ --]] { 6, 1 }, { 8, 1 },
    { 0, 2 }, { 2, 2 }, --[[ --]] { 6, 2 }, { 8, 2 },
    { 0, 3 }, { 2, 3 }, { 4, 3 }, { 6, 3 } --[[ --]]
}

local black_hole                 = {
    Info              = {
        Name          = "Black Hole",
        Family        = "Golf",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation        = {
        Position = { x = 4, y = 1.5 },
        Rule     = { Build = rules.Build.UpOrDownByRank(true), Move = rules.Move.None() }
    },
    Tableau           = {
        Size = 17,
        Pile = function(i)
            return {
                Position = { x = black_hole_pos[i + 1][1], y = black_hole_pos[i + 1][2] },
                Initial = ops.Initial.face_up(3),
                Layout = "Row",
                Rule = { Build = rules.Build.None() }
            }
        end
    },
    on_before_shuffle = function(game, card)
        if card.Rank == "Ace" and card.Suit == "Spades" then
            return game.PlaceTop(card, game.Foundation, true)
        end

        return false
    end
}

------------------------

Sol.register_game(golf)
Sol.register_game(black_hole)
Sol.register_game(double_golf)
Sol.register_game(double_putt)
Sol.register_game(putt_putt)
