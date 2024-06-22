-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local fastness = {
    Info       = {
        Name      = "Fastness",
        Family    = "BeleagueredCastle",
        DeckCount = 1
    },
    FreeCell   = {
        Size = 2,
        Pile = { Rule = Sol.Rules.any_none_top }
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_none }
    },
    Tableau    = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 4 and 7 or 6),
                Layout  = Sol.Pile.Layout.Row,
                Rule    = Sol.Rules.any_downrank_top
            }
        end
    },
    on_init    = Sol.Layout.fastness
}


------

local siegecraft           = Sol.copy(fastness)
siegecraft.Info.Name       = "Siegecraft"
siegecraft.FreeCell        = { Rule = Sol.Rules.any_none_top }
siegecraft.Tableau.Pile    = {
    Initial = Sol.Initial.face_up(6),
    Layout  = Sol.Pile.Layout.Row,
    Rule    = Sol.Rules.any_downrank_top
}
siegecraft.on_before_setup = Sol.Ops.Shuffle.ace_to_foundation


------

local stronghold     = Sol.copy(fastness)
stronghold.Info.Name = "Stronghold"
stronghold.FreeCell  = { Rule = Sol.Rules.any_none_top }


------

------------

Sol.register_game(fastness)
Sol.register_game(siegecraft)
Sol.register_game(stronghold)
