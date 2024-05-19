-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local snake = {
    Info              = {
        Name      = "Snake",
        Family    = "FreeCell",
        DeckCount = 2
    },
    FreeCell          = {
        Size = 7,
        Pile = function(i)
            return {
                Position = { x = i, y = 0 },
                Rule     = Sol.Rules.any_none_top
            }
        end
    },
    Foundation        = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i % 2 + 9, y = i // 2 },
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau           = {
        Size = 9,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial  = Sol.Initial.face_up(i == 0 and 96 or 0),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.DownAlternateColors(), Move = Sol.Rules.Move.SuperMove() }
            }
        end
    },
    on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation,
    on_after_shuffle  = function(game)
        -- kings start new pile
        local tableau  = game.Tableau
        local tabIdx   = 2
        local tableau1 = tableau[1]
        for cardIdx = tableau1.CardCount, 2, -1 do
            local card = tableau1.Cards[cardIdx]
            if card.Rank == "King" then
                tableau1:move_cards(tableau[tabIdx], cardIdx, tableau1.CardCount - cardIdx + 1, false)
                tabIdx = tabIdx + 1
            end
        end
    end
}


------

local cats_tail             = Sol.copy(snake)
cats_tail.Info.Name         = "Cat's Tail"
cats_tail.Tableau.Pile      = function(i)
    return {
        Position = { x = i, y = 1 },
        Initial  = Sol.Initial.face_up(i == 0 and 104 or 0),
        Layout   = Sol.Pile.Layout.Column,
        Rule     = Sol.Rules.none_downac_inseq
    }
end
cats_tail.on_before_shuffle = nil


------

local kings             = Sol.copy(snake)
kings.Info.Name         = "Kings"
kings.FreeCell.Size     = 8
kings.Tableau           = {
    Size = 8,
    Pile = function(i)
        return {
            Position = { x = i, y = 1 },
            Initial  = Sol.Initial.face_up(i == 0 and 103 or 0),
            Layout   = Sol.Pile.Layout.Column,
            Rule     = Sol.Rules.none_downac_inseq
        }
    end
}
kings.on_before_shuffle = function(game, card)
    if card.Rank == "King" then
        return game.PlaceTop(card, game.Tableau[1], true)
    end

    return false
end


------

local retinue             = Sol.copy(kings)
retinue.Info.Name         = "Retinue"
retinue.Tableau.Pile      = function(i)
    return {
        Position = { x = i, y = 1 },
        Initial  = Sol.Initial.face_up(i == 0 and 95 or 0),
        Layout   = Sol.Pile.Layout.Column,
        Rule     = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.DownAlternateColors(), Move = Sol.Rules.Move.SuperMove() }
    }
end
retinue.on_before_shuffle = function(game, card)
    return kings.on_before_shuffle(game, card) or Sol.Ops.Shuffle.ace_to_foundation(game, card)
end


------

------------------------

Sol.register_game(snake)
Sol.register_game(cats_tail)
Sol.register_game(kings)
Sol.register_game(retinue)
