-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local layout = require 'base/layout'
local ops    = require 'base/ops'
local piles  = require 'base/piles'
require 'base/common'

local function golf_check_state(game)
    if game.Foundation[1].Empty then return "Running" end

    local dead    = true -- detect dead game
    local success = true -- detect win state
    for _, tableau in pairs(game.Tableau) do
        if not tableau.Empty then
            success = false
            if game:can_drop(game.Foundation[1], #game.Foundation[1].Cards - 1, tableau.Cards[#tableau.Cards], 1) then
                dead = false
                break
            end
        end
    end


    if success then return "Success" end
    if not game.Stock[1].Empty then return "Running" end
    if dead then return "Failure" end

    return "Running"
end

------

local golf                 = {
    Info        = {
        Name          = "Golf",
        Type          = "ReservedBuilder",
        Family        = "Golf",
        DeckCount     = 1,
        CardDealCount = 1,
        Redeals       = 0
    },
    Stock       = {
        Initial = piles.initial.face_down(16)
    },
    Foundation  = {
        Initial = piles.initial.face_up(1),
        Layout  = "Squared",
        Rule    = { Build = "UpOrDownByRank", Move = "None" }
    },
    Tableau     = {
        Size   = 7,
        create = {
            Initial = piles.initial.top_face_up(5),
            Layout = "Column",
            Rule = { Build = "NoBuilding", Move = "Top" }
        }
    },
    deal        = function(game)
        return game.Stock[1]:deal(game.Foundation[1], game.CardDealCount)
    end,
    check_state = golf_check_state,
    layout      = layout.golf
}

------

local double_golf          = Copy(golf)
double_golf.Info.Name      = "Double Golf"
double_golf.Info.DeckCount = 2
double_golf.Stock.Initial  = piles.initial.face_down(40)
double_golf.Tableau        = {
    Size   = 9,
    create = {
        Initial = piles.initial.top_face_up(7),
        Layout = "Column",
        Rule = { Build = "NoBuilding", Move = "Top" }
    }
}

------

local putt_putt            = Copy(golf)
putt_putt.Info.Name        = "Putt Putt"
putt_putt.Foundation       = {
    Initial = piles.initial.face_up(1),
    Layout  = "Squared",
    Rule    = { Build = "UpOrDownByRank", Wrap = true, Move = "None" }
}

------

local double_putt          = Copy(putt_putt)
double_putt.Info.Name      = "Double Putt"
double_putt.Info.DeckCount = 2
double_putt.Stock.Initial  = piles.initial.face_down(40)
double_putt.Tableau        = Copy(double_golf.Tableau)

------
local black_hole_pos       = {
    { 0, 0 }, { 2, 0 }, { 4, 0 }, { 6, 0 }, { 8, 0 },
    { 0, 1 }, { 2, 1 }, --[[ --]] { 6, 1 }, { 8, 1 },
    { 0, 2 }, { 2, 2 }, --[[ --]] { 6, 2 }, { 8, 2 },
    { 0, 3 }, { 2, 3 }, { 4, 3 }, { 6, 3 } --[[ --]]
}

local black_hole           = {
    Info           = {
        Name          = "Black Hole",
        Type          = "OpenBuilder",
        Family        = "Golf",
        DeckCount     = 1,
        CardDealCount = 1,
        Redeals       = 0
    },
    Foundation     = {
        Position = { x = 4, y = 1.5 },
        Rule     = { Build = "UpOrDownByRank", Wrap = true, Move = "None" }
    },
    Tableau        = {
        Size   = 17,
        create = function(i)
            return {
                Position = { x = black_hole_pos[i + 1][1], y = black_hole_pos[i + 1][2] },
                Initial = piles.initial.face_up(3),
                Layout = "Row",
                Rule = { Build = "NoBuilding" }
            }
        end
    },
    before_shuffle = function(game, card)
        if card.Rank == "Ace" and card.Suit == "Spades" then
            return game.PlaceTop(card, game.Foundation, true)
        end

        return false
    end,
    check_state    = golf_check_state
}

------------------------

RegisterGame(golf)
RegisterGame(double_golf)
RegisterGame(putt_putt)
RegisterGame(double_putt)
RegisterGame(black_hole)
