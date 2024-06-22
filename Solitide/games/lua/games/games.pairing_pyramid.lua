-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT


local pyramid = {
    Info       = {
        Name      = "Pyramid",
        Family    = "Pairing",
        DeckCount = 1,
        Redeals   = 2
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial  = Sol.Initial.face_up(24)
    },
    Waste      = { Position = { x = 0, y = 1 } },
    Foundation = {
        Position = { x = 6, y = 0 },
        Rule     = Sol.Rules.none_none_none
    },
    Tableau    = {
        Size = 28,
        Pile = function(i) return Sol.Ops.Pyramid.face_up_pile(28, 1, { x = 1, y = -0.5 }, i) end
    },
    on_drop    = function(game, pile) pile:move_cards(game.Foundation[1], pile.CardCount - 1, 2, false) end,
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == Sol.Pile.Type.Tableau and not targetPile.IsPlayable then return false end
        if not targetPile.IsEmpty and targetPile.Cards[targetPile.CardCount].IsFaceDown then return false end

        -- card ranks have to add up to 13
        if targetPile.Type == Sol.Pile.Type.Foundation then
            if card.Rank == "King" then return true end
        elseif not targetPile.IsEmpty then
            if Sol.RankValues[card.Rank] + Sol.RankValues[targetPile.Cards[targetCardIndex].Rank] == 13 then
                return true
            end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    deal       = Sol.Ops.Deal.stock_to_waste,
    redeal     = function(game)
        if Sol.Ops.Redeal.waste_to_stock(game) then
            game.Stock[1]:flip_up_cards()
            return true
        end
        return false
    end
}


------

local double_pyramid          = Sol.copy(pyramid)
double_pyramid.Info.Name      = "Double Pyramid"
double_pyramid.Info.DeckCount = 2
double_pyramid.Stock.Initial  = Sol.Initial.face_up(59)
double_pyramid.Tableau        = {
    Size = 45,
    Pile = function(i) return Sol.Ops.Pyramid.face_up_pile(45, 1, { x = 0, y = -0.5 }, i) end
}


------

local relaxed_pyramid          = Sol.copy(pyramid)
relaxed_pyramid.Info.Name      = "Relaxed Pyramid"
relaxed_pyramid.Info.Objective = "ClearTableau"


------

local apophis         = Sol.copy(pyramid)
apophis.Info.Name     = "Apophis"
apophis.Stock.Initial = Sol.Initial.face_down(24)
apophis.Waste         = {
    Size = 3,
    Pile = function(i)
        return {
            Position = { x = 0, y = 1 + i }
        }
    end
}
apophis.deal          = function(game) return Sol.Ops.Deal.to_group(game.Stock[1], game.Waste) end
apophis.redeal        = function(game)
    local stock = game.Stock[1]
    if not stock.IsEmpty then return false end

    local cards = Sol.shuffle_piles(game, { game.Waste })
    if #cards == 0 then return false end

    while #cards > 0 do
        game.PlaceTop(table.remove(cards), stock, false)
    end
    stock:flip_down_cards()

    return true
end


------

local cheops        = Sol.copy(pyramid)
cheops.Info.Name    = "Cheops"
cheops.Info.Redeals = 0
cheops.can_play     = function(game, targetPile, targetCardIndex, card, numCards)
    if targetPile.Type == Sol.Pile.Type.Tableau and not targetPile.IsPlayable then return false end
    if targetPile.Type == Sol.Pile.Type.Foundation then return false end

    -- same rank or off by one
    local target = targetPile.Cards[targetCardIndex]
    if target.Rank == card.Rank or Sol.get_rank(target.Rank, 1, false) == card.Rank or Sol.get_rank(target.Rank, -1, false) == card.Rank then
        return true
    end

    return game:can_play(targetPile, targetCardIndex, card, numCards)
end


------

local king_tut          = Sol.copy(pyramid)
king_tut.Info.Name      = "King Tut"
king_tut.Info.Redeals   = -1
king_tut.Info.Objective = "ClearTableau"
king_tut.Stock.Initial  = Sol.Initial.face_down(24)
king_tut.Waste.Layout   = Sol.Pile.Layout.Fan
king_tut.deal           = Sol.Ops.Deal.stock_to_waste_by_3
king_tut.redeal         = Sol.Ops.Redeal.waste_to_stock


------

local giza = {
    Info       = {
        Name      = "Giza",
        Family    = "Pairing",
        DeckCount = 1
    },
    Reserve    = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i, y = 4 },
                Initial  = Sol.Initial.face_up(3),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.none_none_top
            }
        end
    },
    Foundation = {
        Position = { x = 6, y = 0 },
        Rule     = Sol.Rules.none_none_none
    },
    Tableau    = {
        Size = 28,
        Pile = function(i) return Sol.Ops.Pyramid.face_up_pile(28, 1, { x = 0.5, y = -0.5 }, i) end
    },
    on_drop    = function(game, pile) pile:move_cards(game.Foundation[1], pile.CardCount - 1, 2, false) end,
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        return pyramid.can_play(game, targetPile, targetCardIndex, card, numCards)
    end
}


------

local pyramid_dozen = {
    Info       = {
        Name      = "Pyramid Dozen",
        Family    = "Pairing",
        DeckCount = 1
    },
    Reserve    = {
        Size = 6,
        Pile = function(i)
            return {
                Position = { x = i + 0.5, y = 4 },
                Initial  = Sol.Initial.face_up(4),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.none_none_top
            }
        end
    },
    Foundation = {
        Position = { x = 6, y = 0 },
        Rule     = Sol.Rules.none_none_none
    },
    Tableau    = {
        Size = 28,
        Pile = function(i) return Sol.Ops.Pyramid.face_up_pile(28, 1, { x = 0, y = -0.5 }, i) end
    },
    on_drop    = function(game, pile) pile:move_cards(game.Foundation[1], pile.CardCount - 1, 2, false) end,
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == Sol.Pile.Type.Foundation then return false end
        if targetPile.Type == Sol.Pile.Type.Tableau and not targetPile.IsPlayable then return false end
        if not targetPile.IsEmpty and targetPile.Cards[targetPile.CardCount].IsFaceDown then return false end

        -- card ranks have to add up to 12
        if not targetPile.IsEmpty then
            local targetRank = targetPile.Cards[targetCardIndex].Rank
            local sum        = Sol.RankValues[card.Rank] + Sol.RankValues[targetRank]
            if sum == 12 or sum == 25 then --25 = King + Queen
                return true
            end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end
}


------


------------------------

Sol.register_game(pyramid)
Sol.register_game(apophis)
Sol.register_game(cheops)
Sol.register_game(double_pyramid)
Sol.register_game(giza)
Sol.register_game(king_tut)
Sol.register_game(pyramid_dozen)
Sol.register_game(relaxed_pyramid)
