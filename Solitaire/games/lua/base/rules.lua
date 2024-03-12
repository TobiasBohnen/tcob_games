-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local rules = {
    in_suit = function(card0, card1)
        return card0.Suit == card1.Suit
    end,

    in_color = function(card0, card1)
        return Sol.SuitColors[card0.Suit] == Sol.SuitColors[card1.Suit]
    end,
    alternate_color = function(card0, card1)
        return Sol.SuitColors[card0.Suit] ~= Sol.SuitColors[card1.Suit]
    end,

    in_rank = function(card0, card1)
        return card0.Rank == card1.Rank
    end,

    build_up = function(card0, card1, interval, wrap)
        return Sol.get_rank(card0.Rank, interval, wrap) == card1.Rank
    end,
    build_down = function(card0, card1, interval, wrap)
        return Sol.get_rank(card0.Rank, -interval, wrap) == card1.Rank
    end
}

local build = {
    NoBuilding         = {
        BuildHint = "No building.",
        Build = function(target, drop, interval, wrap)
            return false
        end
    },
    Any                = {
        BuildHint = "Any card.",
        Build = function(target, drop, interval, wrap)
            return true
        end
    },

    InRank             = {
        BuildHint = "Build by same rank.",
        Build = function(target, drop, interval, wrap)
            return rules.in_rank(target, drop)
        end
    },

    InRankOrDownByRank = {
        BuildHint = "Build down by rank or by same rank.",
        Build = function(target, drop, interval, wrap)
            return rules.in_rank(target, drop) or rules.build_down(target, drop, interval, wrap)
        end
    },

    RankPack           = {
        BuildHint = "Build by same rank, then build up by rank.",
        Build = function(target, drop, interval, wrap)
            if target.CardCount % 4 == 0 then
                return rules.build_up(target, drop, interval, wrap)
            end
            return rules.in_rank(target, drop)
        end
    },

    UpOrDownByRank     = {
        BuildHint = "Build up or down by rank.",
        Build = function(target, drop, interval, wrap)
            return rules.build_up(target, drop, interval, wrap) or rules.build_down(target, drop, interval, wrap)
        end
    },
    DownByRank         = {
        BuildHint = "Build down by rank.",
        Build = function(target, drop, interval, wrap)
            return rules.build_down(target, drop, interval, wrap)
        end
    },
    UpByRank           = {
        BuildHint = "Build up by rank.",
        Build = function(target, drop, interval, wrap)
            return rules.build_up(target, drop, interval, wrap)
        end
    },


    UpOrDownAnyButOwnSuit = {
        BuildHint = "Build up or down by any suit but own.",
        Build = function(target, drop, interval, wrap)
            return not rules.in_suit(target, drop) and (rules.build_up(target, drop, interval, wrap) or rules.build_down(target, drop, interval, wrap))
        end
    },
    DownAnyButOwnSuit = {
        BuildHint = "Build down by any suit but own.",
        Build = function(target, drop, interval, wrap)
            return not rules.in_suit(target, drop) and rules.build_down(target, drop, interval, wrap)
        end
    },
    UpAnyButOwnSuit = {
        BuildHint = "Build up by any suit but own.",
        Build = function(target, drop, interval, wrap)
            return not rules.in_suit(target, drop) and rules.build_up(target, drop, interval, wrap)
        end
    },

    UpOrDownInSuit = {
        BuildHint = "Build up or down by suit.",
        Build = function(target, drop, interval, wrap)
            return rules.in_suit(target, drop) and (rules.build_up(target, drop, interval, wrap) or rules.build_down(target, drop, interval, wrap))
        end
    },
    DownInSuit = {
        BuildHint = "Build down by suit.",
        Build = function(target, drop, interval, wrap)
            return rules.in_suit(target, drop) and rules.build_down(target, drop, interval, wrap)
        end
    },
    UpInSuit = {
        BuildHint = "Build up by suit.",
        Build = function(target, drop, interval, wrap)
            return rules.in_suit(target, drop) and rules.build_up(target, drop, interval, wrap)
        end
    },

    UpOrDownInColor = {
        BuildHint = "Build up or down by color.",
        Build = function(target, drop, interval, wrap)
            return rules.in_color(target, drop) and (rules.build_up(target, drop, interval, wrap) or rules.build_down(target, drop, interval, wrap))
        end
    },
    DownInColor = {
        BuildHint = "Build down by color.",
        Build = function(target, drop, interval, wrap)
            return rules.in_color(target, drop) and rules.build_down(target, drop, interval, wrap)
        end
    },
    UpInColor = {
        BuildHint = "Build up by color.",
        Build = function(target, drop, interval, wrap)
            return rules.in_color(target, drop) and rules.build_up(target, drop, interval, wrap)
        end
    },

    UpOrDownAlternateColors = {
        BuildHint = "Build up or down by alternate color.",
        Build = function(target, drop, interval, wrap)
            return rules.alternate_color(target, drop) and (rules.build_up(target, drop, interval, wrap) or rules.build_down(target, drop, interval, wrap))
        end
    },
    DownAlternateColors = {
        BuildHint = "Build down by alternate color.",
        Build = function(target, drop, interval, wrap)
            return rules.alternate_color(target, drop) and rules.build_down(target, drop, interval, wrap)
        end
    },
    UpAlternateColors = {
        BuildHint = "Build up by alternate color.",
        Build = function(target, drop, interval, wrap)
            return rules.alternate_color(target, drop) and rules.build_up(target, drop, interval, wrap)
        end
    },
}

