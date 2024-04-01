// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

local ops = require("base/ops")
local rules = require("base/rules")

local aces_up = {
    Info = {
        Name = "Aces Up",
        Family = "Other",
        DeckCount = 1,
        CardDealCount = 4,
        Redeals = 0
    },
    Stock = {
        Initial = ops.Initial.face_down(48)
    },
    Foundation = {
        Rule = {
            Move = rules.Move.None()
        }
    },
    Tableau = {
        Size = 4,
        Pile = @(i) {
            Initial = ops.Initial.face_up(1),
                Layout = "Column",
                Rule = {
                    Base = rules.Base.Any(),
                    Build = rules.Build.None(),
                    Move = rules.Move.Top()
                }
        }
    },
    check_playable = function(game, targetPile, _, drop, numCards) {
        if (numCards != 1) {
            return false
        }

        if (targetPile.Type == "Foundation") {
            if (drop.Rank == "Ace") {
                return false
            }
            local dropRank = Sol.RankValues[drop.Rank]
            foreach(tab in game.Tableau) {
                if (!tab.IsEmpty) {
                    local tabCard = tab.Cards[tab.CardCount - 1]
                    if (tabCard.Suit == drop.Suit && (tabCard.Rank == "Ace" || Sol.RankValues[tabCard.Rank] > dropRank)) {
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
    on_deal = @(game) ops.Deal.to_group(game.Stock[0], game.Tableau, false),
    on_piles_created = @(game) Lua.Sol.Layout.klondike(game)
}

# # # # # # # #

local aces_up_5 = Sol.copy(aces_up)
aces_up_5.Info.Name = "Aces Up 5"
aces_up_5.Stock.Initial = ops.Initial.face_down(47)
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
        Initial = ops.Initial.face_down(36)
    },
    Foundation = {
        Position = {
            x = 4.5,
            y = 0
        },
        Rule = {
            Move = rules.Move.None()
        }
    },
    Tableau = {
        Size = 16,
        Pile = @(i) {
            Position = {
                    x = i % 4,
                    y = i / 4
                },
                Initial = ops.Initial.face_up(1),
                Layout = "Column",
                Rule = {
                    Base = rules.Base.None(),
                    Build = rules.Build.None(),
                    Move = rules.Move.Top()
                }
        }
    },
    check_playable = function(game, targetPile, _, drop, numCards) {
        if (drop.Rank == "Ace" || targetPile.IsEmpty || numCards > 1) {
            return false
        }

        if (targetPile.Type == "Tableau") {
            local targetCard = targetPile.Cards[0]
            if (targetCard.Rank == "Ace" || targetCard.Suit != drop.Suit) {
                return false
            }
            local tableau = game.Tableau
            local tabSize = tableau.len()

            local srcPile = game.find_pile(drop)
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

        ops.Deal.to_group(game.Stock[0], game.Tableau, true)
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
                Base = rules.Base.Any(),
                Build = rules.Build.Any(),
                Move = rules.Move.Top()
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
                    Base = rules.Base.Ace(),
                    Build = rules.Build.UpByRank(),
                    Move = rules.Move.Top()
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
                Initial = ops.Initial.face_up(6),
                Layout = "Column",
                Rule = {
                    Base = rules.Base.None(),
                    Build = rules.Build.None(),
                    Move = rules.Move.Top()
                }
        }
    },
    on_before_shuffle = ops.Shuffle.ace_to_foundation
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
        Initial = ops.Initial.face_down(46)
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
                Initial = ops.Initial.face_up(i == 0 ? 1 : 0),
                Rule = {
                    Base = rules.Base.FirstFoundation(0),
                    Build = rules.Build.UpInSuit(true),
                    Move = rules.Move.None()
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
                Initial = ops.Initial.face_up(1),
                Layout = "Squared",
                Rule = {
                    Base = rules.Base.Any(),
                    Build = rules.Build.DownByRank(true),
                    Move = rules.Move.Top()
                }

        }
    },
    on_deal = ops.Deal.stock_to_waste
}

# # # # # # # #

# # # # # # # # # # # # # # # #
Sol.register_game(aces_up)
Sol.register_game(aces_up_5)
Sol.register_game(aces_square)
Sol.register_game(aglet)
Sol.register_game(four_seasons)