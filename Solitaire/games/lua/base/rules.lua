-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local build = {
    in_suit = function(card0, card1)
    end,
    suit = function(card0, suit)
    end,

    in_color = function(card0, card1)
    end,
    alternate_color = function(card0, card1)
    end,

    in_rank = function(card0, card1)
    end,
    rank = function(card0, rank)
    end,
    rank_higher = function(card0, card1)
    end,
    build_up = function(card0, card1, interval, wrap)
    end,
    build_down = function(card0, card1, interval, wrap)
    end,

    fill = function(pile, card0, numCards)
    end,
    empty = function(pile)
    end,
    limit_size = function(pile, numCards)
    end,
}

local move = {
    None = {
        IsPlayable = false,
        IsSequence = false,
        move = function(_, _, _) return false end
    },
    Top = {
        IsPlayable = true,
        IsSequence = false,
        move = function(_, target, idx)
            return idx == target.CardCount
        end
    },
    TopOrPile = {
        IsPlayable = true,
        IsSequence = true,
        move = function(_, target, idx)
            return idx == target.CardCount or idx == 1
        end
    },
    FaceUp = {
        IsPlayable = true,
        IsSequence = true,
        move = function(_, target, idx)
            return target.Cards[idx].IsFaceUp
        end
    },
    InSeq = {
        IsPlayable = true,
        IsSequence = true,
        move = function(game, target, idx)
            local cards = target.Cards
            if cards[idx].IsFaceDown then return false end
            for i = idx, #cards - 1, 1 do
                if not game:can_drop(target, i, cards[i + 1], 1) then return false end
            end
            return true
        end
    },
    InSeqInSuit = {
        IsPlayable = true,
        IsSequence = true,
        move = function(game, target, idx)
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
        IsPlayable = true,
        IsSequence = true,
        move = function(game, target, idx)
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
        IsPlayable = true,
        IsSequence = true,
        move = function(game, target, idx)
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
    Move = move,
    Empty = empty,
}
