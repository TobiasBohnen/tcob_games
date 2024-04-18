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

    build_up_or_down = function(card0, card1, wrap, interval) {
        return Sol.get_rank(card0.Rank, interval, wrap) == card1.Rank || Sol.get_rank(card0.Rank, -interval, wrap) == card1.Rank;
    }
};

local build = {
    None = @(wrap = false, interval = 1) {
        Hint = "No building", Func = @(game, dst, src) false
    },

    Any = @(wrap = false, interval = 1) {
        Hint = "Any card", Func = @(game, dst, src) true
    },

    InRank = @(wrap = false, interval = 1) {
        Hint = "By same rank", Func = @(game, dst, src) rules.in_rank(dst, src)
    },

    InRankOrDownByRank = @(wrap = false, interval = 1) {
        Hint = "Down by rank or by same rank", Func = @(game, dst, src) rules.in_rank(dst, src) || rules.build_down(dst, src, wrap, interval)
    },

    RankPack = function(wrap = false, interval = 1) {
        return {
            Hint = "By same rank, then up by rank",
            Func = function(game, dst, src) {
                local count = game.find_pile(dst).CardCount
                if (count > 0 && count % (game.DeckCount * 4) == 0) {
                    return rules.build_up(dst, src, wrap, interval)
                }
                return rules.in_rank(dst, src)
            }
        }
    },

    UpOrDownByRank = @(wrap = false, interval = 1) {
        Hint = "Up or down by rank", Func = @(game, dst, src) rules.build_up_or_down(dst, src, wrap, interval)
    },

    DownByRank = @(wrap = false, interval = 1) {
        Hint = "Down by rank", Func = @(game, dst, src) rules.build_down(dst, src, wrap, interval)
    },

    UpByRank = @(wrap = false, interval = 1) {
        Hint = "Up by rank", Func = @(game, dst, src) rules.build_up(dst, src, wrap, interval)
    },

    UpOrDownAnyButOwnSuit = @(wrap = false, interval = 1) {
        Hint = "Up or down by any suit but own", Func = @(game, dst, src) !rules.in_suit(dst, src) && rules.build_up_or_down(dst, src, wrap, interval)
    },

    DownAnyButOwnSuit = @(wrap = false, interval = 1) {
        Hint = "Down by any suit but own", Func = @(game, dst, src) !rules.in_suit(dst, src) && rules.build_down(dst, src, wrap, interval)
    },

    UpAnyButOwnSuit = @(wrap = false, interval = 1) {
        Hint = "Up by any suit but own", Func = @(game, dst, src) !rules.in_suit(dst, src) && rules.build_up(dst, src, wrap, interval)
    },

    UpOrDownInSuit = @(wrap = false, interval = 1) {
        Hint = "Up or down by suit", Func = @(game, dst, src) rules.in_suit(dst, src) && rules.build_up_or_down(dst, src, wrap, interval)
    },

    DownInSuit = @(wrap = false, interval = 1) {
        Hint = "Down by suit", Func = @(game, dst, src) rules.in_suit(dst, src) && rules.build_down(dst, src, wrap, interval)
    },

    UpInSuit = @(wrap = false, interval = 1) {
        Hint = "Up by suit", Func = @(game, dst, src) rules.in_suit(dst, src) && rules.build_up(dst, src, wrap, interval)
    },

    UpOrDownInColor = {
        Hint = "Up or down by color",
        Func = @(dst, src) rules.in_color(game, dst, src) && rules.build_up_or_down(dst, src, wrap, interval)
    },

    DownInColor = @(wrap = false, interval = 1) {
        Hint = "Down by color", Func = @(game, dst, src) rules.in_color(dst, src) && rules.build_down(dst, src, wrap, interval)
    },

    UpInColor = @(wrap = false, interval = 1) {
        Hint = "Up by color", Func = @(game, dst, src) rules.in_color(dst, src) && rules.build_up(dst, src, wrap, interval)
    },

    UpOrDownAlternateColors = @(wrap = false, interval = 1) {
        Hint = "Up or down by alternate color", Func = @(game, dst, src) rules.alternate_color(dst, src) && rules.build_up_or_down(dst, src, wrap, interval)
    },

    DownAlternateColors = @(wrap = false, interval = 1) {
        Hint = "Down by alternate color", Func = @(game, dst, src) rules.alternate_color(dst, src) && rules.build_down(dst, src, wrap, interval)
    },

    UpAlternateColors = @(wrap = false, interval = 1) {
        Hint = "Up by alternate color", Func = @(game, dst, src) rules.alternate_color(dst, src) && rules.build_up(dst, src, wrap, interval)
    }
};

