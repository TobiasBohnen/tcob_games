// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

local ops = dofile("./base/ops.nut", true)
local piles = dofile("./base/piles.nut", true)
dofile("./base/common.nut", true)

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
        Position = {
            x = 0,
            y = 0
        },
        Initial = piles.initial.face_down(48)
    },
    Foundation = {
        Position = {
            x = 1,
            y = 0
        },
        Rule = {
            Move = "None"
        }
    },
    Tableau = {
        Size = 4,
        create = @(i) {
            Position = {
                    x = i,
                    y = 1
                },
                Initial = piles.initial.face_up(1),
                Layout = "Column",
                Rule = {
                    Build = "NoBuilding",
                    Move = "Top",
                    Empty = "Any"
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
            local dropRank = RankValues[drop.Rank]
            foreach(tab in game.Tableau) {
                if (!tab.IsEmpty) {
                    local tabCard = tab.Cards[tab.CardCount - 1]
                    if (tabCard.Suit == drop.Suit && (tabCard.Rank == "Ace" || RankValues[tabCard.Rank] > dropRank)) {
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
    on_deal = @(game) game.Stock[0].deal_to_group(game.Tableau, false)
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
        Initial = piles.initial.face_down(46)
    },
    Waste = {
        Position = {
            x = 2.5,
            y = 0
        }
    },
    Foundation = {
        Size = 4,
        create = @(i) {
            Position = {
                    x = four_seasons_fou_pos[i][0],
                    y = four_seasons_fou_pos[i][1]
                },
                Initial = piles.initial.face_up(i == 0 ? 1 : 0),
                Rule = {
                    Build = "UpInSuit",
                    Wrap = true,
                    Move = "None",
                    Empty = "FirstFoundation"
                }
        }
    },
    Tableau = {
        Size = 5,
        create = @(i) {
            Position = {
                    x = four_seasons_tab_pos[i][0],
                    y = four_seasons_tab_pos[i][1]
                },
                Initial = piles.initial.face_up(1),
                Layout = "Squared",
                Rule = {
                    Build = "DownByRank",
                    Wrap = true,
                    Move = "Top",
                    Empty = "Any"
                }

        }
    },
    on_deal = ops.deal.stock_to_waste
}

# # # # # # # # # # # # # # # #
RegisterGame(aces_up)
RegisterGame(four_seasons)