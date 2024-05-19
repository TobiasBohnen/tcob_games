-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local board_patience = {
    Info = {
        Name      = "Board Patience",
        Family    = "Other",
        DeckCount = 1
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Initial  = Sol.Initial.face_up(i == 0 and 1 or 0),
                Rule     = Sol.Rules.ff_upsuit_top_l13
            }
        end
    },
    Tableau = {
        Size = 10,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial  = Sol.Initial.face_up(i == 0 and 6 or 5),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownAlternateColors(true), Move = Sol.Rules.Move.Top() }
            }
        end
    }
}


------

local diamond_mine = {
    Info = {
        Name      = "Diamond Mine",
        Family    = "Other",
        DeckCount = 1
    },
    Foundation = {
        Position = { x = 6, y = 0 },
        Rule     = { Base = Sol.Rules.Base.Suits({ "Diamonds" }), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.None() }
    },
    Tableau = {
        Size = 13,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial  = Sol.Initial.top_face_up(4),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.any_downrank_inseq
            }
        end
    },
    get_status = function(game)
        -- all diamonds in foundation; sort remaining suits
        local foundation = game.Foundation
        if foundation[1].CardCount == 13 then
            for _, tableau in ipairs(game.Tableau) do
                if tableau.IsEmpty then goto continue end
                if tableau.CardCount ~= 13 then return Sol.GameStatus.Running end

                local cards = tableau.Cards
                local targetSuit = cards[1].Suit
                for i = 1, 13 do
                    if cards[i].Rank ~= Sol.Ranks[14 - i] or cards[i].Suit ~= targetSuit then
                        return Sol.GameStatus.Running
                    end
                end

                ::continue::
            end
            return Sol.GameStatus.Success
        end
        --TODO check dead
        return Sol.GameStatus.Running
    end,
    can_play = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type ~= Sol.Pile.Type.Foundation and not targetPile.IsEmpty then
            local targetCard = targetPile.Cards[targetCardIndex]
            if targetCard.Suit == "Diamonds" or card.Suit == "Diamonds" then
                return false
            end
        elseif targetPile.Type == Sol.Pile.Type.Tableau and targetPile.IsEmpty then
            if card.Suit == "Diamonds" then
                return false
            end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end
}


------

local double_dot = {
    Info = {
        Name      = "Double Dot",
        Family    = "Other",
        DeckCount = 1
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(40)
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Rule     = {
                    Base = i < 2 and Sol.Rules.Base.Ace() or Sol.Rules.Base.Ranks({ "Two" }),
                    Build = Sol.Rules.Build.UpInSuit(true, 2),
                    Move = Sol.Rules.Move.Top()
                }
            }
        end
    },
    Tableau = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(true, 2), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    on_before_shuffle = function(game, card)
        if card.Rank == "Ace" and card.Color == "Black" then
            return game.PlaceTop(card, game.Foundation, 1, 2, true)
        elseif card.Rank == "Two" and card.Color == "Red" then
            return game.PlaceTop(card, game.Foundation, 3, 2, true)
        end

        return false
    end,
    deal = Sol.Ops.Deal.stock_to_tableau
}


------

local four_by_four = {
    Info = {
        Name      = "Four by Four",
        Family    = "Other",
        DeckCount = 1
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(51)
    },
    Waste = { Position = { x = 1, y = 0 } },
    Foundation = {
        Position = { x = 4.5, y = 0 },
        Initial  = Sol.Initial.face_up(1),
        Rule     = { Base = Sol.Rules.Base.FirstFoundation(), Build = Sol.Rules.Build.RankPack(true), Move = Sol.Rules.Move.None() }
    },
    Tableau = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 1 },
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.any_updownrank_top
            }
        end
    },
    deal = Sol.Ops.Deal.stock_to_waste
}


------

local lucky_thirteen_pos <const> = {
    { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 }, { 4, 1 },
    --[[ --]] { 1, 2 }, { 2, 2 }, { 3, 2 }, --[[ --]]
    { 0, 3 }, { 1, 3 }, { 2, 3 }, { 3, 3 }, { 4, 3 }
}

local lucky_thirteen = {
    Info = {
        Name      = "Lucky Thirteen",
        Family    = "Other",
        DeckCount = 1
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 0.5, y = 0 },
                Rule     = Sol.Rules.ace_uprank_top
            }
        end
    },
    Tableau = {
        Size = 13,
        Pile = function(i)
            return {
                Position = { x = lucky_thirteen_pos[i + 1][1], y = lucky_thirteen_pos[i + 1][2] },
                Initial  = Sol.Initial.face_up(4),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = Sol.Rules.none_downrank_top
            }
        end
    }
}


------

local turncoats = {
    Info = {
        Name      = "Turncoats",
        Family    = "Other",
        DeckCount = 1
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(40)
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau = {
        Size = 12,
        Pile = function(i)
            return {
                Position = { x = i % 6, y = i // 6 + 1 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownAlternateColors(), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    on_end_turn = function(game)
        Sol.Ops.Deal.to_group(game.Stock[1], game.Tableau, Sol.DealMode.IfEmpty)
    end
}


------

------------------------

Sol.register_game(board_patience)
Sol.register_game(diamond_mine)
Sol.register_game(double_dot)
Sol.register_game(four_by_four)
Sol.register_game(lucky_thirteen)
Sol.register_game(turncoats)
