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

    build_up = function(card0, card1, interval, wrap) {
        return Sol.get_rank(card0.Rank, interval, wrap) == card1.Rank;
    },

    build_down = function(card0, card1, interval, wrap) {
        return Sol.get_rank(card0.Rank, -interval, wrap) == card1.Rank;
    }
};

local build = {
    None = {
        Hint = "No building.",
        Build = function(target, drop, interval, wrap) {
            return false;
        }
    },

    Any = {
        Hint = "Any card.",
        Build = function(target, drop, interval, wrap) {
            return true;
        }
    },

    InRank = {
        Hint = "Build by same rank.",
        Build = function(target, drop, interval, wrap) {
            return rules.in_rank(target, drop);
        }
    },

    InRankOrDownByRank = {
        Hint = "Build down by rank or by same rank.",
        Build = function(target, drop, interval, wrap) {
            return rules.in_rank(target, drop) || rules.build_down(target, drop, interval, wrap);
        }
    },

    RankPack = {
        Hint = "Build by same rank, then build up by rank.",
        Build = function(target, drop, interval, wrap) {
            if (target.CardCount % 4 == 0) {
                return rules.build_up(target, drop, interval, wrap);
            }
            return rules.in_rank(target, drop);
        }
    },

    UpOrDownByRank = {
        Hint = "Build up or down by rank.",
        Build = function(target, drop, interval, wrap) {
            return rules.build_up(target, drop, interval, wrap) || rules.build_down(target, drop, interval, wrap);
        }
    },

    DownByRank = {
        Hint = "Build down by rank.",
        Build = function(target, drop, interval, wrap) {
            return rules.build_down(target, drop, interval, wrap);
        }
    },

    UpByRank = {
        Hint = "Build up by rank.",
        Build = function(target, drop, interval, wrap) {
            return rules.build_up(target, drop, interval, wrap);
        }
    },

    UpOrDownAnyButOwnSuit = {
        Hint = "Build up or down by any suit but own.",
        Build = function(target, drop, interval, wrap) {
            return !rules.in_suit(target, drop) && (rules.build_up(target, drop, interval, wrap) || rules.build_down(target, drop, interval, wrap));
        }
    },

    DownAnyButOwnSuit = {
        Hint = "Build down by any suit but own.",
        Build = function(target, drop, interval, wrap) {
            return !rules.in_suit(target, drop) && rules.build_down(target, drop, interval, wrap);
        }
    },

    UpAnyButOwnSuit = {
        Hint = "Build up by any suit but own.",
        Build = function(target, drop, interval, wrap) {
            return !rules.in_suit(target, drop) && rules.build_up(target, drop, interval, wrap);
        }
    },

    UpOrDownInSuit = {
        Hint = "Build up or down by suit.",
        Build = function(target, drop, interval, wrap) {
            return rules.in_suit(target, drop) && (rules.build_up(target, drop, interval, wrap) || rules.build_down(target, drop, interval, wrap));
        }
    },

    DownInSuit = {
        Hint = "Build down by suit.",
        Build = function(target, drop, interval, wrap) {
            return rules.in_suit(target, drop) && rules.build_down(target, drop, interval, wrap);
        }
    },

    UpInSuit = {
        Hint = "Build up by suit.",
        Build = function(target, drop, interval, wrap) {
            return rules.in_suit(target, drop) && rules.build_up(target, drop, interval, wrap);
        }
    },

    UpOrDownInColor = {
        Hint = "Build up or down by color.",
        Build = function(target, drop, interval, wrap) {
            return rules.in_color(target, drop) && (rules.build_up(target, drop, interval, wrap) || rules.build_down(target, drop, interval, wrap));
        }
    },

    DownInColor = {
        Hint = "Build down by color.",
        Build = function(target, drop, interval, wrap) {
            return rules.in_color(target, drop) && rules.build_down(target, drop, interval, wrap);
        }
    },

    UpInColor = {
        Hint = "Build up by color.",
        Build = function(target, drop, interval, wrap) {
            return rules.in_color(target, drop) && rules.build_up(target, drop, interval, wrap);
        }
    },

    UpOrDownAlternateColors = {
        Hint = "Build up or down by alternate color.",
        Build = function(target, drop, interval, wrap) {
            return rules.alternate_color(target, drop) && (rules.build_up(target, drop, interval, wrap) || rules.build_down(target, drop, interval, wrap));
        }
    },

    DownAlternateColors = {
        Hint = "Build down by alternate color.",
        Build = function(target, drop, interval, wrap) {
            return rules.alternate_color(target, drop) && rules.build_down(target, drop, interval, wrap);
        }
    },

    UpAlternateColors = {
        Hint = "Build up by alternate color.",
        Build = function(target, drop, interval, wrap) {
            return rules.alternate_color(target, drop) && rules.build_up(target, drop, interval, wrap);
        }
    }
};


local move = {
    None = {
        IsPlayable = false,
        IsSequence = false
    },
    Top = {
        IsSequence = false,
        Move = @(_, target, idx) idx == target.CardCount - 1
    },
    TopOrPile = {
        Move = @(_, target, idx) idx == target.CardCount - 1 || idx == 0
    },
    FaceUp = {
        Move = @(_, target, idx) target.Cards[idx].IsFaceUp
    },
    InSeq = {
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
    InSeqInSuit = {
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
    InSeqInSuitOrSameRank = {
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
    SuperMove = {
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
        Build = build.UpInSuit,
        Move = move.Top,
        Base = base_tab.Ace
    }
}