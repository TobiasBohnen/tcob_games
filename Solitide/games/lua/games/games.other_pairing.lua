-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT


local pyramid = {
    Info        = {
        Name = "Pyramid",
        Family = "Other",
        DeckCount = 1,
        Redeals = 2
    },
    Stock       = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_up(24)
    },
    Waste       = { Position = { x = 0, y = 1 } },
    Foundation  = {
        Position = { x = 6, y = 0 },
        Rule = Sol.Rules.none_none_none
    },
    Tableau     = {
        Size = 28,
        Pile = function(i) return Sol.Pyramid.face_up_pile(28, 1, { x = 0, y = -0.5 }, i) end
    },
    on_init     = function(game) Sol.Pyramid.face_up_flip(28, 1, game.Tableau) end,
    on_drop     = function(game, pile)
        pile:move_cards(game.Foundation[1], pile.CardCount - 1, 2, false)
    end,
    can_play    = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == "Tableau" and not targetPile.IsPlayable then return false end

        if targetPile.Type == "Foundation" then
            if card.Rank == "King" then return true end
        end
        if targetPile.Type == "Tableau" or targetPile.Type == "Stock" or targetPile.Type == "Waste" then
            if Sol.RankValues[card.Rank] + Sol.RankValues[targetPile.Cards[targetCardIndex].Rank] == 13 then
                return true
            end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    on_end_turn = function(game) Sol.Pyramid.face_up_flip(28, 1, game.Tableau) end,
    deal        = Sol.Ops.Deal.stock_to_waste,
    redeal      = function(game)
        if Sol.Ops.Redeal.waste_to_stock(game) then
            game.Stock[1]:flip_up_cards()
            return true
        end
        return false
    end
}

------

------------------------

Sol.register_game(pyramid)
