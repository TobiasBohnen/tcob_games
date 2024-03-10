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



local empty = {
    ace = function()
        return function(card)
            return card.Rank == "Ace"
        end
    end,
    king = function()
        return function(card)
            return card.Rank == "King"
        end
    end,
    none = function()
        return function()
            return false
        end
    end,
    any = function()
        return function()
            return true
        end
    end,
    any_single = function()
        return function(_, numCards)
            return numCards == 1
        end
    end,
    first_foundation = function(game, interval)
        return function(card)
            local pile = game.Foundation[1]
            if pile.IsEmpty then
                return false
            end
            local rank = pile.Cards[1].Rank
            return card.Rank == Sol.get_rank(rank, interval or 0, true);
        end
    end,
    card = function(suit, rank)
        return function(card)
            return card.Rank == rank and card.Suit == suit;
        end
    end,
    card_color = function(color, rank)
        return function(card)
            return card.Rank == rank and Sol.SuitColors[card.Suit] == color;
        end
    end,
    suits = function(suits)
        return function(card)
            for _, value in ipairs(suits) do
                if value == card.Suit then
                    return true
                end
            end
            return false
        end
    end,
    ranks = function(ranks)
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
    Empty = empty,
}
