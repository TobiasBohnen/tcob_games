-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local canister                 = {
    Info             = {
        Name          = "Canister",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation       = {
        Size = 4,
        Pile = { Rule = rules.ace_upsuit_none }
    },
    Tableau          = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = ops.Initial.face_up((i < 2 or i >= 6) and 6 or 7),
                Layout = "Column",
                Rule = { Base = rules.Base.Any, Build = rules.Build.DownByRank(), Move = rules.Move.InSeq() }
            }
        end
    },
    on_piles_created = Sol.Layout.canister
}

------

local american_canister        = Sol.copy(canister)
american_canister.Info.Name    = "American Canister"
american_canister.Tableau.Pile = function(i)
    return {
        Initial = ops.Initial.face_up((i < 2 or i >= 6) and 6 or 7),
        Layout = "Column",
        Rule = rules.any_downac_inseq
    }
end

------

local british_canister         = Sol.copy(canister)
british_canister.Info.Name     = "British Canister"
british_canister.Tableau.Pile  = function(i)
    return {
        Initial = ops.Initial.face_up((i < 2 or i >= 6) and 6 or 7),
        Layout = "Column",
        Rule = rules.king_downac_top
    }
end

------

------------

Sol.register_game(canister)
Sol.register_game(american_canister)
Sol.register_game(british_canister)
