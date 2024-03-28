-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local bakers_dozen                     = {
    Info             = {
        Name          = "Baker's Dozen",
        Family        = "BakersDozen",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation       = {
        Size   = 4,
        Create = { Rule = rules.ace_upsuit_top }
    },
    Tableau          = {
        Size   = 13,
        Create = {
            Initial = ops.Initial.face_up(4),
            Layout = "Column",
            Rule = rules.none_downrank_top
        }
    },
    on_after_shuffle = ops.Shuffle.kings_to_bottom,
    on_piles_created = Sol.Layout.bakers_dozen
}

------

local bakers_dozen_two_decks           = Sol.copy(bakers_dozen)
bakers_dozen_two_decks.Info.Name       = "Baker's Dozen (2 Decks)"
bakers_dozen_two_decks.Info.DeckCount  = 2
bakers_dozen_two_decks.Foundation.Size = 8
bakers_dozen_two_decks.Tableau.Size    = 26

------

local good_measure                     = Sol.copy(bakers_dozen)
good_measure.Info.Name                 = "Good Measure"
good_measure.Tableau                   = {
    Size   = 10,
    Create = {
        Initial = ops.Initial.face_up(5),
        Layout = "Column",
        Rule = rules.none_downrank_top
    }
}
good_measure.on_before_shuffle         = function(game, card)
    if card.Rank == "Ace" then
        return game.PlaceTop(card, game.Foundation, 1, 2, true)
    end

    return false
end

------

local fifteen                          = {
    Info             = {
        Name          = "Fifteen",
        Family        = "BakersDozen",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation       = {
        Size   = 8,
        Create = { Rule = rules.ace_upsuit_top }
    },
    Tableau          = {
        Size   = 15,
        Create = function(i)
            return {
                Initial = ops.Initial.face_up(i ~= 14 and 7 or 6),
                Layout = "Column",
                Rule = rules.any_updownsuit_top
            }
        end
    },
    on_piles_created = Sol.Layout.canister
}

------

local nationale                        = {
    Info              = {
        Name          = "Nationale",
        Family        = "BakersDozen",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation        = {
        Size   = 8,
        Create = function(i)
            if i < 4 then
                return { Rule = rules.ace_upsuit_top }
            else
                return { Rule = rules.king_downsuit_top }
            end
        end
    },
    Tableau           = {
        Size   = 12,
        Create = {
            Initial = ops.Initial.face_up(8),
            Layout = "Column",
            Rule = rules.any_updownsuit_top
        }
    },
    on_before_shuffle = function(game, card)
        if card.Rank == "Ace" then
            return game.PlaceTop(card, game.Foundation, 1, 4, true)
        end
        if card.Rank == "King" then
            return game.PlaceTop(card, game.Foundation, 5, 4, true)
        end

        return false
    end,
    on_piles_created  = Sol.Layout.canister
}

------

------------------------

Sol.register_game(bakers_dozen)
Sol.register_game(bakers_dozen_two_decks)
Sol.register_game(fifteen)
Sol.register_game(good_measure)
Sol.register_game(nationale)
