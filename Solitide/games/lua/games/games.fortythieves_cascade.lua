-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local cascade = {
    Info = {
        Name      = "Cascade",
        Family    = "FortyThieves",
        DeckCount = 1
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(17)
    },
    Waste = { Position = { x = 1, y = 0 } },
    FreeCell = {
        Size = 2,
        Pile = function(i)
            return {
                Position = { x = 0, y = i + 1 },
                Rule = Sol.Rules.any_none_top
            }
        end
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Rule = Sol.Rules.ace_upac_top
            }
        end
    },
    Tableau = {
        Size = 7,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 1 },
                Initial = Sol.Initial.face_up(5),
                Layout = "Column",
                Rule = Sol.Rules.any_downac_top
            }
        end
    },
    deal = Sol.Ops.Deal.stock_to_waste,
    redeal = Sol.Ops.Redeal.waste_to_stock
}


------

local jacks_in_the_box = {
    Info = {
        Name      = "Jacks in the Box",
        Family    = "FortyThieves",
        DeckCount = 2
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(90)
    },
    Waste = { Position = { x = 1, y = 0 } },
    FreeCell = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = 0, y = i + 1 },
                Rule = Sol.Rules.any_none_top
            }
        end
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Rule = { Base = Sol.Rules.Base.Ranks({ "Jack" }), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.Top(), Limit = 13 }
            }
        end
    },
    Tableau = {
        Size = 6,
        Pile = function(i)
            return {
                Position = { x = i + 4, y = 1 },
                Initial = Sol.Initial.face_up(1),
                Layout = "Column",
                Rule = Sol.Rules.any_downsuit_top
            }
        end
    },
    deal = Sol.Ops.Deal.stock_to_waste,
    redeal = Sol.Ops.Redeal.waste_to_stock,
    on_before_shuffle = function(game, card)
        if card.Rank == "Jack" then
            return game.PlaceTop(card, game.Foundation, true)
        end
        return false
    end
}


------

local squadron = {
    Info = {
        Name      = "Squadron",
        Family    = "FortyThieves",
        DeckCount = 2
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(61)
    },
    Waste = { Position = { x = 1, y = 0 } },
    FreeCell = {
        Size = 3,
        Pile = function(i)
            return {
                Position = { x = 0, y = i + 1 },
                Initial = Sol.Initial.face_up(1),
                Rule = Sol.Rules.any_none_top
            }
        end
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Rule = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau = {
        Size = 10,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 1 },
                Initial = Sol.Initial.face_up(4),
                Layout = "Column",
                Rule = Sol.Rules.any_downsuit_top
            }
        end
    },
    deal = Sol.Ops.Deal.stock_to_waste,
    redeal = Sol.Ops.Redeal.waste_to_stock
}


------

------------------------

Sol.register_game(cascade)
Sol.register_game(jacks_in_the_box)
Sol.register_game(squadron)
