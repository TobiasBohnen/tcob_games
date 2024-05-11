-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT


local four_seasons_fou_pos <const> = { { x = 0, y = 1 }, { x = 0, y = 3 }, { x = 4, y = 1 }, { x = 4, y = 3 } }
local four_seasons_tab_pos <const> = { { x = 2, y = 1 }, { x = 1, y = 2 }, { x = 2, y = 2 }, { x = 3, y = 2 }, { x = 2, y = 3 } }

local four_seasons = {
    Info = {
        Name = "Four Seasons",
        Family = "Other",
        DeckCount = 1
    },
    Stock = {
        Position = { x = 1.5, y = 0 },
        Initial = Sol.Initial.face_down(46)
    },
    Waste = { Position = { x = 2.5, y = 0 } },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = four_seasons_fou_pos[i + 1],
                Initial = Sol.Initial.face_up(i == 0 and 1 or 0),
                Rule = { Base = Sol.Rules.Base.FirstFoundation(0), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.None() }
            }
        end
    },
    Tableau = {
        Size = 5,
        Pile = function(i)
            return {
                Position = four_seasons_tab_pos[i + 1],
                Initial = Sol.Initial.face_up(1),
                Layout = "Squared",
                Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(true), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    deal = Sol.Ops.Deal.stock_to_waste
}


------

local czarina = Sol.copy(four_seasons)
czarina.Info.Name = "Czarina"
czarina.on_end_turn = Sol.Ops.Deal.waste_or_stock_to_empty_tableau


------

local corners = {
    Info = {
        Name = "Corners",
        Family = "Other",
        DeckCount = 1,
        Redeals = 2
    },
    Stock = {
        Position = { x = 1.5, y = 0 },
        Initial = Sol.Initial.face_down(43)
    },
    Waste = { Position = { x = 2.5, y = 0 } },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = four_seasons_fou_pos[i + 1],
                Rule = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.None() }
            }
        end
    },
    Tableau = {
        Size = 5,
        Pile = function(i)
            return {
                Position = four_seasons_tab_pos[i + 1],
                Initial = Sol.Initial.face_up(1),
                Layout = "Squared",
                Rule = Sol.Rules.any_none_top
            }
        end
    },
    on_before_shuffle = function(game, card)
        local dest = { Hearts = 1, Diamonds = 2, Spades = 3, Clubs = 4 }
        local foundation = game.Foundation[dest[card.Suit]]
        if foundation.IsEmpty then
            return game.PlaceTop(card, foundation, true)
        end

        return false
    end,
    deal = Sol.Ops.Deal.stock_to_waste,
    on_end_turn = Sol.Ops.Deal.waste_or_stock_to_empty_tableau
}


------

local simplicity_fou_pos <const> = { { x = 0, y = 0 }, { x = 0, y = 3 }, { x = 7, y = 0 }, { x = 7, y = 3 } }

local simplicity = {
    Info = {
        Name = "Simplicity",
        Family = "Other",
        DeckCount = 1
    },
    Stock = {
        Position = { x = 3, y = 0 },
        Initial = Sol.Initial.face_down(39)
    },
    Waste = { Position = { x = 4, y = 0 } },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = simplicity_fou_pos[i + 1],
                Initial = Sol.Initial.face_up(i == 0 and 1 or 0),
                Rule = { Base = Sol.Rules.Base.FirstFoundation(0), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.None() }
            }
        end
    },
    Tableau = {
        Size = 12,
        Pile = function(i)
            return {
                Position = { x = i % 6 + 1, y = i // 6 + 1 },
                Initial = Sol.Initial.face_up(1),
                Layout = "Squared",
                Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownAlternateColors(true), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    deal = Sol.Ops.Deal.stock_to_waste
}


------

------------------------

Sol.register_game(four_seasons)
Sol.register_game(corners)
Sol.register_game(czarina)
Sol.register_game(simplicity)
