// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

local fifty_two_pickup = {
    Info = {
        Name = "Fifty-Two PickUp",
        Family = "Other",
        DeckCount = 1,
        DisableAutoPlay = true
    },
    Foundation = {
        Position = {
            x = 0,
            y = 0
        }
        Rule = {
            Base = Sol.Rules.Base.Any()
            Build = Sol.Rules.Build.Any()
            Move = Sol.Rules.Move.None()
        }
    },
    Tableau = {
        Size = 52,
        Pile = @(i) {
            Position = {
                x = 1 + (rand() % 50) / 10.0,
                y = 1 + (rand() % 50) / 10.0
            }
            Initial = Sol.Initial.face_up(1)
            Layout = Sol.Pile.Layout.Column
            HasMarker = false
            Rule = {
                Base = Sol.Rules.Base.None()
                Build = Sol.Rules.Build.None()
                Move = Sol.Rules.Move.Top()
            }
        }
    }
}

//////

////////////////////////

Sol.register_game(fifty_two_pickup)