local move = {
    None = @() {
        Hint = "None", IsPlayable = false, IsSequence = false
    },
    Top = @() {
        Hint = "Top card", IsSequence = false, Func = @(_, pile, idx) idx == pile.CardCount - 1
    },
    TopOrPile = @() {
        Hint = "Top card or whole pile", Func = @(_, pile, idx) idx == pile.CardCount - 1 || idx == 0
    },
    FaceUp = @() {
        Hint = "Face-up cards", Func = function(_, pile, idx) {
            for (local i = idx; i < pile.CardCount - 1; ++i) {
                if (!pile.Cards[i].IsFaceUp) {
                    return false
                }
            }
            return true
        }
    },
    InSeq = function() {
        return {
            Hint = "Sequence of cards",
            Func = function(game, pile, idx) {
                local cards = pile.Cards
                if (cards[idx].IsFaceDown) {
                    return false
                }
                for (local i = idx; i < cards.len() - 1; ++i) {
                    if (!game.can_play(pile, i, cards[i + 1], 1)) {
                        return false
                    }
                }
                return true
            }
        }
    },
    InSeqInSuit = function() {
        return {
            Hint = "Sequence of cards in the same suit",
            Func = function(game, pile, idx) {
                local cards = pile.Cards
                if (cards[idx].IsFaceDown) {
                    return false
                }
                local targetSuit = cards[idx].Suit
                for (local i = idx; i < cards.len() - 1; ++i) {
                    if (cards[i + 1].Suit != targetSuit ||
                        !game.can_play(pile, i, cards[i + 1], 1)) {
                        return false
                    }
                }
                return true
            }
        }
    },
    InSeqAlternateColors = function() {
        return {
            Hint = "Color-alternating card sequence",
            Func = function(game, pile, idx) {
                local cards = pile.Cards
                if (cards[idx].IsFaceDown) {
                    return false
                }
                for (local i = idx; i < cards.len() - 1; ++i) {
                    local targetColor = Sol.SuitColors[cards[i].Suit]
                    if (cards[i + 1].Suit == targetColor ||
                        !game.can_play(pile, i, cards[i + 1], 1)) {
                        return false
                    }
                }
                return true
            }
        }
    },
    InSeqInSuitOrSameRank = function() {
        return {
            Hint = "Sequence of cards in the same suit or rank",
            Func = function(game, pile, idx) {
                local cards = pile.Cards
                if (cards[idx].IsFaceDown) {
                    return false
                }
                local targetSuit = cards[idx].Suit

                local result = true
                for (local i = idx; i < cards.len() - 1; ++i) {
                    if (cards[i + 1].Suit != targetSuit ||
                        !game.can_play(pile, i, cards[i + 1], 1)) {
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
        }
    },
    SuperMove = function() {
        return {
            Hint = "Top card (SuperMove)",
            Func = function(game, pile, idx) {
                local cards = pile.Cards
                if (cards[idx].IsFaceDown) {
                    return false
                }

                local freeCell = game.FreeCell
                if (freeCell.len() == 0) {
                    return idx == pile.CardCount - 1
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
                    if (!game.can_play(pile, i, cards[i + 1], 1)) {
                        return false
                    }
                }
                return true
            }
        }
    }
}

local base_tab = {
    //direct
    Ace = @() {
        Hint = "Ace", Func = @(_, card, _) card.Rank == "Ace"
    },
    King = @() {
        Hint = "King", Func = @(_, card, _) card.Rank == "King"
    },
    None = @() {
        Hint = "None", Func = @(_, _, _) false
    },
    Any = @() {
        Hint = "Any", Func = @(_, _, _) true
    },
    AnySingle = @() {
        Hint = "Any; no sequences", Func = @(_, _, numCards) numCards == 1
    },
    FirstFoundation = function(interval) {
        local intervalStr = ""
        if (interval != 0) {
            intervalStr = interval.tostring()
            if (interval > 0) {
                intervalStr += "+"
            }
        }

        return {
            Hint = "Rank" + intervalStr + " of first foundation card",
            Func = function(game, card, numCards) {
                local pile = game.Foundation[0]
                if (pile.IsEmpty) {
                    return false
                }
                local rank = pile.Cards[0].Rank
                return card.Rank == Sol.get_rank(rank, interval, true)
            }
        }
    },
    Card = function(suit, rank) {
        return {
            Hint = rank + " of " + suit,
            Func = @(game, card, numCards) card.Rank == rank && card.Suit == suit
        }
    },
    CardColor = function(color, rank) {
        return {
            Hint = color + " " + rank,
            Func = @(game, card, numCards) card.Rank == rank && Sol.SuitColors[card.Suit] == color
        }
    },
    Suits = function(s) {
        local suits = "";
        foreach(str in s) {
            suits += str;
        }
        return {
            Hint = suits,
            Func = function(game, card, numCards) {
                foreach(value in s) {
                    if (value == card.Suit) {
                        return true
                    }
                }
                return false
            }
        }
    },
    Ranks = function(r) {
        local ranks = "";
        foreach(str in r) {
            ranks += str;
        }
        return {
            Hint = ranks,
            Func = function(game, card, numCards) {
                foreach(value in r) {
                    if (value == card.Rank) {
                        return true
                    }
                }
                return false
            }
        }
    }
}

return {
    Build = build,
    Move = move,
    Base = base_tab,

    ace_upsuit_top = {
        Base = base_tab.Ace(),
        Build = build.UpInSuit()
        Move = move.Top()
    },
    any_downac_inseq = {
        Base = base_tab.Any(),
        Build = build.DownAlternateColors(),
        Move = move.InSeq()
    },
    any_none_top = {
        Base = base_tab.Any(),
        Build = build.None(),
        Move = move.Top()
    },
}