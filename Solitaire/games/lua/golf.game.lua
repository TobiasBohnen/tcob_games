-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local layout = require 'base/layout'
local ops    = require 'base/ops'
local piles  = require 'base/piles'
require 'base/common'

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
        create = function()
            return {
                Initial = piles.initial.top_face_up(5),
                Layout = "Column",
                Rule = { Build = "NoBuilding", Move = "Top" }
            }
        end
    },
    deal        = function(game)
        return game.Stock[1]:deal(game.Foundation[1], game.CardDealCount)
    end,
    check_state = function(game)
        return game:check_state("Golf")
    end,
    layout      = layout.golf
}

------

local double_golf          = Copy(golf)
double_golf.Info.Name      = "Double Golf"
double_golf.Info.DeckCount = 2
double_golf.Stock.Initial  = piles.initial.face_down(40)
double_golf.Tableau        = {
    Size   = 9,
    create = function()
        return {
            Initial = piles.initial.top_face_up(7),
            Layout = "Column",
            Rule = { Build = "NoBuilding", Move = "Top" }
        }
    end
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
    { 0, 1 }, { 2, 1 }, { 6, 1 }, { 8, 1 },
    { 0, 2 }, { 2, 2 }, { 6, 2 }, { 8, 2 },
    { 0, 3 }, { 2, 3 }, { 4, 3 }, { 6, 3 }
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
            return game.put_card(card, game.Foundation)
        end

        return false
    end,
    check_state    = function(game)
        return game:check_state("Golf")
    end
}

------------------------

register_game(golf)
register_game(double_golf)
register_game(putt_putt)
register_game(double_putt)
register_game(black_hole)
