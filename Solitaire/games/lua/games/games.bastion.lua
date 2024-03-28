-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local bastion               = {
    Info             = {
        Name          = "Bastion",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell         = {
        Size = 2,
        Pile = {
            Initial = ops.Initial.face_up(1),
            Rule = rules.any_none_top
        }
    },
    Foundation       = {
        Size = 4,
        Pile = { Rule = rules.ace_upsuit_top }
    },
    Tableau          = {
        Size = 10,
        Pile = {
            Initial = ops.Initial.face_up(5),
            Layout = "Column",
            Rule = rules.any_updownsuit_top
        }
    },
    on_piles_created = Sol.Layout.free_cell
}

------

local castles_end           = Sol.copy(bastion)
castles_end.Info.Name       = "Castles End"
castles_end.Foundation.Pile = { Rule = rules.ff_upsuit_top }
castles_end.Tableau.Pile    = {
    Initial = ops.Initial.face_up(5),
    Layout = "Column",
    Rule = rules.any_updownac_top
}
castles_end.check_playable  = function(game, targetPile, targetIndex, drop, numCards)
    local foundation1 = game.Foundation[1]
    if foundation1.IsEmpty then -- block card drops if foundation is empty
        return targetPile == foundation1
    end

    return game:can_play(targetPile, targetIndex, drop, numCards)
end

------

local ten_by_one            = Sol.copy(bastion)
ten_by_one.Info.Name        = "Ten by One"
ten_by_one.FreeCell         = { Rule = rules.any_none_top }
ten_by_one.Tableau.Pile     = function(i)
    return {
        Initial = ops.Initial.face_up(i < 2 and 6 or 5),
        Layout = "Column",
        Rule = rules.any_updownsuit_top
    }
end

------

------------

Sol.register_game(bastion)
Sol.register_game(castles_end)
Sol.register_game(ten_by_one)
