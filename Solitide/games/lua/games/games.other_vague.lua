-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local vague = {
    Info = {
        Name      = "Vague",
        Family    = "Other",
        DeckCount = 1
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.top_face_up(34)
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Rule     = { Base = Sol.Rules.Base.Suits({ Sol.Suits[i + 1] }), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.None() }
            }
        end
    },
    Reserve = {
        Size = 18,
        Pile = function(i)
            return {
                Position = { x = i % 6, y = i // 6 + 1 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = Sol.Rules.none_none_top
            }
        end
    },
    on_end_turn = function(game)
        Sol.Ops.Deal.to_group(game.Stock[1], game.Reserve, Sol.DealMode.IfEmpty)
    end
}


------

local thirty_two_cards = {
    Info = {
        Name      = "Thirty-Two Cards",
        Family    = "Other",
        DeckCount = 2
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(72)
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            if i < 4 then
                return {
                    Position = { x = i + 2, y = 0 },
                    Rule     = { Base = Sol.Rules.Base.Ace(), Build = Sol.Rules.Build.UpInSuit(), Move = Sol.Rules.Move.Top(), Limit = 13 }
                }
            else
                return {
                    Position = { x = i + 2, y = 0 },
                    Rule     = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownInSuit(), Move = Sol.Rules.Move.Top(), Limit = 13 }
                }
            end
        end
    },
    Reserve = {
        Size = 32,
        Pile = function(i)
            return {
                Position = { x = i % 8 + 1, y = i // 8 + 1 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = Sol.Rules.none_none_top
            }
        end
    },
    on_end_turn = function(game)
        Sol.Ops.Deal.to_group(game.Stock[1], game.Reserve, Sol.DealMode.IfEmpty)
    end
}


------


------------------------

Sol.register_game(vague)
Sol.register_game(thirty_two_cards)
