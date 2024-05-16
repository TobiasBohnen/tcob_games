-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local duke = {
    Info       = {
        Name      = "Duke",
        Family    = "Canfield",
        DeckCount = 1,
        Redeals   = 2
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(36)
    },
    Waste      = { Position = { x = 1, y = 0 } },
    Reserve    = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i % 2 * 2, y = i // 2 + 1 },
                Initial  = Sol.Initial.face_up(3),
                Layout   = Sol.Pile.Layout.Row,
                Rule     = Sol.Rules.none_none_top,
            }
        end
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 4, y = 1 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.any_downac_inseq
            }
        end
    },
    redeal     = Sol.Ops.Redeal.waste_to_stock,
    deal       = Sol.Ops.Deal.stock_to_waste
}


------

local dutchess           = Sol.copy(duke)
dutchess.Info.Name       = "Dutchess"
dutchess.Info.Redeals    = 1
dutchess.Foundation.Pile = function(i)
    return {
        Position = { x = i + 3, y = 0 },
        Rule     = Sol.Rules.ff_upsuit_none_l13
    }
end
dutchess.Tableau.Pile    = function(i)
    return {
        Position = { x = i + 4, y = 1 },
        Initial  = Sol.Initial.face_up(1),
        Layout   = Sol.Pile.Layout.Column,
        Rule     = Sol.Rules.any_downac_inseq
    }
end
dutchess.can_play        = function(game, targetPile, targetCardIndex, card, numCards)
    local srcPile = game:find_pile(card)
    if game.Foundation[1].IsEmpty then
        return srcPile.Type == Sol.Pile.Type.Reserve and targetPile == game.Foundation[1]
    end
    if targetPile.Type == Sol.Pile.Type.Tableau and targetPile.IsEmpty then
        local reserveEmpty = true
        for _, v in ipairs(game.Reserve) do
            if v.IsEmpty then
                reserveEmpty = false
                break
            end
        end
        if (reserveEmpty and srcPile.Type ~= Sol.Pile.Type.Waste)
            or (not reserveEmpty and srcPile.Type ~= Sol.Pile.Type.Reserve) then
            return false
        end
    end

    return game:can_play(targetPile, targetCardIndex, card, numCards)
end
dutchess.deal            = function(game)
    if game.Foundation[1].IsEmpty then return false end
    return Sol.Ops.Deal.stock_to_waste(game)
end


------

------------------------

Sol.register_game(duke)
Sol.register_game(dutchess)
