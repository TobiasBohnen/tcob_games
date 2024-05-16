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
            Layout = Sol.Pile.Layout.Column
            Rule = {
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
            Rule = {
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
            Layout = Sol.Pile.Layout.Column
            Rule = {
                Base = Sol.Rules.Base.None(),
                Build = Sol.Rules.Build.None(),
                Move = Sol.Rules.Move.Top()
            }
        }
    },
    on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation
}

//////

local beacon = {
    Info = {
        Name = "Beacon",
        Family = "Other",
        DeckCount = 1
    },
    Stock = {
        Position = {
            x = 0,
            y = 0
        }
        Initial = Sol.Initial.face_down(20)
    },
    Foundation = {
        Position = {
            x = 3.5,
            y = 0
        }
        Rule = {
            Base = Sol.Rules.Base.Any(),
            Build = Sol.Rules.Build.UpByRank(true),
            Move = Sol.Rules.Move.Top()
        }
    },
    Tableau = {
        Size = 8,
        Pile = @(i) {
            Position = {
                x = i,
                y = 1
            }
            Initial = Sol.Initial.face_up(4)
            Layout = Sol.Pile.Layout.Column
            Rule = {
                Base = Sol.Rules.Base.Any(),
                Build = Sol.Rules.Build.DownByRank(true),
                Move = Sol.Rules.Move.InSeq()
            }
        }
    },
    on_end_turn = function(game) {
        Sol.Ops.Deal.to_group(game.Stock[0], game.Tableau, deal_mode.IfEmpty)
    }
}

//////

local bisley = {
    Info = {
        Name = "Bisley",
        Family = "Other",
        DeckCount = 1
    },
    Foundation = {
        Size = 8,
        Pile = function(i) {
            local rule = {}
            if (i < 4) {
                rule = {
                    Base = Sol.Rules.Base.Ace(),
                    Build = Sol.Rules.Build.UpInSuit(),
                    Move = Sol.Rules.Move.None()
                }
            } else {
                rule = {
                    Base = Sol.Rules.Base.King(),
                    Build = Sol.Rules.Build.DownInSuit(),
                    Move = Sol.Rules.Move.None()
                }
            }
            return {
                Position = {
                    x = i / 4 + 7,
                    y = i % 4
                }
                Rule = rule
            }
        }
    },
    Tableau = {
        Size = 12,
        Pile = @(i) {
            Position = {
                x = i % 6,
                y = i / 6 * 2
            }
            Initial = Sol.Initial.face_up(4)
            Layout = Sol.Pile.Layout.Column
            Rule = {
                Base = Sol.Rules.Base.None(),
                Build = Sol.Rules.Build.UpOrDownInSuit(),
                Move = Sol.Rules.Move.Top()
            }
        }
    },
    on_before_shuffle = function(game, card) {
        if (card.Rank == "Ace") {
            return game.PlaceTop(card, game.Foundation, 0, 4, true)
        }

        return false
    }
}

//////

local bisley_13 = Sol.copy(bisley)
bisley_13.Info.Name = "Bisley 13"
bisley_13.Tableau = {
    Size = 13,
    Pile = @(i) {
        Position = {
            x = i % 7,
            y = i / 7 * 2
        }
        Initial = Sol.Initial.face_up(i < 4 ? 3 : 4)
        Layout = Sol.Pile.Layout.Column
        Rule = {
            Base = Sol.Rules.Base.None(),
            Build = Sol.Rules.Build.UpOrDownInSuit(),
            Move = Sol.Rules.Move.Top()
        }
    }
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
            Layout = Sol.Pile.Layout.Column
            Rule = {
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
            Rule = Sol.Rules.ace_upsuit_top
        }
    },
    on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation,
    deal = Sol.Ops.Deal.stock_to_waste
}

////////////////////////

Sol.register_game(aglet)
Sol.register_game(beacon)
Sol.register_game(bisley)
Sol.register_game(bisley_13)
Sol.register_game(carpet)