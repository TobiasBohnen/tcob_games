// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

local move = {
    None = {
        IsPlayable = false,
        IsSequence = false,
        move = @(_, _, _) false
    },
    Top = {
        IsPlayable = true,
        IsSequence = false,
        move = @(_, target, idx) idx == target.CardCount - 1
    },
    TopOrPile = {
        IsPlayable = true,
        IsSequence = true,
        move = @(_, target, idx) idx == target.CardCount - 1 || idx == 0
    },
    FaceUp = {
        IsPlayable = true,
        IsSequence = true,
        move = @(_, target, idx) target.Cards[idx].IsFaceUp
    },
    InSeq = {
        IsPlayable = true,
        IsSequence = true,
        move = function(game, target, idx) {
            local cards = target.Cards
            if (cards[idx].IsFaceDown) {
                return false
            }
            for (local i = idx; i < cards.len() - 1; ++i) {
                if (!game.can_drop(target, i, cards[i + 1], 1)) {
                    return false
                }
            }
            return true
        }
    },
    InSeqInSuit = {
        IsPlayable = true,
        IsSequence = true,
        move = function(game, target, idx) {
            local cards = target.Cards
            if (cards[idx].IsFaceDown) {
                return false
            }
            for (local i = idx; i < cards.len() - 1; ++i) {
                if (cards[i + 1].Suit != targetSuit ||
                    !game.can_drop(target, i, cards[i + 1], 1)) {
                    return false
                }
            }
            return true
        }
    },
    InSeqInSuitOrSameRank = {
        IsPlayable = true,
        IsSequence = true,
        move = function(game, target, idx) {
            local cards = target.Cards
            if (cards[idx].IsFaceDown) {
                return false
            }

            local result = true
            for (local i = idx; i < cards.len() - 1; ++i) {
                if (cards[i + 1].Suit != targetSuit ||
                    !game.can_drop(target, i, cards[i + 1], 1)) {
                    result = false
                    break
                }
            }
            if (result) {
                return true
            }

            local targetRank = cards[cards.len() - 1].Rank
            for (local i = idx; i < cards.len() - 1; ++i) {
                if (cards[i].Rank != targetRank) {
                    return false
                }
            }
            return true
        }
    },
    SuperMove = {
        IsPlayable = true,
        IsSequence = true,
        move = function(game, target, idx) {
            local cards = target.Cards
            if (cards[idx].IsFaceDown) {
                return false
            }

            local freeCell = game.FreeCell
            if (freeCell.len() == 0) {
                return idx == target.CardCount - 1
            }

            local movableCards = 0
            foreach(fc in freeCell) {
                if (fc.IsEmpty) {
                    movableCards++
                }
            }

            if (idx + movableCards < cards.len()) {
                return false
            }

            for (local i = idx; i < cards.len() - 1; ++i) {
                if (!game.can_drop(target, i, cards[i + 1], 1)) {
                    return false
                }
            }
            return true
        }
    },
}

local empty = {
    Ace = @(_) @(card, _) card.Rank == "Ace",
    King = @(_) @(card, _) card.Rank == "King",
    None = @(_) @(_, _) false,
    Any = @(_) @(_, _) true,
    AnySingle = @(_) @(_, numCards) numCards == 1,
    FirstFoundation = function(game, interval) {
        return function(card, _) {
            local pile = game.Foundation[0]
            if (pile.IsEmpty) {
                return false
            }
            local rank = pile.Cards[0].Rank
            return card.Rank == Sol.get_rank(rank, interval, true)
        }
    },
    Card = @(suit, rank) @(card, _) card.Rank == rank && card.Suit == suit,
    CardColor = @(color, rank) @(card, _) card.Rank == rank && Sol.SuitColors[card.Suit] == color,
    Suits = function(s) {
        return function(card, _) {
            foreach(value in s) {
                if (value == card.Suit) {
                    return true
                }
            }
            return false
        }
    },
    Ranks = function(r) {
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
    Move = move,
    Empty = empty,
}