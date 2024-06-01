-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local new_york = {
    Info        = {
        Name      = "New York",
        Family    = "Fan",
        DeckCount = 2
    },
    Stock       = {
        Position = { x = 0, y = 0 },
        Initial  = Sol.Initial.top_face_up(95),
        Rule     = Sol.Rules.none_none_top
    },
    FreeCell    = {
        Size = 3,
        Pile = function(i)
            return {
                Position = { x = 0, y = i + 1 },
                Rule     = Sol.Rules.any_any_top
            }
        end
    },
    Foundation  = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Initial  = Sol.Initial.face_up(i == 0 and 1 or 0),
                Rule     = Sol.Rules.ff_upsuit_none
            }
        end
    },
    Tableau     = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 1 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownAlternateColors(true), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    on_end_turn = function(game)
        game.Stock[1]:flip_up_top_card()
    end,
    can_play    = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == Sol.Pile.Type.FreeCell then -- freecells only accept stock cards
            if game:find_pile(card).Type ~= Sol.Pile.Type.Stock then return false end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end
}


------

local gotham         = Sol.copy(new_york)
gotham.Info.Name     = "Gotham"
gotham.Stock.Initial = Sol.Initial.top_face_up(79)
gotham.Tableau.Pile  = function(i)
    return {
        Position = { x = i + 2, y = 1 },
        Initial  = Sol.Initial.face_up(3),
        Layout   = Sol.Pile.Layout.Column,
        Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(true), Move = Sol.Rules.Move.InSeq() }
    }
end
gotham.can_play      = function(game, targetPile, targetCardIndex, card, numCards)
    local srcPile = game:find_pile(card)

    if targetPile.Type == Sol.Pile.Type.FreeCell then -- freecells only accept stock cards
        if srcPile.Type ~= Sol.Pile.Type.Stock then return false end
    end

    if targetPile.Type == Sol.Pile.Type.Tableau and targetPile.IsEmpty then -- empty piles can only be filled from stock or freecell
        if srcPile.Type ~= Sol.Pile.Type.Stock and srcPile.Type ~= Sol.Pile.Type.FreeCell then return false end
    end

    return game:can_play(targetPile, targetCardIndex, card, numCards)
end


------

local interment = {
    Info = {
        Name      = "Interment",
        Family    = "Fan",
        DeckCount = 2
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial  = Sol.Initial.top_face_up(78),
        Rule     = Sol.Rules.none_none_top
    },
    Reserve = {
        Size = 6,
        Pile = function(i)
            if i == 0 then
                return {
                    Position = { x = 10, y = i },
                    Initial  = Sol.Initial.face_down(13)
                }
            else
                return {
                    Position = { x = 10, y = i },
                    Initial  = Sol.Initial.face_up(1),
                    Rule     = Sol.Rules.none_none_top
                }
            end
        end
    },
    FreeCell = {
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
                Position = { x = i + 1.5, y = 0 },
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 1.5, y = 1 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownInSuit(), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    can_play = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == Sol.Pile.Type.FreeCell then -- freecells only accept stock cards
            if game:find_pile(card).Type ~= Sol.Pile.Type.Stock then return false end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    on_end_turn = function(game)
        Sol.Ops.Deal.to_group(game.Stock[1], game.Tableau, Sol.DealMode.IfEmpty)
        game.Stock[1]:flip_up_top_card()
        Sol.Ops.Deal.to_group(game.Reserve[1], game.Reserve, Sol.DealMode.IfEmpty)
        game.Reserve[1]:flip_down_top_card()
    end
}

------

------------------------

Sol.register_game(new_york)
Sol.register_game(gotham)
Sol.register_game(interment)
