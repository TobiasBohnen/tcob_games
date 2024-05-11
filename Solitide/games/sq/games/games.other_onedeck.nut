// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

local aglet = {
    Info = {
        Name = "Aglet",
        Family = "Other",
        DeckCount = 1
    },
    FreeCell = {
        Pile = {
            Position = {
                x = 0,
                y = 0
            }
            Layout = "Column"
            Rule  = {
                Base = Sol.Rules.Base.Any(),
                Build = Sol.Rules.Build.Any(),
                Move = Sol.Rules.Move.Top()
            }
        }
    },
    Foundation = {
        Size = 4,
        Pile = @(i) {
            Position = {
                x = i + 3,
                y = 0
            }
            Rule  = {
                Base = Sol.Rules.Base.Ace(),
                Build = Sol.Rules.Build.UpByRank(),
                Move = Sol.Rules.Move.Top()
            }
        }
    },
    Tableau = {
        Size = 8,
        Pile = @(i) {
            Position = {
                x = i + 1,
                y = 1
            }
            Initial = Sol.Initial.face_up(6)
            Layout = "Column"
            Rule  = {
                Base = Sol.Rules.Base.None(),
                Build = Sol.Rules.Build.None(),
                Move = Sol.Rules.Move.Top()
            }
        }
    },
    on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation
}

//////

local carpet_fou_pos = [
    [2, 1],
    [2, 2],
    [8, 1],
    [8, 2]
]

local carpet = {
    Info = {
        Name = "Carpet",
        Family = "Other",
        DeckCount = 1
    },
    Stock = {
        Position = {
            x = 0,
            y = 0
        },
        Initial = Sol.Initial.face_down(28)
    },
    Waste = {
        Position = {
            x = 0,
            y = 1
        }
    },
    FreeCell = {
        Size = 20,
        Pile = @(i) {
            Position = {
                x = i % 5 + 3,
                y = i / 5
            }
            Initial = Sol.Initial.face_up(1)
            Layout = "Column"
            Rule  = {
                Base = Sol.Rules.Base.Any(),
                Build = Sol.Rules.Build.None(),
                Move = Sol.Rules.Move.Top()
            }
        }
    },
    Foundation = {
        Size = 4,
        Pile = @(i) {
            Position = {
                x = carpet_fou_pos[i][0],
                y = carpet_fou_pos[i][1]
            }
            Rule  = Sol.Rules.ace_upsuit_top
        }
    },
    on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation,
    deal = Sol.Ops.Deal.stock_to_waste
}

////////////////////////

Sol.register_game(aglet)
Sol.register_game(carpet)