-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local terrace_tableau = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownAlternateColors(true), Move = Sol.Rules.Move.Top() }

local terrace = {
    Info = {
        Name      = "Terrace",
        Family    = "Terrace",
        DeckCount = 2,
        Redeals   = 0
    },
    Stock = {
        Initial = Sol.Initial.face_down(89)
    },
    Waste = {},
    Reserve = {
        Initial = Sol.Initial.top_face_up(11),
        Layout = Sol.Pile.Layout.Row
    },
    Foundation = {
        Size = 8,
        Pile = {
            Rule = { Base = Sol.Rules.Base.FirstFoundation(), Build = Sol.Rules.Build.UpAlternateColors(true), Move = Sol.Rules.Move.None(), Limit = 13 }
        }
    },
    Tableau = {
        Size = 9,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 4 and 1 or 0),
                Layout  = Sol.Pile.Layout.Column,
                Rule    = terrace_tableau
            }
        end
    },
    deal = function(game)
        if game.Foundation[1].IsEmpty then return false end

        return Sol.Ops.Deal.stock_to_waste(game)
    end,
    can_play = function(game, targetPile, targetCardIndex, card, numCards)
        -- must play tableau card to foundation as first turn
        local foundation1 = game.Foundation[1]
        if foundation1.IsEmpty then
            return targetPile == foundation1 and game:find_pile(card).Type == Sol.Pile.Type.Tableau
        end

        -- reserve must be played to foundation
        if targetPile.Type ~= Sol.Pile.Type.Foundation and game:find_pile(card).Type == Sol.Pile.Type.Reserve then
            return false
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    on_drop = function(game, pile)
        -- fill tableau after first turn
        if pile.Type == Sol.Pile.Type.Foundation and pile.CardCount == 1 and pile.Index == 1 then
            Sol.Ops.Deal.to_group(game.Stock[1], game.Tableau, Sol.DealMode.IfEmpty)
            Sol.Ops.Deal.stock_to_waste(game)
        end
    end,
    on_end_turn = Sol.Ops.Deal.waste_or_stock_to_empty_tableau,
    on_init = Sol.Layout.terrace
}


------

local big_terrace = Sol.copy(terrace)
big_terrace.Info.Name = "Big Terrace"
big_terrace.Info.DeckCount = 4
big_terrace.Stock.Initial = Sol.Initial.face_down(184)
big_terrace.Reserve.Initial = Sol.Initial.face_up(19)
big_terrace.Foundation.Size = 16
big_terrace.Tableau = {
    Size = 11,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 5 and 1 or 0),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = terrace_tableau
        }
    end
}


------

local generals_patience = Sol.copy(terrace)
generals_patience.Info.Name = "General's Patience"
generals_patience.Stock.Initial = Sol.Initial.face_down(87)
generals_patience.Reserve.Initial = Sol.Initial.face_up(13)
generals_patience.Foundation = {
    Size = 8,
    Pile = {
        Rule = { Base = Sol.Rules.Base.FirstFoundation(), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.None(), Limit = 13 }
    }
}


------

local admirals_patience = Sol.copy(generals_patience)
admirals_patience.Info.Name = "Admiral's Patience"
admirals_patience.Info.DeckCount = 4
admirals_patience.Stock.Initial = Sol.Initial.face_down(190)
admirals_patience.Foundation.Size = 16
admirals_patience.Tableau = {
    Size = 11,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 5 and 1 or 0),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = terrace_tableau
        }
    end
}


------

local queen_of_italy = Sol.copy(terrace)
queen_of_italy.Info.Name = "Queen of Italy"
queen_of_italy.on_end_turn = nil


------

local empress_of_italy = Sol.copy(big_terrace)
empress_of_italy.Info.Name = "Empress of Italy"
empress_of_italy.on_end_turn = nil


------

------------------------

Sol.register_game(terrace)
Sol.register_game(admirals_patience)
Sol.register_game(big_terrace)
Sol.register_game(empress_of_italy)
Sol.register_game(generals_patience)
Sol.register_game(queen_of_italy)
