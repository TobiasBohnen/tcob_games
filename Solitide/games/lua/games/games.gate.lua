-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local gate                = {
    Info        = {
        Name          = "Gate",
        Family        = "Canfield",
        DeckCount     = 1,
        CardDealCount = 1,
        Redeals       = 0
    },
    Stock       = {
        Position = { x = 0, y = 0 },
        Initial = ops.Initial.face_down(34)
    },
    Waste       = { Position = { x = 1, y = 0 } },
    Reserve     = {
        Size = 2,
        Pile = function(i)
            return {
                Position = { x = i == 0 and 0 or 10, y = 1 },
                Initial = ops.Initial.face_up(5),
                Layout = "Column"
            }
        end
    },
    Foundation  = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 2.5, y = 0 },
                Rule = rules.ace_upsuit_top
            }
        end
    },
    Tableau     = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 1.5, y = 1 },
                Initial = ops.Initial.face_up(1),
                Layout = "Column",
                Rule = rules.any_downac_inseq
            }
        end
    },
    on_end_turn = function(game)
        local reserve = game.Reserve
        if reserve[1].IsEmpty and reserve[2].IsEmpty then
            ops.Deal.to_group(game.Waste[1], game.Tableau, true)
        else
            if reserve[1].CardCount > reserve[2].CardCount then
                ops.Deal.to_group(game.Reserve[1], game.Tableau, true)
            else
                ops.Deal.to_group(game.Reserve[2], game.Tableau, true)
            end
        end
    end,
    on_deal     = ops.Deal.stock_to_waste
}

------

local little_gate         = Sol.copy(gate)
little_gate.Info.Name     = "Little Gate"
little_gate.Stock.Initial = ops.Initial.face_down(38)
little_gate.Reserve.Pile  = function(i)
    return {
        Position = { x = i == 0 and 0 or 6, y = 1 },
        Initial = ops.Initial.face_up(5),
        Layout = "Column"
    }
end
little_gate.Tableau.Size  = 4

------

------------------------

Sol.register_game(gate)
Sol.register_game(little_gate)
