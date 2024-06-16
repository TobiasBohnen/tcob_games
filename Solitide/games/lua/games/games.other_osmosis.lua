-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local function osmosis_layout(game)
    if #game.Stock > 0 then
        game.Stock[1].Position = { x = 5.5, y = 1 }
        game.Waste[1].Position = { x = 5.5, y = 2 }
    end

    local foundation = game.Foundation
    for i = 0, #foundation - 1 do
        foundation[i + 1].Position = { x = 2, y = i }
    end

    local tableau = game.Tableau
    if #tableau > 0 then
        for i = 0, #tableau - 1 do
            tableau[i + 1].Position = { x = 0, y = i }
        end
    end

    local reserve = game.Reserve
    if #reserve > 0 then
        for i = 0, #reserve - 1 do
            reserve[i + 1].Position = { x = i % 8 + 5.5, y = i // 8 }
        end
    end
end

local function osmosis_build(game, base, drop)
    if base.Suit ~= drop.Suit then return false end

    -- foundation directly above has to contain rank
    local pile = game:find_pile(base)
    if pile.Index == 1 then return true end
    local foundation = game.Foundation[pile.Index - 1]
    local cards      = foundation.Cards
    for _, card in ipairs(cards) do
        if card.Rank == drop.Rank then return true end
    end

    return false
end

local function osmium_build(game, base, drop)
    if base.Suit ~= drop.Suit then return false end

    -- foundations have to be in the same sequence
    local pile              = game:find_pile(base)
    local foundation        = game.Foundation
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

------

local osmosis = {
    Info       = {
        Name      = "Osmosis",
        Family    = "Other",
        DeckCount = 1,
        Redeals   = -1
    },
    Stock      = {
        Initial = Sol.Initial.face_down(35)
    },
    Waste      = {},
    Foundation = {
        Size = 4,
        Pile = function(i)
            local build = {}
            if i == 0 then
                build = Sol.Rules.Build.InSuit()
            else
                build = {
                    Hint = { "InRank" },
                    Func = osmosis_build
                }
            end
            return {
                Initial = Sol.Initial.face_up(i == 0 and 1 or 0),
                Layout  = Sol.Pile.Layout.Row,
                Rule    = { Base = Sol.Rules.Base.FirstFoundation(), Build = build, Move = Sol.Rules.Move.None() }
            }
        end
    },
    Tableau    = {
        Size = 4,
        Pile = {
            Initial = Sol.Initial.top_face_up(4),
            Layout  = Sol.Pile.Layout.Row,
            Rule    = Sol.Rules.none_none_top
        }
    },
    on_init    = osmosis_layout,
    deal       = Sol.Ops.Deal.stock_to_waste_by_3,
    redeal     = Sol.Ops.Redeal.waste_to_stock,
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == Sol.Pile.Type.Foundation and targetPile.IsEmpty then -- disallow gaps in the foundation column
            if game.Foundation[targetPile.Index - 1].IsEmpty then return false end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end
}


------

local bridesmaids         = Sol.copy(osmosis)
bridesmaids.Info.Name     = "Bridesmaids"
bridesmaids.Tableau       = nil
bridesmaids.Stock.Initial = Sol.Initial.face_down(51)


------

local peek                = Sol.copy(osmosis)
peek.Info.Name            = "Peek"
peek.Tableau.Pile.Initial = Sol.Initial.face_up(4)


------

local osmium      = Sol.copy(osmosis)
osmium.Info.Name  = "Osmium"
osmium.Foundation = {
    Size = 4,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i == 0 and 1 or 0),
            Layout  = Sol.Pile.Layout.Row,
            Rule    = {
                Base = Sol.Rules.Base.FirstFoundation(),
                Build = { Hint = { "InRank" }, Func = osmium_build },
                Move = Sol.Rules.Move.None()
            }
        }
    end
}
osmium.deal       = Sol.Ops.Deal.stock_to_waste
osmium.can_play   = nil


------

local osmium_2                = Sol.copy(osmium)
osmium_2.Info.Name            = "Osmium II"
osmium_2.Tableau.Pile.Initial = Sol.Initial.face_up(4)


------

local open_peek = {
    Info       = {
        Name      = "Open Peek",
        Family    = "Other",
        DeckCount = 1,
        Redeals   = -1
    },
    Reserve    = {
        Size = 32,
        Pile = {
            Initial = Sol.Initial.face_up(1),
            Rule    = Sol.Rules.none_none_top
        }
    },
    Foundation = {
        Size = 4,
        Pile = {
            Layout = Sol.Pile.Layout.Row,
            Rule   = {
                Base = Sol.Rules.Base.FirstFoundation(),
                Build = {
                    Hint = { "InRank" },
                    Func = osmosis_build
                },
                Move = Sol.Rules.Move.None()
            }
        }
    },
    Tableau    = {
        Size = 4,
        Pile = {
            Initial = Sol.Initial.face_up(5),
            Layout  = Sol.Pile.Layout.Row,
            Rule    = Sol.Rules.none_none_top
        }
    },
    on_init    = osmosis_layout,
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == Sol.Pile.Type.Foundation and targetPile.IsEmpty then -- disallow gaps in the foundation column
            if targetPile.Index == 1 then return true end
            if game.Foundation[targetPile.Index - 1].IsEmpty then return false end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end
}


------

local open_osmium           = Sol.copy(open_peek)
open_osmium.Info.Name       = "Open Osmium"
open_osmium.Foundation.Pile = {
    Layout = Sol.Pile.Layout.Row,
    Rule   = {
        Base = Sol.Rules.Base.FirstFoundation(),
        Build = { Hint = { "InRank" }, Func = osmium_build },
        Move = Sol.Rules.Move.None()
    }
}
open_osmium.can_play        = function(game, targetPile, targetCardIndex, card, numCards)
    if targetPile.Type == Sol.Pile.Type.Foundation and targetPile.Index == 1 and targetPile.IsEmpty then -- allow any card on first foundation
        return true
    end

    return game:can_play(targetPile, targetCardIndex, card, numCards)
end


------

------------------------

Sol.register_game(osmosis)
Sol.register_game(bridesmaids)
Sol.register_game(open_osmium)
Sol.register_game(open_peek)
Sol.register_game(osmium)
Sol.register_game(osmium_2)
Sol.register_game(peek)
