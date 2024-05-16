-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local gate = {
    Info        = {
        Name      = "Gate",
        Family    = "Canfield",
        DeckCount = 1
    },
    Stock       = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(34)
    },
    Waste       = { Position = { x = 1, y = 0 } },
    Reserve     = {
        Size = 2,
        Pile = function(i)
            return {
                Position = { x = i == 0 and 0 or 10, y = 1 },
                Initial = Sol.Initial.face_up(5),
                Layout = Sol.Pile.Layout.Column
            }
        end
    },
    Foundation  = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 2.5, y = 0 },
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau     = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 1.5, y = 1 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.any_downac_inseq
            }
        end
    },
    on_end_turn = function(game)
        local reserve = game.Reserve
        if reserve[1].IsEmpty and reserve[2].IsEmpty then
            Sol.Ops.Deal.to_group(game.Waste[1], game.Tableau, Sol.DealMode.IfEmpty)
        else
            if reserve[1].CardCount >= reserve[2].CardCount then
                Sol.Ops.Deal.to_group(game.Reserve[1], game.Tableau, Sol.DealMode.IfEmpty)
            else
                Sol.Ops.Deal.to_group(game.Reserve[2], game.Tableau, Sol.DealMode.IfEmpty)
            end
        end
    end,
    deal        = Sol.Ops.Deal.stock_to_waste
}


------

local little_gate         = Sol.copy(gate)
little_gate.Info.Name     = "Little Gate"
little_gate.Stock.Initial = Sol.Initial.face_down(38)
little_gate.Reserve.Pile  = function(i)
    return {
        Position = { x = i == 0 and 0 or 6, y = 1 },
        Initial = Sol.Initial.face_up(5),
        Layout = Sol.Pile.Layout.Column
    }
end
little_gate.Tableau.Size  = 4


------

------------------------

Sol.register_game(gate)
Sol.register_game(little_gate)
