-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local churchill = {
    Info       = {
        Name      = "Churchill",
        Family    = "Gypsy",
        DeckCount = 2
    },
    Reserve    = {
        Initial = Sol.Initial.face_up(6),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.none_none_top
    },
    Stock      = {
        Initial = Sol.Initial.face_down(68)
    },
    Foundation = {
        Size = 8,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = Sol.Initial.top_face_up(i < 5 and i + 1 or 10 - i),
                Layout  = Sol.Pile.Layout.Column,
                Rule    = Sol.Rules.king_downac_inseq
            }
        end
    },
    deal       = function(game)
        -- don't deal card on sequence with king as first card in pile
        local stock = game.Stock[1]
        local tableau = game.Tableau

        if stock.IsEmpty then return false end
        for _, toPile in ipairs(tableau) do
            if stock.IsEmpty then break end
            local inSeq = true
            local cards = toPile.Cards
            for index, card in ipairs(cards) do
                if index > 13 or card.IsFaceDown or card.Rank ~= Sol.Ranks[14 - index] then
                    inSeq = false
                    break
                end
            end

            if not inSeq then
                stock:move_cards(toPile, #stock.Cards, 1, false)
                toPile:flip_up_top_card()
            end
        end
        return true
    end,
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        -- reserve only to foundation
        if game:find_pile(card).Type == Sol.Pile.Type.Reserve and targetPile.Type ~= Sol.Pile.Type.Foundation then return false end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    on_init    = Sol.Layout.canfield
}


------

local pitt_the_younger           = Sol.copy(churchill)
pitt_the_younger.Info.Name       = "Pitt the Younger"
pitt_the_younger.Stock.Initial   = Sol.Initial.face_down(63)
pitt_the_younger.Reserve.Initial = Sol.Initial.face_up(11)


------

------------------------

Sol.register_game(churchill)
Sol.register_game(pitt_the_younger)
