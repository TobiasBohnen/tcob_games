// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

local rules = {
    in_suit = @(card0, card1) card0.Suit == card1.Suit
    in_color = @(card0, card1) card0.Color == card1.Color
    alternate_color = @(card0, card1) card0.Color != card1.Color
    in_rank = @(card0, card1) card0.Rank == card1.Rank
    build_up = @(card0, card1, wrap, interval) Sol.get_rank(card0.Rank, interval, wrap) == card1.Rank
    build_down = @(card0, card1, wrap, interval) Sol.get_rank(card0.Rank, -interval, wrap) == card1.Rank
    build_up_or_down = function(card0, card1, wrap, interval) {
        return Sol.get_rank(card0.Rank, interval, wrap) == card1.Rank || Sol.get_rank(card0.Rank, -interval, wrap) == card1.Rank;
    }
}

local build = {
    None = @(wrap = false, interval = 1) {
        Hint = "No building", Func = @(game, cbase, drop) false
    },

    Any = @(wrap = false, interval = 1) {
        Hint = "Any card", Func = @(game, cbase, drop) true
    },

    InRank = @(wrap = false, interval = 1) {
        Hint = "By same rank", Func = @(game, cbase, drop) rules.in_rank(cbase, drop)
    },

    InRankOrDownByRank = @(wrap = false, interval = 1) {
        Hint = "Down by rank or by same rank", Func = @(game, cbase, drop) rules.in_rank(cbase, drop) || rules.build_down(cbase, drop, wrap, interval)
    },

    RankPack = @(wrap = false, interval = 1) {
        Hint = "By same rank, then up by rank"
        Func = function(game, cbase, drop) {
            local count = game.find_pile(cbase).CardCount
            if (count > 0 && count % (game.DeckCount * 4) == 0) {
                return rules.build_up(cbase, drop, wrap, interval)
            }
            return rules.in_rank(cbase, drop)
        }
    },

    UpOrDownByRank = @(wrap = false, interval = 1) {
        Hint = "Up or down by rank", Func = @(game, cbase, drop) rules.build_up_or_down(cbase, drop, wrap, interval)
    },

    DownByRank = @(wrap = false, interval = 1) {
        Hint = "Down by rank", Func = @(game, cbase, drop) rules.build_down(cbase, drop, wrap, interval)
    },

    UpByRank = @(wrap = false, interval = 1) {
        Hint = "Up by rank", Func = @(game, cbase, drop) rules.build_up(cbase, drop, wrap, interval)
    },

    UpOrDownAnyButOwnSuit = @(wrap = false, interval = 1) {
        Hint = "Up or down by any suit but own", Func = @(game, cbase, drop) !rules.in_suit(cbase, drop) && rules.build_up_or_down(cbase, drop, wrap, interval)
    },

    DownAnyButOwnSuit = @(wrap = false, interval = 1) {
        Hint = "Down by any suit but own", Func = @(game, cbase, drop) !rules.in_suit(cbase, drop) && rules.build_down(cbase, drop, wrap, interval)
    },

    UpAnyButOwnSuit = @(wrap = false, interval = 1) {
        Hint = "Up by any suit but own", Func = @(game, cbase, drop) !rules.in_suit(cbase, drop) && rules.build_up(cbase, drop, wrap, interval)
    },

    UpOrDownInSuit = @(wrap = false, interval = 1) {
        Hint = "Up or down by suit", Func = @(game, cbase, drop) rules.in_suit(cbase, drop) && rules.build_up_or_down(cbase, drop, wrap, interval)
    },

    DownInSuit = @(wrap = false, interval = 1) {
        Hint = "Down by suit", Func = @(game, cbase, drop) rules.in_suit(cbase, drop) && rules.build_down(cbase, drop, wrap, interval)
    },

    UpInSuit = @(wrap = false, interval = 1) {
        Hint = "Up by suit", Func = @(game, cbase, drop) rules.in_suit(cbase, drop) && rules.build_up(cbase, drop, wrap, interval)
    },

    UpOrDownInColor = {
        Hint = "Up or down by color",
        Func = @(cbase, drop) rules.in_color(game, cbase, drop) && rules.build_up_or_down(cbase, drop, wrap, interval)
    },

    DownInColor = @(wrap = false, interval = 1) {
        Hint = "Down by color", Func = @(game, cbase, drop) rules.in_color(cbase, drop) && rules.build_down(cbase, drop, wrap, interval)
    },

    UpInColor = @(wrap = false, interval = 1) {
        Hint = "Up by color", Func = @(game, cbase, drop) rules.in_color(cbase, drop) && rules.build_up(cbase, drop, wrap, interval)
    },

    UpOrDownAlternateColors = @(wrap = false, interval = 1) {
        Hint = "Up or down by alternate color", Func = @(game, cbase, drop) rules.alternate_color(cbase, drop) && rules.build_up_or_down(cbase, drop, wrap, interval)
    },

    DownAlternateColors = @(wrap = false, interval = 1) {
        Hint = "Down by alternate color", Func = @(game, cbase, drop) rules.alternate_color(cbase, drop) && rules.build_down(cbase, drop, wrap, interval)
    },

    UpAlternateColors = @(wrap = false, interval = 1) {
        Hint = "Up by alternate color", Func = @(game, cbase, drop) rules.alternate_color(cbase, drop) && rules.build_up(cbase, drop, wrap, interval)
    }
};

