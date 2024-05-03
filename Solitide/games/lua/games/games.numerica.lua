-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local numerica                   = {
    Info        = {
        Name      = "Numerica",
        Family    = "Numerica",
        DeckCount = 1
    },
    Stock       = {
        Initial = Sol.Initial.face_down(51)
    },
    Waste       = {
        Initial = Sol.Initial.face_up(1)
    },
    Foundation  = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_uprank_top }
    },
    Tableau     = {
        Size = 4,
        Pile = {
            Layout = "Column",
            Rule = Sol.Rules.any_any_top
        }
    },
    can_play    = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == "Tableau" then
            local srcPile = game:find_pile(card)
            if srcPile.Type ~= "Waste" then return false end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    on_end_turn = function(game)
        local waste = game.Waste[1]

        if waste.IsEmpty then
            Sol.Ops.Deal.stock_to_waste(game)
        end
    end,
    on_init     = Sol.Layout.klondike
}

------

local numerica_2_decks           = Sol.copy(numerica)
numerica_2_decks.Info.Name       = "Numerica (2 Decks)"
numerica_2_decks.Info.DeckCount  = 2
numerica_2_decks.Stock.Initial   = Sol.Initial.face_down(103)
numerica_2_decks.Foundation.Size = 8
numerica_2_decks.Tableau.Size    = 6

------

------------------------

Sol.register_game(numerica)
Sol.register_game(numerica_2_decks)
