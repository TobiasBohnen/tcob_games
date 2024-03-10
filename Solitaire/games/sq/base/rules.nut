// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT


local empty = {
    ace = @(_) @(card, _) card.Rank == "Ace",
    king = @(_) @(card, _) card.Rank == "King",
    none = @(_) @(_, _) false,
    any = @(_) @(_, _) true,
    any_single = @(_) @(_, numCards) numCards == 1,
    first_foundation = function(game, interval) {
        return function(card, _) {
            local pile = game.Foundation[0]
            if (pile.IsEmpty) {
                return false
            }
            local rank = pile.Cards[0].Rank
            return card.Rank == Sol.get_rank(rank, interval, true)
        }
    },
    card = @(suit, rank) @(card, _) card.Rank == rank && card.Suit == suit,
    card_color = @(color, rank) @(card, _) card.Rank == rank && Sol.SuitColors[card.Suit] == color,
    suits = function(s) {
        return function(card, _) {
            foreach(value in s) {
                if (value == card.Suit) {
                    return true
                }
            }
            return false
        }
    },
    ranks = function(r) {
        return function(card, _) {
            foreach(value in r) {
                if (value == card.Rank) {
                    return true
                }
            }
            return false
        }
    }
}

return {
    Empty = empty,
}