-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local eighteens = {
    Info       = {
        Name         = "Eighteens",
        Family       = "Other",
        DeckCount    = 2,
        DisableHints = true
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(92)
    },
    Reserve    = {
        Size = 12,
        Pile = function(i)
            return {
                Position = { x = i % 4 + 1.5, y = i // 4 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = Sol.Rules.none_none_top
            }
        end
    },
    Foundation = {
        Position = { x = 0, y = 1 },
        Rule = Sol.Rules.none_none_none
    },
    Tableau    = {
        Position = { x = 0, y = 2 },
        Layout   = Sol.Pile.Layout.Row,
        Rule     = Sol.Rules.any_any_top
    },
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        if card.Rank == "Ace" then
            return targetPile.Type == Sol.Pile.Type.Foundation
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    on_drop    = function(game, pile)
        if pile.Type == Sol.Pile.Type.Foundation then
            if game.Tableau[1].IsEmpty then
                Sol.Ops.Deal.to_group(game.Stock[1], game.Reserve, Sol.DealMode.IfEmpty)
            end
            return
        end

        local complete, bust = false, false
        local cards          = pile.Cards

        local faceCardCount  = 0
        local value          = 0
        for _, card in ipairs(cards) do
            local rank = Sol.RankValues[card.Rank]
            if rank > 10 then
                faceCardCount = faceCardCount + 1
            else
                value = value + rank
            end
        end

        if #cards == 4 then
            complete = value == 18 and faceCardCount == 1
            bust     = not complete
        else
            complete = false
            bust     = value > 18 or faceCardCount > 1
        end

        if complete then
            pile:move_cards(game.Foundation[1], 1, #cards, false)
            Sol.Ops.Deal.to_group(game.Stock[1], game.Reserve, Sol.DealMode.IfEmpty)
            game:give_score(#cards * 10)
        elseif bust then
            Sol.Ops.Deal.to_group(pile, game.Reserve, Sol.DealMode.IfEmpty) --TODO: message
        end
    end
}


------

local fifteens = {
    Info = {
        Name         = "Fifteens",
        Family       = "Other",
        DeckCount    = 1,
        DisableHints = true
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(36)
    },
    Reserve = {
        Size = 16,
        Pile = function(i)
            return {
                Position = { x = i % 4 + 1.5, y = i // 4 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = Sol.Rules.none_none_top
            }
        end
    },
    Foundation = {
        Position = { x = 0, y = 1 },
        Rule = Sol.Rules.none_none_none
    },
    Tableau = {
        Position = { x = 0, y = 2 },
        Layout   = Sol.Pile.Layout.Column,
        Rule     = Sol.Rules.any_any_top
    },
    on_drop = function(game, pile)
        local complete, bust = false, false
        local cards          = pile.Cards
        local rank0          = Sol.RankValues[cards[#cards].Rank]

        if #cards > 1 then
            local rank1 = Sol.RankValues[cards[#cards - 1].Rank]
            if rank0 >= 10 or rank1 >= 10 then
                if rank0 < 10 or rank1 < 10 or rank0 ~= rank1 then -- only allow same rank for Ten, Jack, Queen, King
                    bust = true
                elseif #cards == 4 then                            -- complete quadruple
                    complete = true
                end
            else
                local value = 0
                for _, card in ipairs(cards) do
                    value = value + Sol.RankValues[card.Rank]
                end
                if value == 15 then
                    complete = true
                elseif value > 15 then
                    bust = true
                end
            end
        end

        if complete then
            pile:move_cards(game.Foundation[1], 1, #cards, false)
            Sol.Ops.Deal.to_group(game.Stock[1], game.Reserve, Sol.DealMode.IfEmpty)
            game:give_score(#cards * 10)
        elseif bust then
            Sol.Ops.Deal.to_group(pile, game.Reserve, Sol.DealMode.IfEmpty) --TODO: message
        end
    end
}


------

local fourteen = {
    Info       = {
        Name      = "Fourteen",
        Family    = "Other",
        DeckCount = 1
    },
    Foundation = {
        Position = { x = 6.5, y = 0 },
        Rule = Sol.Rules.none_none_none
    },
    Tableau    = {
        Size = 12,
        Pile = function(i)
            return {
                Position = { x = i % 6, y = i // 6 * 2 },
                Initial  = Sol.Initial.face_up(i < 4 and 5 or 4),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.none_none_top
            }
        end
    },
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == Sol.Pile.Type.Tableau and targetCardIndex > 0 then
            if Sol.RankValues[card.Rank] + Sol.RankValues[targetPile.Cards[targetCardIndex].Rank] == 14 then
                return true
            end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    on_drop    = function(game, pile)
        pile:move_cards(game.Foundation[1], pile.CardCount - 1, 2, false)
    end
}


------

local double_fourteen               = Sol.copy(fourteen)
double_fourteen.Info.Name           = "Double Fourteen"
double_fourteen.Info.DeckCount      = 2
double_fourteen.Foundation.Position = { x = 9.5, y = 0 }
double_fourteen.Tableau             = {
    Size = 18,
    Pile = function(i)
        return {
            Position = { x = i % 9, y = i // 9 * 2 },
            Initial  = Sol.Initial.face_up(i < 14 and 6 or 5),
            Layout   = Sol.Pile.Layout.Column,
            Rule     = Sol.Rules.none_none_top
        }
    end
}


------

local patient_pairs = {
    Info       = {
        Name      = "Patient Pairs",
        Family    = "Other",
        DeckCount = 1
    },
    Foundation = {
        Position = { x = 7.5, y = 0 },
        Rule = Sol.Rules.none_none_none
    },
    Tableau    = {
        Size = 13,
        Pile = function(i)
            return {
                Position = { x = i % 7, y = i // 7 * 1.5 },
                Initial  = Sol.Initial.top_face_up(4),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    on_drop    = function(game, pile)
        pile:move_cards(game.Foundation[1], pile.CardCount - 1, 2, false)
    end
}


------

local patient_pairs_open     = Sol.copy(patient_pairs)
patient_pairs_open.Info.Name = "Patient Pairs (Open)"
patient_pairs_open.Tableau   = {
    Size = 13,
    Pile = function(i)
        return {
            Position = { x = i % 7, y = i // 7 * 2 },
            Initial  = Sol.Initial.face_up(4),
            Layout   = Sol.Pile.Layout.Column,
            Rule     = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.Top() }
        }
    end
}


------

local simple_pairs = {
    Info       = {
        Name      = "Simple Pairs",
        Family    = "Other",
        DeckCount = 1
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(43)
    },
    Foundation = {
        Position = { x = 1, y = 0 },
        Rule = Sol.Rules.none_none_none
    },
    Tableau    = {
        Size = 9,
        Pile = function(i)
            return {
                Position = { x = i % 3, y = i // 3 + 1 },
                Initial  = Sol.Initial.face_up(1),
                Rule     = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    on_drop    = function(game, pile)
        pile:move_cards(game.Foundation[1], pile.CardCount - 1, 2, false)
        Sol.Ops.Deal.to_group(game.Stock[1], game.Tableau, Sol.DealMode.IfEmpty)
    end
}


------

local triple_alliance = {
    Info        = {
        Name      = "Triple Alliance",
        Family    = "Other",
        DeckCount = 1,
        Objective = "AllCardsButOneToFoundation"
    },
    FreeCell    = {
        Size = 3,
        Pile = function(i)
            return {
                Position = { x = i, y = 0 },
                Rule     = {
                    Base = Sol.Rules.Base.Any(),
                    Build = Sol.Rules.Build.None(),
                    Move = Sol.Rules.Move.None()
                }
            }
        end
    },
    Foundation  = {
        Position = { x = 10, y = 0 },
        Rule = Sol.Rules.none_none_none
    },
    Tableau     = {
        Size = 18,
        Pile = function(i)
            return {
                Position = { x = i % 5 * 2, y = i // 5 + 1 },
                Initial  = Sol.Initial.face_up(i < 16 and 3 or 2),
                Layout   = Sol.Pile.Layout.Row,
                Rule     = Sol.Rules.none_none_top
            }
        end
    },
    can_play    = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == Sol.Pile.Type.FreeCell and targetPile.IsEmpty then --build consecutive ranks on freecells
            local freeCell = game.FreeCell
            local ranks = {}
            if not freeCell[1].IsEmpty then
                ranks[1] = freeCell[1].Cards[1].Rank
                ranks[2] = Sol.get_rank(ranks[1], 1, true)
                ranks[3] = Sol.get_rank(ranks[1], 2, true)
            elseif not freeCell[2].IsEmpty then
                ranks[2] = freeCell[2].Cards[1].Rank
                ranks[1] = Sol.get_rank(ranks[2], -1, true)
                ranks[3] = Sol.get_rank(ranks[2], 1, true)
            elseif not freeCell[3].IsEmpty then
                ranks[3] = freeCell[3].Cards[1].Rank
                ranks[1] = Sol.get_rank(ranks[3], -2, true)
                ranks[2] = Sol.get_rank(ranks[3], -1, true)
            else
                return true
            end

            return card.Rank == ranks[targetPile.Index]
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    on_end_turn = function(game)
        --move complete freecells to foundation
        local freeCell = game.FreeCell
        local foundation = game.Foundation
        if not freeCell[1].IsEmpty and not freeCell[2].IsEmpty and not freeCell[3].IsEmpty then
            freeCell[1]:move_cards(foundation[1], 1, 1, false)
            freeCell[2]:move_cards(foundation[1], 1, 1, false)
            freeCell[3]:move_cards(foundation[1], 1, 1, false)
            game:give_score(30)
        end
    end
}


------

local triple_alliance_2_decks          = Sol.copy(triple_alliance)
triple_alliance_2_decks.Info.Name      = "Triple Alliance (2 decks)"
triple_alliance_2_decks.Info.DeckCount = 2
triple_alliance_2_decks.Info.Objective = "AllCardsButTwoToFoundation"
triple_alliance_2_decks.Tableau.Pile   = function(i)
    return {
        Position = { x = i % 5 * 2, y = i // 5 + 1 },
        Initial  = Sol.Initial.face_up(i < 16 and 6 or 4),
        Layout   = Sol.Pile.Layout.Row,
        Rule     = Sol.Rules.none_none_top
    }
end


------

------------------------

Sol.register_game(double_fourteen)
Sol.register_game(eighteens)
Sol.register_game(fifteens)
Sol.register_game(fourteen)
Sol.register_game(patient_pairs)
Sol.register_game(patient_pairs_open)
Sol.register_game(simple_pairs)
Sol.register_game(triple_alliance)
Sol.register_game(triple_alliance_2_decks)
