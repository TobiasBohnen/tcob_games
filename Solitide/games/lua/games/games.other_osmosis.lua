-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local function osmosis_layout(game)
    game.Stock[1].Position = { x = 5, y = 1 }
    game.Waste[1].Position = { x = 5, y = 2 }
    local foundation = game.Foundation
    for i = 0, #foundation - 1 do
        foundation[i + 1].Position = { x = 2, y = i }
    end
    local tableau = game.Tableau
    for i = 0, #tableau - 1 do
        tableau[i + 1].Position = { x = 0, y = i }
    end
end

local osmosis = {
    Info = {
        Name      = "Osmosis",
        Family    = "Other",
        DeckCount = 1,
        Redeals   = -1
    },
    Stock = {
        Initial = Sol.Initial.face_down(35)
    },
    Waste = {},
    Foundation = {
        Size = 4,
        Pile = function(i)
            local build = {}
            if i == 0 then
                build = Sol.Rules.Build.InSuit()
            else
                build = {
                    Hint = "By same rank",
                    Func = function(game, base, drop)
                        if base.Suit ~= drop.Suit then return false end

                        -- foundation directly above has to contain rank
                        local pile = game:find_pile(base)
                        local foundation = game.Foundation[pile.Index - 1]
                        local cards = foundation.Cards
                        for _, card in ipairs(cards) do
                            if card.Rank == drop.Rank then return true end
                        end

                        return false
                    end
                }
            end
            return {
                Initial = Sol.Initial.face_up(i == 0 and 1 or 0),
                Layout  = Sol.Pile.Layout.Row,
                Rule    = { Base = Sol.Rules.Base.FirstFoundation(), Build = build, Move = Sol.Rules.Move.None() }
            }
        end
    },
    Tableau = {
        Size = 4,
        Pile = {
            Initial = Sol.Initial.top_face_up(4),
            Layout  = Sol.Pile.Layout.Row,
            Rule    = Sol.Rules.none_none_top
        }
    },
    on_init = osmosis_layout,
    deal = Sol.Ops.Deal.stock_to_waste_by_3,
    redeal = Sol.Ops.Redeal.waste_to_stock,
    can_play = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == Sol.Pile.Type.Foundation and targetPile.IsEmpty then -- disallow gaps in the foundation column
            if game.Foundation[targetPile.Index - 1].IsEmpty then return false end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end
}


------

local peek = Sol.copy(osmosis)
peek.Info.Name = "Peek"
peek.Tableau.Pile.Initial = Sol.Initial.face_up(4)


------

local osmium = Sol.copy(osmosis)
osmium.Info.Name = "Osmium"
osmium.Foundation = {
    Size = 4,
    Pile = function(i)
        local build = {
            Hint = "By same rank",
            Func = function(game, base, drop)
                if base.Suit ~= drop.Suit then return false end

                -- foundations have to be in the same sequence
                local pile = game:find_pile(base)

                local foundation = game.Foundation
                local largestFoundation = pile
                for _, fou in ipairs(foundation) do
                    if fou.CardCount > largestFoundation.CardCount then
                        largestFoundation = fou
                    end
                end
                if largestFoundation.IsEmpty then return true end
                if largestFoundation.CardCount == pile.CardCount then return true end
                return largestFoundation.Cards[pile.CardCount + 1].Rank == drop.Rank
            end
        }

        return {
            Initial = Sol.Initial.face_up(i == 0 and 1 or 0),
            Layout  = Sol.Pile.Layout.Row,
            Rule    = { Base = Sol.Rules.Base.FirstFoundation(), Build = build, Move = Sol.Rules.Move.None() }
        }
    end
}
osmium.deal = Sol.Ops.Deal.stock_to_waste
osmium.can_play = nil


------

local osmium_2 = Sol.copy(osmium)
osmium_2.Info.Name = "Osmium II"
osmium_2.Tableau.Pile.Initial = Sol.Initial.face_up(4)

------

------------------------

Sol.register_game(osmosis)
Sol.register_game(osmium)
Sol.register_game(osmium_2)
Sol.register_game(peek)
