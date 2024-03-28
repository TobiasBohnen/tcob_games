-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local fastness               = {
    Info             = {
        Name          = "Fastness",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell         = {
        Size   = 2,
        Create = { Rule = rules.any_none_top }
    },
    Foundation       = {
        Size   = 4,
        Create = { Rule = rules.ace_upsuit_none }
    },
    Tableau          = {
        Size   = 8,
        Create = function(i)
            return {
                Initial = ops.Initial.face_up(i < 4 and 7 or 6),
                Layout = "Row",
                Rule = rules.any_downrank_top
            }
        end
    },
    on_piles_created = Sol.Layout.fastness
}

------

local siegecraft             = Sol.copy(fastness)
siegecraft.Info.Name         = "Siegecraft"
siegecraft.FreeCell          = { Rule = rules.any_none_top }
siegecraft.Tableau.Create    = {
    Initial = ops.Initial.face_up(6),
    Layout = "Row",
    Rule = rules.any_downrank_top
}
siegecraft.on_before_shuffle = ops.Shuffle.ace_to_foundation

------

local stronghold             = Sol.copy(fastness)
stronghold.Info.Name         = "Stronghold"
stronghold.FreeCell          = { Rule = rules.any_none_top }

------------

Sol.register_game(fastness)
Sol.register_game(siegecraft)
Sol.register_game(stronghold)
