-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local grandee = {
    Info = {
        Name      = "Grandee",
        Family    = "Other",
        DeckCount = 2
    },
    Stock = {
        Position = { x = 7, y = 4 },
        Initial = Sol.Initial.face_down(90)
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau = {
        Size = 14,
        Pile = function(i)
            return {
                Position = { x = i % 7 + 0.5, y = i // 7 + 2 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = Sol.Rules.any_downsuit_top
            }
        end
    },
    deal = Sol.Ops.Deal.stock_to_tableau
}


------

local union_square_foundation = {
    Base = Sol.Rules.Base.Ace(),
    Build = {
        Hint = { "UpOrDownInSuit" },
        Func = function(game, base, drop)
            local count = game:find_pile(base).CardCount
            if count < 13 then
                return base.Suit == drop.Suit and Sol.get_rank(base.Rank, 1, false) == drop.Rank
            else
                return base.Suit == drop.Suit and Sol.get_rank(base.Rank, -1, false) == drop.Rank
            end
        end
    },
    Move = Sol.Rules.Move.None()
}
local union_square_tableau = {
    Base = Sol.Rules.Base.Any(),
    Build = {
        Hint = { "UpOrDownInSuit" },
        Func = function(game, base, drop)
            if base.Suit ~= drop.Suit then return false end

            local pile     = game:find_pile(base)
            local up, down = true, true
            if pile.CardCount > 1 then
                up = Sol.RankValues[base.Rank] > Sol.RankValues[pile.Cards[pile.CardCount - 1].Rank]
                down = not up
            end

            if up and Sol.get_rank(base.Rank, 1, false) == drop.Rank then
                return true
            end
            if down and Sol.get_rank(base.Rank, -1, false) == drop.Rank then
                return true
            end
            return false
        end
    },
    Move = Sol.Rules.Move.Top()
}

local union_square = {
    Info = {
        Name      = "Union Square",
        Family    = "Other",
        DeckCount = 2
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(88)
    },
    Waste = {
        Position = { x = 1, y = 0 }
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = 7, y = i },
                Rule     = union_square_foundation
            }
        end
    },
    Tableau = {
        Size = 16,
        Pile = function(i)
            return {
                Position = { x = i % 4 + 2.5, y = i // 4 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = union_square_tableau
            }
        end
    },
    deal = Sol.Ops.Deal.stock_to_waste
}


------

local solid_square = {
    Info            = {
        Name      = "Solid Square",
        Family    = "Other",
        DeckCount = 2
    },
    Stock           = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(80)
    },
    Waste           = {
        Position = { x = 1, y = 0 }
    },
    Foundation      = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = 8, y = i },
                Rule     = union_square_foundation
            }
        end
    },
    Tableau         = {
        Size = 20,
        Pile = function(i)
            return {
                Position = { x = i % 5 + 2.5, y = i // 5 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = union_square_tableau
            }
        end
    },
    on_before_setup = function(game, card)
        if card.Rank == "Ace" and card.Deck == 1 then
            return game.PlaceTop(card, game.Foundation, true)
        end

        return false
    end,
    deal            = Sol.Ops.Deal.stock_to_waste,
    on_end_turn     = Sol.Ops.Deal.waste_or_stock_to_empty_tableau
}


------

local s_patience_pos <const> = {
    { 0, 0.75 }, { 0, 2.50 },
    { 1, 0.25 }, { 1, 1.25 }, { 1, 2.75 },
    { 2, 0.00 }, { 2, 1.50 }, { 2, 3.00 },
    { 3, 0.25 }, { 3, 1.75 }, { 3, 2.75 },
    { 4, 0.50 }, { 4, 2.25 },
}

local s_patience = {
    Info       = {
        Name      = "S Patience",
        Family    = "Other",
        DeckCount = 2
    },
    Stock      = {
        Position = { x = 5.5, y = 1 },
        Initial = Sol.Initial.face_down(91)
    },
    Waste      = {
        Position = { x = 6.5, y = 1 }
    },
    FreeCell   = {
        Size = 2,
        Pile = function(i)
            return {
                Position = { x = 5.5 + i, y = 2 },
                Rule     = Sol.Rules.any_any_top
            }
        end
    },
    Foundation = {
        Size = 13,
        Pile = function(i)
            return {
                Position = { x = s_patience_pos[i + 1][1], y = s_patience_pos[i + 1][2] },
                Initial  = Sol.Initial.face_up(1),
                Rule     = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpByRank(true), Move = Sol.Rules.Move.None(), Limit = 8 }
            }
        end
    },
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        local srcPile = game:find_pile(card)
        if targetPile.Type == Sol.Pile.Type.FreeCell and srcPile.Type == Sol.Pile.Type.FreeCell then
            return false
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    deal       = function(game)
        if game.Waste[1].IsEmpty then
            return Sol.Ops.Deal.to_pile(game.Stock[1], game.Waste[1], 1)
        end
        return false
    end,
}


------

------------------------

Sol.register_game(grandee)
Sol.register_game(union_square)
Sol.register_game(s_patience)
Sol.register_game(solid_square)
