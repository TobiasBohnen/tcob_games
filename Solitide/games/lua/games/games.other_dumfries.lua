-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local dumfries = {
    Info = {
        Name      = "Dumfries",
        Family    = "Other",
        DeckCount = 1
    },
    Reserve = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.top_face_up(44),
        Rule = Sol.Rules.none_none_top
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Rule     = { Base = Sol.Rules.Base.Ace(), Build = Sol.Rules.Build.UpInSuit(), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    Tableau = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = {
                    Base = Sol.Rules.Base.Any(),
                    Build = {
                        Hint = { "DownAlternateColors" }, --custom "Same rank or down by alternate color"
                        Func = function(_, base, drop)
                            return base.Color ~= drop.Color and (base.Rank == drop.Rank or Sol.get_rank(base.Rank, -1, false) == drop.Rank)
                        end
                    },
                    Move = Sol.Rules.Move.TopOrPile()
                }
            }
        end
    }
}


------

local galloway           = Sol.copy(dumfries)
galloway.Info.Name       = "Galloway"
galloway.Reserve.Initial = Sol.Initial.top_face_up(45)
galloway.Tableau.Size    = 7


------

local robin           = Sol.copy(dumfries)
robin.Info.Name       = "Robin"
robin.Info.DeckCount  = 2
robin.Reserve.Initial = Sol.Initial.top_face_up(92)
robin.Foundation.Size = 8
robin.Tableau.Size    = 12


------


------------------------

Sol.register_game(dumfries)
Sol.register_game(galloway)
Sol.register_game(robin)
