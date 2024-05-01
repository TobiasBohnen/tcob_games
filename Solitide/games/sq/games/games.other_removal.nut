// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

local function move_multiple(game) {
    local foundation = game.Foundation[0]
    foreach(tab in game.Tableau) {
        if (tab.CardCount > 1) {
            tab.move_cards(foundation, 0, tab.CardCount, false)
        }
    }

    Sol.Ops.Deal.to_group(game.Stock[0], game.Tableau, true)
}

//////

local aces_up = {
    Info = {
        Name = "Aces Up",
        Family = "Other",
        DeckCount = 1
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
    can_play = function(game, targetPile, _, card, numCards) {
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
    get_status = function(game) {
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
    deal = @(game) Sol.Ops.Deal.to_group(game.Stock[0], game.Tableau, false),
    on_init = @(game) Lua.Sol.Layout.klondike(game)
}

//////

local aces_up_5 = Sol.copy(aces_up)
aces_up_5.Info.Name = "Aces Up 5"
aces_up_5.Stock.Initial = Sol.Initial.face_down(47)
aces_up_5.Tableau.Size = 5

//////

local aces_square = {
    Info = {
        Name = "Aces Square",
        Family = "Other",
        DeckCount = 1
    },
    Stock = {
        Position = {
            x = 4.5,
            y = 2
        }
        Initial = Sol.Initial.face_down(36)
    },
    Foundation = {
        Position = {
            x = 4.5,
            y = 0
        }
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
            }
            Initial = Sol.Initial.face_up(1)
            Layout = "Column"
            Rule = {
                Base = Sol.Rules.Base.None(),
                Build = Sol.Rules.Build.None(),
                Move = Sol.Rules.Move.Top()
            }
        }
    },
    can_play = function(game, targetPile, _, card, numCards) {
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
    get_status = function(game) {
        if (game.Foundation[0].CardCount == 48) {
            return "Success"
        }
        return "Running"
    },
    on_end_turn = move_multiple
}

//////

local cover = {
    Info = {
        Name = "Cover",
        Family = "Other",
        DeckCount = 1
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
                    Base = Sol.Rules.Base.None(),
                    Build = {
                        Hint = "Same suit",
                        Func = @(game, dst, src) dst.Suit == src.Suit
                    }
                    Move = Sol.Rules.Move.Top()
                }
        }
    },
    get_status = function(game) {
        if (game.Foundation[0].CardCount == 48) {
            return "Success"
        }

        local suits = {}
        foreach(tab in game.Tableau) {
            if (suits.keys().find(tab.Cards[tab.CardCount - 1].Suit) != null) {
                return "Running"
            }
            suits[tab.Cards[tab.CardCount - 1].Suit] <- true
        }
        return "Failure"
    },
    on_end_turn = move_multiple
    on_init = @(game) Lua.Sol.Layout.klondike(game)
}

//////

local deck = Sol.copy(cover)
deck.Info.Name = "Deck"
deck.deal <- @(game) Sol.Ops.Deal.to_group(game.Stock[0], game.Tableau, false)
deck.get_status = function(game) {
    if (game.Foundation[0].CardCount == 48) {
        return "Success"
    }

    if (game.Stock[0].IsEmpty) {
        local suits = {}
        foreach(tab in game.Tableau) {
            if (tab.IsEmpty) {
                continue
            }
            if (suits.keys().find(tab.Cards[tab.CardCount - 1].Suit) != null) {
                return "Running"
            }
            suits[tab.Cards[tab.CardCount - 1].Suit] <- true
        }
        return "Failure"
    }

    return "Running"
}
deck.on_drop <- @(game, pile) pile.move_cards(game.Foundation[0], pile.CardCount - 2, 2, false)
deck.on_end_turn = null

////////////////////////

Sol.register_game(aces_up)
Sol.register_game(aces_up_5)
Sol.register_game(aces_square)
Sol.register_game(cover)
Sol.register_game(deck)