-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local capricieuse              = {
    Info              = {
        Name          = "Capricieuse",
        Family        = "BakersDozen",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 2
    },
    Stock             = {},
    Foundation        = {
        Size = 8,
        Pile = function(i)
            if i < 4 then
                return { Rule = rules.ace_upsuit_top }
            else
                return { Rule = rules.king_downsuit_top }
            end
        end
    },
    Tableau           = {
        Size = 12,
        Pile = {
            Initial = ops.Initial.face_up(8),
            Layout = "Column",
            Rule = rules.any_updownsuit_top
        }
    },
    on_redeal         = function(game)
        local cards = Sol.shuffle_tableau(game)
        if #cards == 0 then return false end

        local tableau = game.Tableau
        local tabIdx = 1
        while #cards > 0 do
            game.PlaceTop(table.remove(cards), tableau[tabIdx], false)
            tabIdx = tabIdx + 1
            if tabIdx > #tableau then tabIdx = 1 end
        end

        return true
    end,
    on_before_shuffle = function(game, card)
        if card.Rank == "Ace" then
            return game.PlaceTop(card, game.Foundation, 1, 4, true)
        end
        if card.Rank == "King" then
            return game.PlaceTop(card, game.Foundation, 5, 4, true)
        end

        return false
    end,
    on_piles_created  = Sol.Layout.capricieuse
}

------

local strata                   = Sol.copy(capricieuse)
strata.Info.Name               = "Strata"
strata.Foundation.Pile         = { Rule = rules.ace_upsuit_top }
strata.Info.DeckRanks          = { "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King", "Ace" }
strata.Tableau                 = {
    Size = 8,
    Pile = {
        Initial = ops.Initial.face_up(8),
        Layout = "Column",
        Rule = rules.any_downac_top
    }
}
strata.on_before_shuffle       = nil

------

local cruel                    = Sol.copy(capricieuse)
cruel.Info.Name                = "Cruel"
cruel.Info.DeckCount           = 1
cruel.Info.Redeals             = -1
cruel.Foundation               = {
    Size = 4,
    Pile = { Rule = rules.ace_upsuit_top }
}
cruel.Tableau                  = {
    Size = 12,
    Pile = {
        Initial = ops.Initial.face_up(4),
        Layout = "Column",
        Rule = rules.none_downsuit_top
    }
}
cruel.on_before_shuffle        = ops.Shuffle.ace_to_foundation
cruel.on_redeal                = function(game)
    local cards = {}

    local tableau = game.Tableau
    for _, tab in ipairs(tableau) do
        for j = #tab.Cards, 1, -1 do
            cards[#cards + 1] = tab.Cards[j]
        end
        tab:clear()
    end

    if #cards == 0 then return false end

    for _, tab in ipairs(tableau) do
        for _ = 1, 4 do
            game.PlaceBottom(table.remove(cards, 1), tab, false)
        end
        if #cards == 0 then break end
    end

    return true
end
cruel.on_piles_created         = Sol.Layout.bakers_dozen

------

local indefatigable            = Sol.copy(cruel)
indefatigable.Info.Name        = "Indefatigable"
indefatigable.Info.Redeals     = 2
indefatigable.Tableau.Pile     = {
    Initial = ops.Initial.face_up(4),
    Layout = "Column",
    Rule = rules.any_updownsuit_top
}

------

local perseverance             = Sol.copy(cruel)
perseverance.Info.Name         = "Perseverance"
perseverance.Info.Redeals      = 2
perseverance.Tableau.Pile      = {
    Initial = ops.Initial.face_up(4),
    Layout = "Column",
    Rule = { Base = rules.Base.None(), Build = rules.Build.DownInSuit(), Move = rules.Move.InSeq() }
}

------

local royal_family             = Sol.copy(cruel)
royal_family.Info.Name         = "Royal Family"
royal_family.Info.Redeals      = 1
royal_family.Foundation.Pile   = { Rule = rules.king_downsuit_top }
royal_family.Tableau.Pile      = {
    Initial = ops.Initial.face_up(4),
    Layout = "Column",
    Rule = rules.any_updownac_top
}
royal_family.on_before_shuffle = ops.Shuffle.king_to_foundation

------

local ripple_fan               = Sol.copy(cruel)
ripple_fan.Info.Name           = "Ripple Fan"
ripple_fan.Tableau.Size        = 13
ripple_fan.on_before_shuffle   = nil

------

local unusual                  = Sol.copy(cruel)
unusual.Info.Name              = "Unusual"
unusual.Info.DeckCount         = 2
unusual.Foundation.Size        = 8
unusual.Tableau.Size           = 24

------

------------------------

Sol.register_game(capricieuse)
Sol.register_game(cruel)
Sol.register_game(indefatigable)
Sol.register_game(perseverance)
Sol.register_game(royal_family)
Sol.register_game(ripple_fan)
Sol.register_game(strata)
Sol.register_game(unusual)