local move = {
    None = @() {
        Hint = "None", IsPlayable = false, IsSequence = false
    },
    Top = @() {
        Hint = "Top card", IsPlayable = true, IsSequence = false, Func = @(_, pile, idx) idx == pile.CardCount - 1
    },
    TopOrPile = @() {
        Hint = "Top card or whole pile", IsPlayable = true, IsSequence = true, Func = @(_, pile, idx) idx == pile.CardCount - 1 || idx == 0
    },
    FaceUp = @() {
        Hint = "Face-up cards"
        IsPlayable = true
        IsSequence = true
        Func = function(_, pile, idx) {
            for (local i = idx; i < pile.CardCount - 1; ++i) {
                if (!pile.Cards[i].IsFaceUp) {
                    return false
                }
            }
            return true
        }
    },
    InSeq = @() {
        Hint = "Sequence of cards"
        IsPlayable = true
        IsSequence = true
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
    },
    InSeqInSuit = @() {
        Hint = "Sequence of cards in the same suit"
        IsPlayable = true
        IsSequence = true
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
    },
    InSeqAlternateColors = @() {
        Hint = "Color-alternating card sequence"
        IsPlayable = true
        IsSequence = true
        Func = function(game, pile, idx) {
            local cards = pile.Cards
            if (cards[idx].IsFaceDown) {
                return false
            }
            for (local i = idx; i < cards.len() - 1; ++i) {
                local targetColor = cards[i].Color
                if (cards[i + 1].Suit == targetColor ||
                    !game.can_play(pile, i, cards[i + 1], 1)) {
                    return false
                }
            }
            return true
        }
    },
    InSeqInSuitOrSameRank = @() {
        Hint = "Sequence of cards in the same suit or rank"
        IsPlayable = true
        IsSequence = true
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
    },
    SuperMove = @() {
        Hint = "Top card (SuperMove)"
        IsPlayable = true
        IsSequence = true
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
    FirstFoundation = function(interval = 1) {
        local intervalStr = ""
        if (interval != 0) {
            if (interval > 0) {
                intervalStr += "+"
            }
            intervalStr += interval.tostring()
        }

        return {
            Hint = "Rank " + intervalStr + " of first foundation card",
            Func = function(game, card, numCards) {
                local rank = game.Storage["foundation_base"]
                if (rank == null) {
                    local pile = game.Foundation[0]
                    if (pile.IsEmpty) {
                        return false
                    }
                    rank = pile.Cards[1].Rank
                    game.Storage["foundation_base"] = rank
                }

                return card.Rank == Sol.get_rank(rank, interval, true)
            }
        }
    },
    Card = @(suit, rank) {
        Hint = rank + " of " + suit
        Func = @(game, card, numCards) card.Rank == rank && card.Suit == suit
    },
    ColorRank = @(color, rank) {
        Hint = color + " " + rank
        Func = @(game, card, numCards) card.Rank == rank && card.Color == color
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
        local ranks = ""

        for (local i = 0; i < suits.len(); i++) {
            ranks += suits[i]
            if (i < suits.len() - 1) {
                ranks += "/"
            }
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
    },
    SuitStack = @() {
        Hint = "King to Ace"
        Func = function(game, card, numCards) {
            if (numCards != 13 || card.Rank != "King") {
                return false
            }

            local srcPile = game.find_pile(card)
            local cards = srcPile.Cards
            local srcIdx = srcPile.get_card_index(card)
            for (local i = srcIdx; i < cards.len() - 1; ++i) {
                if (cards[i].Suit != card.Suit) {
                    return false
                }
            }
            return true
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