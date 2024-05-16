-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local board_patience = {
    Info = {
        Name      = "Board Patience",
        Family    = "Other",
        DeckCount = 1
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Initial  = Sol.Initial.face_up(i == 0 and 1 or 0),
                Rule     = Sol.Rules.ff_upsuit_none_l13
            }
        end
    },
    Tableau = {
        Size = 10,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial  = Sol.Initial.face_up(i == 0 and 6 or 5),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownAlternateColors(true), Move = Sol.Rules.Move.Top() }
            }
        end
    }
}


------

local four_by_four = {
    Info = {
        Name      = "Four by Four",
        Family    = "Other",
        DeckCount = 1
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(51)
    },
    Waste = { Position = { x = 1, y = 0 } },
    Foundation = {
        Position = { x = 4.5, y = 0 },
        Initial  = Sol.Initial.face_up(1),
        Rule     = { Base = Sol.Rules.Base.FirstFoundation(), Build = Sol.Rules.Build.RankPack(true), Move = Sol.Rules.Move.None() }
    },
    Tableau = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 1 },
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.any_updownrank_top
            }
        end
    },
    deal = Sol.Ops.Deal.stock_to_waste
}


------

local lucky_thirteen_pos <const> = {
    { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 }, { 4, 1 },
    --[[ --]] { 1, 2 }, { 2, 2 }, { 3, 2 }, --[[ --]]
    { 0, 3 }, { 1, 3 }, { 2, 3 }, { 3, 3 }, { 4, 3 }
}

local lucky_thirteen = {
    Info = {
        Name      = "Lucky Thirteen",
        Family    = "Other",
        DeckCount = 1
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 0.5, y = 0 },
                Rule     = Sol.Rules.ace_uprank_top
            }
        end
    },
    Tableau = {
        Size = 13,
        Pile = function(i)
            return {
                Position = { x = lucky_thirteen_pos[i + 1][1], y = lucky_thirteen_pos[i + 1][2] },
                Initial  = Sol.Initial.face_up(4),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = Sol.Rules.none_downrank_top
            }
        end
    }
}


------

------------------------

Sol.register_game(board_patience)
Sol.register_game(four_by_four)
Sol.register_game(lucky_thirteen)
