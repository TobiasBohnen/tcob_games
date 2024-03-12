// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

local ops = dofile("./base/ops.nut", true)
local piles = dofile("./base/piles.nut", true)
local rules = dofile("./base/rules.nut", true)

local aces_up = {
    Info = {
        Name = "Aces Up",
        Type = "ClosedNonBuilder",
        Family = "Other",
        DeckCount = 1,
        CardDealCount = 4,
        Redeals = 0
    },
    Stock = {
        Initial = piles.Initial.face_down(48)
    },
    Foundation = {
        Rule = {
            Move = rules.Move.None
        }
    },
    Tableau = {
        Size = 4,
        Create = @(i) {
            Initial = piles.Initial.face_up(1),
                Layout = "Column",
                Rule = {
                    Build = rules.Build.NoBuilding,
                    Move = rules.Move.Top,
                    Empty = rules.Empty.Any
                }
        }
    },
    check_drop = function(game, targetPile, _, drop, numCards) {
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
    on_deal = @(game) game.Stock[0].deal_to_group(game.Tableau, false),
    on_created = @(game) Lua.Sol.Layout.klondike(game)
}

# # # # # # # #

local aces_up_5 = Sol.copy(aces_up)
aces_up_5.Info.Name = "Aces Up 5"
aces_up_5.Stock.Initial = piles.Initial.face_down(47)
aces_up_5.Tableau.Size = 5

# # # # # # # #

local aces_square = {
    Info = {
        Name = "Aces Square",
        Type = "ClosedNonBuilder",
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
        Initial = piles.Initial.face_down(36)
    },
    Foundation = {
        Position = {
            x = 4.5,
            y = 0
        },
        Rule = {
            Move = rules.Move.None
        }
    },
    Tableau = {
        Size = 16,
        Create = @(i) {
            Position = {
                    x = i % 4,
                    y = i / 4
                },
                Initial = piles.Initial.face_up(1),
                Layout = "Column",
                Rule = {
                    Build = rules.Build.NoBuilding,
                    Move = rules.Move.Top,
                    Empty = rules.Empty.None
                }
        }
    },
    check_drop = function(game, targetPile, _, drop, numCards) {
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
    on_change = function(game) {
        local foundation = game.Foundation[0]
        foreach(tab in game.Tableau) {
            if (tab.CardCount > 1) {
                tab.move_cards(foundation, 0, tab.CardCount, false)
            }
        }

        game.Stock[0].deal_to_group(game.Tableau, true)
    },
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
        Type = "SimplePacker",
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
        Initial = piles.Initial.face_down(46)
    },
    Waste = {
        Position = {
            x = 2.5,
            y = 0
        }
    },
    Foundation = {
        Size = 4,
        Create = @(i) {
            Position = {
                    x = four_seasons_fou_pos[i][0],
                    y = four_seasons_fou_pos[i][1]
                },
                Initial = piles.Initial.face_up(i == 0 ? 1 : 0),
                Rule = {
                    Build = rules.Build.UpInSuit,
                    Wrap = true,
                    Move = rules.Move.None,
                    Empty = @(game) rules.Empty.FirstFoundation(game, 0)
                }
        }
    },
    Tableau = {
        Size = 5,
        Create = @(i) {
            Position = {
                    x = four_seasons_tab_pos[i][0],
                    y = four_seasons_tab_pos[i][1]
                },
                Initial = piles.Initial.face_up(1),
                Layout = "Squared",
                Rule = {
                    Build = rules.Build.DownByRank,
                    Wrap = true,
                    Move = rules.Move.Top,
                    Empty = rules.Empty.Any
                }

        }
    },
    on_deal = ops.Deal.stock_to_waste
}

# # # # # # # # # # # # # # # #
Sol.register_game(aces_up)
Sol.register_game(aces_up_5)
Sol.register_game(aces_square)
Sol.register_game(four_seasons)