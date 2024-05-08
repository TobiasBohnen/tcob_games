-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

------

local take_away             = {
    Info       = {
        Name      = "Take Away",
        Family    = "Golf",
        DeckCount = 1
    },
    Foundation = {
        Size = 6,
        Pile = {
            Layout = "Squared",
            Rule   = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.UpOrDownByRank(true), Move = Sol.Rules.Move.None() }
        }
    },
    Tableau    = {
        Size = 4,
        Pile = {
            Initial = Sol.Initial.face_up(13),
            Layout = "Column",
            Rule = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.None(), Move = Sol.Rules.Move.Top() }
        }
    },
    on_init    = Sol.Layout.canister
}

------

local take_away_8           = Sol.copy(take_away)
take_away_8.Info.Name       = "Take Away 8"
take_away_8.Foundation.Size = 8

------

------------------------

Sol.register_game(take_away)
Sol.register_game(take_away_8)
