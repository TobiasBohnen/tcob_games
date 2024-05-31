-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

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
        Pile = {
            Position = { x = 10, y = 0 },
            Rule = Sol.Rules.none_none_none
        }
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

Sol.register_game(triple_alliance)
Sol.register_game(triple_alliance_2_decks)
