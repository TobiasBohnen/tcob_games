// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

local rules = {
    in_suit = function(card0, card1) {
        return card0.Suit == card1.Suit;
    },

    in_color = function(card0, card1) {
        return Sol.SuitColors[card0.Suit] == Sol.SuitColors[card1.Suit];
    },

    alternate_color = function(card0, card1) {
        return Sol.SuitColors[card0.Suit] != Sol.SuitColors[card1.Suit];
    },

    in_rank = function(card0, card1) {
        return card0.Rank == card1.Rank;
    },

    build_up = function(card0, card1, wrap, interval) {
        return Sol.get_rank(card0.Rank, interval, wrap) == card1.Rank;
    },

    build_down = function(card0, card1, wrap, interval) {
        return Sol.get_rank(card0.Rank, -interval, wrap) == card1.Rank;
    }
};

local build = {
    None = @(wrap = false, interval = 1) {
        Hint = "No building.", Build = @(target, drop) false
    },

    Any = @(wrap = false, interval = 1) {
        Hint = "Any card.", Build = @(target, drop) true
    },

    InRank = @(wrap = false, interval = 1) {
        Hint = "Build by same rank.", Build = @(target, drop) rules.in_rank(target, drop)
    },

    InRankOrDownByRank = @(wrap = false, interval = 1) {
        Hint = "Build down by rank or by same rank.", Build = @(target, drop) rules.in_rank(target, drop) || rules.build_down(target, drop, wrap, interval)
    },

    RankPack = @(wrap = false, interval = 1) {
        Hint = "Build by same rank, then build up by rank.",
            Build = function(target, drop) {
                if (target.CardCount % 4 == 0) {
                    return rules.build_up(target, drop, wrap, interval)
                }
                return rules.in_rank(target, drop)
            }
    },

    UpOrDownByRank = @(wrap = false, interval = 1) {
        Hint = "Build up or down by rank.", Build = @(target, drop) rules.build_up(target, drop, wrap, interval) || rules.build_down(target, drop, wrap, interval)
    },

    DownByRank = @(wrap = false, interval = 1) {
        Hint = "Build down by rank.", Build = @(target, drop) rules.build_down(target, drop, wrap, interval)
    },

    UpByRank = @(wrap = false, interval = 1) {
        Hint = "Build up by rank.", Build = @(target, drop) rules.build_up(target, drop, wrap, interval)
    },

    UpOrDownAnyButOwnSuit = @(wrap = false, interval = 1) {
        Hint = "Build up or down by any suit but own.", Build = @(target, drop) !rules.in_suit(target, drop) && (rules.build_up(target, drop, wrap, interval) || rules.build_down(target, drop, wrap, interval))
    },

    DownAnyButOwnSuit = @(wrap = false, interval = 1) {
        Hint = "Build down by any suit but own.", Build = @(target, drop) !rules.in_suit(target, drop) && rules.build_down(target, drop, wrap, interval)
    },

    UpAnyButOwnSuit = @(wrap = false, interval = 1) {
        Hint = "Build up by any suit but own.", Build = @(target, drop) !rules.in_suit(target, drop) && rules.build_up(target, drop, wrap, interval)
    },

    UpOrDownInSuit = @(wrap = false, interval = 1) {
        Hint = "Build up or down by suit.", Build = @(target, drop) rules.in_suit(target, drop) && (rules.build_up(target, drop, wrap, interval) || rules.build_down(target, drop, wrap, interval))
    },

    DownInSuit = @(wrap = false, interval = 1) {
        Hint = "Build down by suit.", Build = @(target, drop) rules.in_suit(target, drop) && rules.build_down(target, drop, wrap, interval)
    },

    UpInSuit = @(wrap = false, interval = 1) {
        Hint = "Build up by suit.", Build = @(target, drop) rules.in_suit(target, drop) && rules.build_up(target, drop, wrap, interval)
    },

    UpOrDownInColor = {
        Hint = "Build up or down by color.",
        Build = @(target, drop) rules.in_color(target, drop) && (rules.build_up(target, drop, wrap, interval) || rules.build_down(target, drop, wrap, interval))
    },

    DownInColor = @(wrap = false, interval = 1) {
        Hint = "Build down by color.", Build = @(target, drop) rules.in_color(target, drop) && rules.build_down(target, drop, wrap, interval)
    },

    UpInColor = @(wrap = false, interval = 1) {
        Hint = "Build up by color.", Build = @(target, drop) rules.in_color(target, drop) && rules.build_up(target, drop, wrap, interval)
    },

    UpOrDownAlternateColors = @(wrap = false, interval = 1) {
        Hint = "Build up or down by alternate color.", Build = @(target, drop) rules.alternate_color(target, drop) && (rules.build_up(target, drop, wrap, interval) || rules.build_down(target, drop, wrap, interval))
    },

    DownAlternateColors = @(wrap = false, interval = 1) {
        Hint = "Build down by alternate color.", Build = @(target, drop) rules.alternate_color(target, drop) && rules.build_down(target, drop, wrap, interval)
    },

    UpAlternateColors = @(wrap = false, interval = 1) {
        Hint = "Build up by alternate color.", Build = @(target, drop) rules.alternate_color(target, drop) && rules.build_up(target, drop, wrap, interval)
    }
};

local move = {
    None = @() {
        IsPlayable = false, IsSequence = false
    },
    Top = @() {
        IsSequence = false, Move = @(_, target, idx) idx == target.CardCount - 1
    },
    TopOrPile = @() {
        Move = @(_, target, idx) idx == target.CardCount - 1 || idx == 0
    },
    FaceUp = @() {
        Move = @(_, target, idx) target.Cards[idx].IsFaceUp
    },
    InSeq = @() {
        Move = function(game, target, idx) {
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
    InSeqInSuit = @() {
        Move = function(game, target, idx) {
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
    InSeqInSuitOrSameRank = @() {
        Move = function(game, target, idx) {
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
    SuperMove = @() {
        Move = function(game, target, idx) {
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

local base_tab = {
    //direct
    Ace = @(_, card, _) card.Rank == "Ace",
    King = @(_, card, _) card.Rank == "King",
    None = @(_, _, _) false,
    Any = @(_, _, _) true,
    AnySingle = @(_, _, numCards) numCards == 1,
    //indirect
    FirstFoundation = function(game, card, interval) {
        local pile = game.Foundation[0]
        if (pile.IsEmpty) {
            return false
        }
        local rank = pile.Cards[0].Rank
        return card.Rank == Sol.get_rank(rank, interval, true)
    },
    Card = @(card, suit, rank) card.Rank == rank && card.Suit == suit,
    CardColor = @(card, color, rank) card.Rank == rank && Sol.SuitColors[card.Suit] == color,
    Suits = function(card, s) {
        foreach(value in s) {
            if (value == card.Suit) {
                return true
            }
        }
        return false
    },
    Ranks = function(card, r) {
        foreach(value in r) {
            if (value == card.Rank) {
                return true
            }
        }
        return false
    }
}

return {
    Build = build,
    Move = move,
    Base = base_tab,

    ace_upsuit_top = {
        Base = base_tab.Ace,
        Build = build.UpInSuit()
        Move = move.Top()
    },
    any_downac_inseq = {
        Base = base_tab.Any,
        Build = build.DownAlternateColors(),
        Move = move.InSeq()
    },
    any_none_top = {
        Base = base_tab.Any,
        Build = build.None(),
        Move = move.Top()
    },
}