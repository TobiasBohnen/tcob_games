-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local needle = {
    Info       = {
        Name      = "Needle",
        Family    = "FreeCell",
        DeckCount = 1
    },
    FreeCell   = {
        Initial = Sol.Initial.face_up(8),
        Layout  = "Row",
        Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.Any(), Move = Sol.Rules.Move.Top(), Limit = 18 }
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 9,
        Pile = function(i)
            local count = 0
            if i < 2 or i > 6 then
                count = 8
            elseif i == 2 or i == 6 then
                count = 4
            elseif i == 3 or i == 5 then
                count = 2
            end

            return {
                Initial = Sol.Initial.face_up(count),
                Layout  = "Column",
                Rule    = Sol.Rules.any_downac_top
            }
        end
    },
    on_init    = Sol.Layout.free_cell
}


------

local haystack = Sol.copy(needle)
haystack.Info.Name = "Haystack"
haystack.FreeCell.Rule.Limit = 8


------

local pitchfork         = Sol.copy(needle)
pitchfork.Info.Name     = "Pitchfork"
pitchfork.FreeCell.Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.None(), Move = Sol.Rules.Move.Top() }


------

------------------------

Sol.register_game(needle)
Sol.register_game(haystack)
Sol.register_game(pitchfork)