local move = {
    None = {
        IsPlayable = false,
        IsSequence = false,
        Move = function(_, _, _) return false end
    },
    Top = {
        IsSequence = false,
        Move = function(_, target, idx)
            return idx == target.CardCount
        end
    },
    TopOrPile = {
        Move = function(_, target, idx)
            return idx == target.CardCount or idx == 1
        end
    },
    FaceUp = {
        Move = function(_, target, idx)
            return target.Cards[idx].IsFaceUp
        end
    },
    InSeq = {
        Move = function(game, target, idx)
            local cards = target.Cards
            if cards[idx].IsFaceDown then return false end
            for i = idx, #cards - 1, 1 do
                if not game:can_drop(target, i, cards[i + 1], 1) then return false end
            end
            return true
        end
    },
    InSeqInSuit = {
        Move = function(game, target, idx)
            local cards = target.Cards
            if cards[idx].IsFaceDown then return false end

            local targetSuit = cards[idx].Suit
            for i = idx, #cards - 1, 1 do
                if not game:can_drop(target, i, cards[i + 1], 1)
                    or cards[i + 1].Suit ~= targetSuit
                then
                    return false
                end
            end
            return true
        end
    },
    InSeqInSuitOrSameRank = {
        Move = function(game, target, idx)
            local cards = target.Cards
            if cards[idx].IsFaceDown then return false end
            local targetSuit = cards[idx].Suit

            local result = true
            for i = idx, #cards - 1, 1 do
                if not game:can_drop(target, i, cards[i + 1], 1)
                    or cards[i + 1].Suit ~= targetSuit
                then
                    result = false
                    break
                end
            end
            if result then return true end

            local targetRank = cards[#cards].Rank

            for i = idx, #cards - 1, 1 do
                if cards[i].Rank ~= targetRank then
                    return false
                end
            end
            return true
        end
    },
    SuperMove = {
        Move = function(game, target, idx)
            local cards = target.Cards
            if cards[idx].IsFaceDown then return false end

            local freeCell = game.FreeCell
            if #freeCell == 0 then return idx == target.CardCount end

            local movableCards = 0
            for _, fc in ipairs(freeCell) do
                if fc.IsEmpty then movableCards = movableCards + 1 end
            end
            if idx + movableCards < #cards then
                return false
            end

            for i = idx, #cards - 1, 1 do
                if not game:can_drop(target, i, cards[i + 1], 1) then return false end
            end
            return true
        end
    },
}

local empty = {
    Ace = function()
        return function(card)
            return card.Rank == "Ace"
        end
    end,
    King = function()
        return function(card)
            return card.Rank == "King"
        end
    end,
    None = function()
        return function()
            return false
        end
    end,
    Any = function()
        return function()
            return true
        end
    end,
    AnySingle = function()
        return function(_, numCards)
            return numCards == 1
        end
    end,
    FirstFoundation = function(game, interval)
        return function(card)
            local pile = game.Foundation[1]
            if pile.IsEmpty then
                return false
            end
            local rank = pile.Cards[1].Rank
            return card.Rank == Sol.get_rank(rank, interval or 0, true);
        end
    end,
    Card = function(suit, rank)
        return function(card)
            return card.Rank == rank and card.Suit == suit;
        end
    end,
    CardColor = function(color, rank)
        return function(card)
            return card.Rank == rank and Sol.SuitColors[card.Suit] == color;
        end
    end,
    Suits = function(suits)
        return function(card)
            for _, value in ipairs(suits) do
                if value == card.Suit then
                    return true
                end
            end
            return false
        end
    end,
    Ranks = function(ranks)
        return function(card)
            for _, value in ipairs(ranks) do
                if value == card.Rank then
                    return true
                end
            end
            return false
        end
    end
}

return {
    Build = build,
    Move = move,
    Empty = empty,

    ace_upsuit_top = { Build = build.UpInSuit, Move = move.Top, Empty = empty.Ace },
}
