// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

local aces_up = {
    Info = {
        Name = "Aces Up",
        Family = "Other",
        DeckCount = 1,
        CardDealCount = 4,
        Redeals = 0
    },
    Stock = {
        Initial = Sol.Initial.face_down(48)
    },
    Foundation = {
        Rule = {
            Move = Sol.Rules.Move.None()
        }
    },
    Tableau = {
        Size = 4,
        Pile = @(i) {
            Initial = Sol.Initial.face_up(1),
                Layout = "Column",
                Rule = {
                    Base = Sol.Rules.Base.Any(),
                    Build = Sol.Rules.Build.None(),
                    Move = Sol.Rules.Move.Top()
                }
        }
    },
    check_playable = function(game, targetPile, _, card, numCards) {
        if (numCards != 1) {
            return false
        }

        if (targetPile.Type == "Foundation") {
            if (card.Rank == "Ace") {
                return false
            }
            local cardRank = Sol.RankValues[card.Rank]
            foreach(tab in game.Tableau) {
                if (!tab.IsEmpty) {
                    local tabCard = tab.Cards[tab.CardCount - 1]
                    if (tabCard.Suit == card.Suit && (tabCard.Rank == "Ace" || Sol.RankValues[tabCard.Rank] > cardRank)) {
                        return true
                    }
                }
            }
        } else if (targetPile.Type == "Tableau") {
            return targetPile.IsEmpty
        }

        return false
    },
    check_state = function(game) {
        if (game.Foundation[0].CardCount == 48) {
            return "Success"
        }

        if (game.Stock[0].IsEmpty) {
            local suits = {}
            foreach(tab in game.Tableau) {
                if (tab.IsEmpty || suits.keys().find(tab.Cards[tab.CardCount - 1].Suit) != null) {
                    return "Running"
                }
                suits[tab.Cards[tab.CardCount - 1].Suit] <- true
            }
            return "Failure"
        }

        return "Running"
    },
    on_deal = @(game) Sol.Ops.Deal.to_group(game.Stock[0], game.Tableau, false),
    on_piles_created = @(game) Lua.Sol.Layout.klondike(game)
}

# # # # # # # #

local aces_up_5 = Sol.copy(aces_up)
aces_up_5.Info.Name = "Aces Up 5"
aces_up_5.Stock.Initial = Sol.Initial.face_down(47)
aces_up_5.Tableau.Size = 5

# # # # # # # #

local aces_square = {
    Info = {
        Name = "Aces Square",
        Family = "Other",
        DeckCount = 1,
        CardDealCount = 2,
        Redeals = 0
    },
    Stock = {
        Position = {
            x = 4.5,
            y = 2
        },
        Initial = Sol.Initial.face_down(36)
    },
    Foundation = {
        Position = {
            x = 4.5,
            y = 0
        },
        Rule = {
            Move = Sol.Rules.Move.None()
        }
    },
    Tableau = {
        Size = 16,
        Pile = @(i) {
            Position = {
                    x = i % 4,
                    y = i / 4
                },
                Initial = Sol.Initial.face_up(1),
                Layout = "Column",
                Rule = {
                    Base = Sol.Rules.Base.None(),
                    Build = Sol.Rules.Build.None(),
                    Move = Sol.Rules.Move.Top()
                }
        }
    },
    check_playable = function(game, targetPile, _, card, numCards) {
        if (card.Rank == "Ace" || targetPile.IsEmpty || numCards > 1) {
            return false
        }

        if (targetPile.Type == "Tableau") {
            local targetCard = targetPile.Cards[0]
            if (targetCard.Rank == "Ace" || targetCard.Suit != card.Suit) {
                return false
            }
            local tableau = game.Tableau
            local tabSize = tableau.len()

            local srcPile = game.find_pile(card)
            local dstPileIndex = -1
            local srcPileIndex = -1
            for (local i = 0; i < tabSize; i++) {
                if ((tableau[i] <=> targetPile) == 0) {
                    dstPileIndex = i;
                }
                if ((tableau[i] <=> srcPile) == 0) {
                    srcPileIndex = i;
                }
            }

            if (dstPileIndex == -1 || srcPileIndex == -1) {
                return false
            }

            return (dstPileIndex / 4 == srcPileIndex / 4) || (dstPileIndex % 4 == srcPileIndex % 4)
        }

        return false
    },
    check_state = function(game) {
        if (game.Foundation[0].CardCount == 48) {
            return "Success"
        }
        return "Running"
    },
    on_end_turn = function(game) {
        local foundation = game.Foundation[0]
        foreach(tab in game.Tableau) {
            if (tab.CardCount > 1) {
                tab.move_cards(foundation, 0, tab.CardCount, false)
            }
        }

        Sol.Ops.Deal.to_group(game.Stock[0], game.Tableau, true)
    }
}

# # # # # # # #

local aglet = {
    Info = {
        Name = "Aglet",
        Family = "Other",
        DeckCount = 1,
        CardDealCount = 0,
        Redeals = 0
    },
    FreeCell = {
        Pile = {
            Position = {
                x = 0,
                y = 0
            },
            Layout = "Column",
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
                },
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
                },
                Initial = Sol.Initial.face_up(6),
                Layout = "Column",
                Rule = {
                    Base = Sol.Rules.Base.None(),
                    Build = Sol.Rules.Build.None(),
                    Move = Sol.Rules.Move.Top()
                }
        }
    },
    on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation
}

# # # # # # # #

local four_seasons_fou_pos = [
    [0, 1],
    [0, 3],
    [4, 1],
    [4, 3]
]

local four_seasons_tab_pos = [
    [2, 1],
    [1, 2],
    [2, 2],
    [3, 2],
    [2, 3]
]

local four_seasons = {
    Info = {
        Name = "Four Seasons",
        Family = "Other",
        DeckCount = 1,
        CardDealCount = 1,
        Redeals = 0
    },
    Stock = {
        Position = {
            x = 1.5,
            y = 0
        },
        Initial = Sol.Initial.face_down(46)
    },
    Waste = {
        Position = {
            x = 2.5,
            y = 0
        }
    },
    Foundation = {
        Size = 4,
        Pile = @(i) {
            Position = {
                    x = four_seasons_fou_pos[i][0],
                    y = four_seasons_fou_pos[i][1]
                },
                Initial = Sol.Initial.face_up(i == 0 ? 1 : 0),
                Rule = {
                    Base = Sol.Rules.Base.FirstFoundation(0),
                    Build = Sol.Rules.Build.UpInSuit(true),
                    Move = Sol.Rules.Move.None()
                }
        }
    },
    Tableau = {
        Size = 5,
        Pile = @(i) {
            Position = {
                    x = four_seasons_tab_pos[i][0],
                    y = four_seasons_tab_pos[i][1]
                },
                Initial = Sol.Initial.face_up(1),
                Layout = "Squared",
                Rule = {
                    Base = Sol.Rules.Base.Any(),
                    Build = Sol.Rules.Build.DownByRank(true),
                    Move = Sol.Rules.Move.Top()
                }

        }
    },
    on_deal = Sol.Ops.Deal.stock_to_waste
}

# # # # # # # #

# # # # # # # # # # # # # # # #
Sol.register_game(aces_up)
Sol.register_game(aces_up_5)
Sol.register_game(aces_square)
Sol.register_game(aglet)
Sol.register_game(four_seasons)