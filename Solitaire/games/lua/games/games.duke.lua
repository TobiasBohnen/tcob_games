-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local duke               = {
    Info       = {
        Name          = "Duke",
        Family        = "Canfield",
        DeckCount     = 1,
        CardDealCount = 1,
        Redeals       = 2
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = ops.Initial.face_down(36)
    },
    Waste      = { Position = { x = 1, y = 0 } },
    Reserve    = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i % 2 * 2, y = i // 2 + 1 },
                Initial  = ops.Initial.face_up(3),
                Layout   = "Row",
                Rule     = rules.none_none_top,
            }
        end
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Rule = rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 4, y = 1 },
                Initial = ops.Initial.face_up(1),
                Layout = "Column",
                Rule = rules.any_downac_inseq
            }
        end
    },
    on_redeal  = ops.Redeal.waste_to_stock,
    on_deal    = ops.Deal.stock_to_waste
}

------

local dutchess           = Sol.copy(duke)
dutchess.Info.Name       = "Dutchess"
dutchess.Info.Redeals    = 1
dutchess.Foundation.Pile = function(i)
    return {
        Position = { x = i + 3, y = 0 },
        Rule = rules.ff_upsuit_none
    }
end
dutchess.Tableau.Pile    = function(i)
    return {
        Position = { x = i + 4, y = 1 },
        Initial = ops.Initial.face_up(1),
        Layout = "Column",
        Rule = { Base = rules.Base.Any, Build = rules.Build.DownAlternateColors(), Move = rules.Move.InSeq() }
    }
end
dutchess.check_playable  = function(game, targetPile, targetIndex, drop, numCards)
    local srcPile = game:find_pile(drop)
    if game.Foundation[1].IsEmpty then
        return srcPile.Type == "Reserve" and targetPile == game.Foundation[1]
    end
    if targetPile.Type == "Tableau" and targetPile.IsEmpty then
        local reserveEmpty = true
        for _, v in ipairs(game.Reserve) do
            if v.IsEmpty then
                reserveEmpty = false
                break
            end
        end
        if (reserveEmpty and srcPile.Type ~= "Waste")
            or (not reserveEmpty and srcPile.Type ~= "Reserve") then
            return false
        end
    end

    return game:can_play(targetPile, targetIndex, drop, numCards)
end
dutchess.on_deal         = function(game)
    if game.Foundation[1].IsEmpty then return false end
    return ops.Deal.stock_to_waste(game)
end

------------------------

Sol.register_game(duke)
Sol.register_game(dutchess)
