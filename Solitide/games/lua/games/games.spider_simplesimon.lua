-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT


local simple_simon           = {
    Info       = {
        Name      = "Simple Simon",
        Family    = "Spider",
        DeckCount = 1
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.spider_foundation }
    },
    Tableau    = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 3 and 8 or 10 - i),
                Layout  = "Column",
                Rule    = Sol.Rules.spider_tableau
            }
        end
    },
    on_init    = Sol.Layout.klondike
}

------

local simon_jester           = Sol.copy(simple_simon)
simon_jester.Info.Name       = "Simon Jester"
simon_jester.Info.DeckCount  = 2
simon_jester.Foundation.Size = 8
simon_jester.Tableau         = {
    Size = 14,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 2 and 13 or 14 - i),
            Layout  = "Column",
            Rule    = Sol.Rules.spider_tableau
        }
    end
}

------

local york                   = Sol.copy(simple_simon)
york.Info.Name               = "York"
york.Info.DeckCount          = 2
york.Foundation.Size         = 8
york.Tableau                 = {
    Size = 12,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up((i < 2 or i > 9) and 8 or 9),
            Layout  = "Column",
            Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(true), Move = Sol.Rules.Move.InSeqInSuit() }
        }
    end
}

------

local big_york               = Sol.copy(york)
big_york.Info.Name           = "Big York"
big_york.Info.DeckCount      = 3
big_york.Foundation.Size     = 12
big_york.Tableau             = {
    Size = 14,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up((i < 1 or i > 12) and 12 or 11),
            Layout  = "Column",
            Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(true), Move = Sol.Rules.Move.InSeqInSuit() }
        }
    end
}

------

------------------------

Sol.register_game(simple_simon)
Sol.register_game(big_york)
Sol.register_game(simon_jester)
Sol.register_game(york)
