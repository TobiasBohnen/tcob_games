-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

------

local take_away = {
    Info       = {
        Name      = "Take Away",
        Family    = "Golf",
        DeckCount = 1
    },
    Foundation = {
        Size = 6,
        Pile = {
            Layout = Sol.Pile.Layout.Squared,
            Rule   = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.UpOrDownByRank(true), Move = Sol.Rules.Move.None(), Limit = -1 }
        }
    },
    Tableau    = {
        Size = 4,
        Pile = {
            Initial = Sol.Initial.face_up(13),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.None(), Move = Sol.Rules.Move.Top() }
        }
    },
    on_init    = Sol.Layout.canister
}


------

local take_away_8           = Sol.copy(take_away)
take_away_8.Info.Name       = "Take Away 8"
take_away_8.Foundation.Size = 8


------

local striptease = {
    Info              = {
        Name      = "Striptease",
        Family    = "Golf",
        DeckCount = 1,
        Objective = "AllCardsButFourToFoundation"
    },
    Foundation        = {
        Size = 6,
        Pile = {
            Layout = Sol.Pile.Layout.Squared,
            Rule   = {
                Base = Sol.Rules.Base.Any(),
                Build = {
                    Hint = { "UpOrDownByRank" },
                    Func = function(_, base, drop)
                        if base.Rank == "Queen" or drop.Rank == "Queen" then return false end
                        if base.Rank == "Jack" and drop.Rank == "King" then return true end
                        if base.Rank == "King" and drop.Rank == "Jack" then return true end
                        return Sol.get_rank(base.Rank, 1, true) == drop.Rank or Sol.get_rank(base.Rank, -1, true) == drop.Rank
                    end
                },
                Move = Sol.Rules.Move.None(),
                Limit = -1
            }
        }
    },
    Tableau           = {
        Size = 4,
        Pile = {
            Initial = { false, true, true, true, true, true, true, true, true, true, true, true },
            Layout  = Sol.Pile.Layout.Column,
            Rule    = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.None(), Move = Sol.Rules.Move.Top() }
        }
    },
    on_before_shuffle = function(game, card)
        if card.Rank == "Queen" then
            return game.PlaceTop(card, game.Tableau, true)
        end
        return false
    end,
    on_end_turn       = function(game)
        -- Flip face-down cards if there are no other cards on the tableau
        local check = true
        for _, tab in ipairs(game.Tableau) do
            if tab.CardCount > 2 then
                check = false
                break
            end
        end
        for _, tab in ipairs(game.Tableau) do
            tab:flip_cards({ true, check })
        end
    end,
    on_init           = Sol.Layout.canister
}


------

------------------------

Sol.register_game(take_away)
Sol.register_game(striptease)
Sol.register_game(take_away_8)
