-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local picture_gallery_base <const> = {
    foundation = function(i, columns)
        if i == columns * 3 then
            return {
                Position = { x = columns + 0.5, y = 0 },
                Rule     = { Base = Sol.Rules.Base.Ace(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.None() },
            }
        end

        local base = {}
        if i < columns then
            base = Sol.Rules.Base.Ranks({ "Four" })
        elseif i < columns * 2 then
            base = Sol.Rules.Base.Ranks({ "Three" })
        else
            base = Sol.Rules.Base.Ranks({ "Two" })
        end
        return {
            Position = { x = i % columns, y = i // columns },
            Initial  = Sol.Initial.face_up(1),
            Rule     = { Base = base, Build = Sol.Rules.Build.UpInSuit(false, 3), Move = Sol.Rules.Move.Top() }
        }
    end,
    tableau = function(i, columns)
        return {
            Position = { x = i % columns, y = 3.25 },
            Initial  = Sol.Initial.face_up(1),
            Layout   = Sol.Pile.Layout.Column,
            Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.None(), Move = Sol.Rules.Move.Top() }
        }
    end,
    can_play = function(targetPile, columns)
        if targetPile.Type == Sol.Pile.Type.Foundation and not targetPile.IsEmpty then
            local i = targetPile.Index
            local firstCard = targetPile.Cards[1]
            if i >= 1 and i <= columns and firstCard.Rank ~= "Four" then
                return false
            elseif i >= columns + 1 and i <= columns * 2 and firstCard.Rank ~= "Three" then
                return false
            elseif i >= columns * 2 + 1 and i <= columns * 3 and firstCard.Rank ~= "Two" then
                return false
            end
        end
        return true
    end
}


------

local picture_gallery = { --TODO: enable "Foundation to Foundation" hints
    Info        = {
        Name      = "Picture Gallery",
        Family    = "PictureGallery",
        DeckCount = 2
    },
    Stock       = {
        Position = { x = 8.5, y = 3.25 },
        Initial = Sol.Initial.face_down(72)
    },
    Foundation  = {
        Size = 25,
        Pile = function(i) return picture_gallery_base.foundation(i, 8) end
    },
    Tableau     = {
        Size = 8,
        Pile = function(i) return picture_gallery_base.tableau(i, 8) end
    },
    can_play    = function(game, targetPile, targetCardIndex, card, numCards)
        if not picture_gallery_base.can_play(targetPile, 8) then return false end
        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    deal        = Sol.Ops.Deal.stock_to_tableau,
    on_end_turn = function(game)
        Sol.Ops.Deal.to_group(game.Stock[1], game.Tableau, Sol.DealMode.IfEmpty)
    end
}


------

local big_picture_gallery          = Sol.copy(picture_gallery)
big_picture_gallery.Info.Name      = "Big Picture Gallery"
big_picture_gallery.Info.DeckCount = 3
big_picture_gallery.Stock          = {
    Position = { x = 12.5, y = 3.25 },
    Initial = Sol.Initial.face_down(108)
}
big_picture_gallery.Foundation     = {
    Size = 37,
    Pile = function(i) return picture_gallery_base.foundation(i, 12) end
}
big_picture_gallery.Tableau        = {
    Size = 12,
    Pile = function(i) return picture_gallery_base.tableau(i, 12) end
}
big_picture_gallery.can_play       = function(game, targetPile, targetCardIndex, card, numCards)
    if not picture_gallery_base.can_play(targetPile, 12) then return false end
    return game:can_play(targetPile, targetCardIndex, card, numCards)
end


------

------------------------

Sol.register_game(picture_gallery)
Sol.register_game(big_picture_gallery)
