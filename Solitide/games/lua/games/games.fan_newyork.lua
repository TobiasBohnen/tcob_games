-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local new_york = {
    Info       = {
        Name      = "New York",
        Family    = "Fan",
        DeckCount = 2
    },
    Reserve    = {
        Position = { x = 0, y = 0 },
        Initial  = Sol.Initial.top_face_up(95),
        Rule     = Sol.Rules.none_none_top
    },
    FreeCell   = {
        Size = 3,
        Pile = function(i)
            return {
                Position = { x = 0, y = i + 1 },
                Rule     = Sol.Rules.any_any_top
            }
        end
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Initial  = Sol.Initial.face_up(i == 0 and 1 or 0),
                Rule     = Sol.Rules.ff_upsuit_none_l13
            }
        end
    },
    Tableau    = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 1 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = "Column",
                Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownAlternateColors(true), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == "FreeCell" then -- freecells only accept reserve cards
            if game:find_pile(card).Type ~= "Reserve" then return false end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end
}


------

local gotham           = Sol.copy(new_york)
gotham.Info.Name       = "Gotham"
gotham.Reserve.Initial = Sol.Initial.top_face_up(79)
gotham.Tableau.Pile    = function(i)
    return {
        Position = { x = i + 2, y = 1 },
        Initial  = Sol.Initial.face_up(3),
        Layout   = "Column",
        Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(true), Move = Sol.Rules.Move.InSeq() }
    }
end
gotham.can_play        = function(game, targetPile, targetCardIndex, card, numCards)
    local srcPile = game:find_pile(card)

    if targetPile.Type == "FreeCell" then -- freecells only accept reserve cards
        if srcPile.Type ~= "Reserve" then return false end
    end

    if targetPile.Type == "Tableau" and targetPile.IsEmpty then -- empty piles can only be filled from reserve or freecell
        if srcPile.Type ~= "Reserve" and srcPile.Type ~= "FreeCell" then return false end
    end

    return game:can_play(targetPile, targetCardIndex, card, numCards)
end


------

------------------------

Sol.register_game(new_york)
Sol.register_game(gotham)
