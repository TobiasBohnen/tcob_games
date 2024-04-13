-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local function golf_check_state(game)
    local foundation = game.Foundation
    if foundation[1].IsEmpty then return "Running" end

    local dead    = true -- detect dead game
    local success = true -- detect win state
    for _, tableau in ipairs(game.Tableau) do
        if not tableau.IsEmpty then
            success = false
            if game:can_play(foundation[1], #foundation[1].Cards, tableau.Cards[#tableau.Cards], 1) then
                dead = false
                break
            end
        end
    end


    if success then return "Success" end
    if not game.Stock[1].IsEmpty then return "Running" end
    if dead then return "Failure" end

    return "Running"
end

------

local golf                                = {
    Info       = {
        Name      = "Golf",
        Family    = "Golf",
        DeckCount = 1
    },
    Stock      = {
        Initial = Sol.Initial.face_down(16)
    },
    Foundation = {
        Initial = Sol.Initial.face_up(1),
        Layout  = "Squared",
        Rule    = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownByRank(), Move = Sol.Rules.Move.None() }
    },
    Tableau    = {
        Size = 7,
        Pile = {
            Initial = Sol.Initial.face_up(5),
            Layout = "Column",
            Rule = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.None(), Move = Sol.Rules.Move.Top() }
        }
    },
    do_deal    = function(game)
        return Sol.Ops.Deal.to_pile(game.Stock[1], game.Foundation[1], 1)
    end,
    get_state  = golf_check_state,
    on_init    = Sol.Layout.golf
}

------

local double_golf                         = Sol.copy(golf)
double_golf.Info.Name                     = "Double Golf"
double_golf.Info.DeckCount                = 2
double_golf.Stock.Initial                 = Sol.Initial.face_down(40)
double_golf.Tableau.Size                  = 9
double_golf.Tableau.Pile.Initial          = Sol.Initial.top_face_up(7)

------

local putt_putt                           = Sol.copy(golf)
putt_putt.Info.Name                       = "Putt Putt"
putt_putt.Foundation                      = {
    Initial = Sol.Initial.face_up(1),
    Layout  = "Squared",
    Rule    = { Base = Sol.Rules.Base.None, Build = Sol.Rules.Build.UpOrDownByRank(true), Move = Sol.Rules.Move.None() }
}

------

local double_putt                         = Sol.copy(putt_putt)
double_putt.Info.Name                     = "Double Putt"
double_putt.Info.DeckCount                = 2
double_putt.Stock.Initial                 = Sol.Initial.face_down(40)
double_putt.Tableau.Size                  = 9
double_putt.Tableau.Pile.Initial          = Sol.Initial.top_face_up(7)

------

local all_in_a_row                        = {
    Info       = {
        Name      = "All in a Row",
        Family    = "Golf",
        DeckCount = 1
    },
    Foundation = {
        Layout = "Squared",
        Rule   = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.UpOrDownByRank(true), Move = Sol.Rules.Move.None() }
    },
    Tableau    = {
        Size = 13,
        Pile = {
            Initial = Sol.Initial.face_up(4),
            Layout = "Column",
            Rule = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.None(), Move = Sol.Rules.Move.Top() }
        }
    },
    on_init    = Sol.Layout.golf
}

------
local black_hole_pos                      = {
    { 0, 0 }, { 2, 0 }, { 4, 0 }, { 6, 0 }, { 8, 0 },
    { 0, 1 }, { 2, 1 }, --[[ --]] { 6, 1 }, { 8, 1 },
    { 0, 2 }, { 2, 2 }, --[[ --]] { 6, 2 }, { 8, 2 },
    { 0, 3 }, { 2, 3 }, { 4, 3 }, { 6, 3 } --[[ --]]
}

local black_hole                          = {
    Info              = {
        Name      = "Black Hole",
        Family    = "Golf",
        DeckCount = 1
    },
    Foundation        = {
        Position = { x = 4, y = 1.5 },
        Rule     = { Build = Sol.Rules.Build.UpOrDownByRank(true), Move = Sol.Rules.Move.None() }
    },
    Tableau           = {
        Size = 17,
        Pile = function(i)
            return {
                Position = { x = black_hole_pos[i + 1][1], y = black_hole_pos[i + 1][2] },
                Initial = Sol.Initial.face_up(3),
                Layout = "Row",
                Rule = { Build = Sol.Rules.Build.None() }
            }
        end
    },
    on_before_shuffle = function(game, card)
        if card.Rank == "Ace" and card.Suit == "Spades" then
            return game.PlaceTop(card, game.Foundation, true)
        end

        return false
    end
}

------

local dolphin                             = {
    Info = {
        Name      = "Dolphin",
        Family    = "Golf",
        DeckCount = 1
    },
    FreeCell = {
        Size = 4,
        Pile = { Rule = Sol.Rules.any_none_top }
    },
    Foundation = {
        Layout = "Squared",
        Rule   = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.UpOrDownByRank(true), Move = Sol.Rules.Move.Top() }
    },
    Tableau = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 4 and 7 or 6),
                Layout = "Column",
                Rule = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.None(), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    on_init = Sol.Layout.free_cell
}

------

local double_dolphin                      = Sol.copy(dolphin)
double_dolphin.Info.Name                  = "Double Dolphin"
double_dolphin.Info.DeckCount             = 2
double_dolphin.FreeCell.Size              = 5
double_dolphin.Tableau                    = {
    Size = 10,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 4 and 11 or 10),
            Layout = "Column",
            Rule = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.None(), Move = Sol.Rules.Move.Top() }
        }
    end
}

------

local flake                               = {
    Info = {
        Name      = "Flake",
        Family    = "Golf",
        DeckCount = 1
    },
    Foundation = {
        Layout = "Squared",
        Rule   = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.UpOrDownByRank(true), Move = Sol.Rules.Move.None() }
    },
    Tableau = {
        Size = 6,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 4 and 9 or 8),
                Layout = "Column",
                Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.UpOrDownByRank(true), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    on_init = Sol.Layout.free_cell
}

------

local flake_2_decks                       = Sol.copy(flake)
flake_2_decks.Info.Name                   = "Flake (2 decks)"
flake_2_decks.Info.DeckCount              = 2
flake_2_decks.Tableau                     = {
    Size = 8,
    Pile = {
        Initial = Sol.Initial.face_up(13),
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.UpOrDownByRank(true), Move = Sol.Rules.Move.Top() }
    }
}

------

local robert                              = {
    Info       = {
        Name      = "Robert",
        Family    = "Golf",
        DeckCount = 1,
        Redeals   = 2
    },
    Stock      = {
        Position = { x = 0, y = 1 },
        Initial = Sol.Initial.face_down(51)
    },
    Waste      = {
        Position = { x = 1, y = 1 }
    },
    Foundation = {
        Position = { x = 0.5, y = 0 },
        Initial  = Sol.Initial.face_up(1),
        Layout   = "Squared",
        Rule     = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownByRank(true), Move = Sol.Rules.Move.None() }
    },
    do_deal    = Sol.Ops.Deal.stock_to_waste,
    do_redeal  = Sol.Ops.Redeal.waste_to_stock
}

------

local wasatch                             = Sol.copy(robert)
wasatch.Info.Name                         = "Wasatch"
wasatch.Info.Redeals                      = -1
wasatch.do_deal                           = Sol.Ops.Deal.stock_to_waste_by_3

------

local bobby                               = Sol.copy(robert)
bobby.Info.Name                           = "Bobby"
bobby.Foundation                          = {
    Size = 2,
    Pile = function(i)
        return {
            Position = { x = i, y = 0 },
            Initial  = Sol.Initial.face_up(1 - i),
            Layout   = "Squared",
            Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.UpOrDownByRank(true), Move = Sol.Rules.Move.None() }
        }
    end
}

------

local dead_king_golf                      = Sol.copy(golf)
dead_king_golf.Info.Name                  = "Dead King Golf"
dead_king_golf.Foundation.Rule.Build.Func = function(_, target, card)
    if target.Rank == "King" then return false end
    return Sol.get_rank(target.Rank, 1, false) == card.Rank or Sol.get_rank(target.Rank, -1, false) == card.Rank
end

------

local uintah                              = {
    Info              = {
        Name      = "Uintah",
        Family    = "Golf",
        DeckCount = 1,
        Redeals   = -1
    },
    Stock             = {
        Position = { x = 1, y = 1 },
        Initial = Sol.Initial.face_down(48)
    },
    Waste             = {
        Position = { x = 2, y = 1 }
    },
    Foundation        = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i, y = 0 },
                Layout   = "Squared",
                Rule     = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownInColor(true), Move = Sol.Rules.Move.None() }
            }
        end
    },
    do_deal           = Sol.Ops.Deal.stock_to_waste_by_3,
    do_redeal         = Sol.Ops.Redeal.waste_to_stock,
    on_before_shuffle = function(game, card)
        if card.Suit == "Clubs" then
            return game.PlaceTop(card, game.Foundation[1], true)
        end
        if card.Suit == "Spades" then
            return game.PlaceTop(card, game.Foundation[2], true)
        end
        if card.Suit == "Hearts" then
            return game.PlaceTop(card, game.Foundation[3], true)
        end
        if card.Suit == "Diamonds" then
            return game.PlaceTop(card, game.Foundation[4], true)
        end
        return false
    end
}

------

local double_uintah                       = Sol.copy(uintah)
double_uintah.Info.Name                   = "Double Uintah"
double_uintah.Info.DeckCount              = 2
double_uintah.Stock                       = {
    Position = { x = 3, y = 1 },
    Initial = Sol.Initial.face_down(96)
}
double_uintah.Waste                       = { Position = { x = 4, y = 1 } }
double_uintah.Foundation.Size             = 8
double_uintah.on_before_shuffle           = function(game, card)
    if card.Suit == "Clubs" then
        return game.PlaceTop(card, game.Foundation[1], true) or game.PlaceTop(card, game.Foundation[2], true)
    end
    if card.Suit == "Spades" then
        return game.PlaceTop(card, game.Foundation[3], true) or game.PlaceTop(card, game.Foundation[4], true)
    end
    if card.Suit == "Hearts" then
        return game.PlaceTop(card, game.Foundation[5], true) or game.PlaceTop(card, game.Foundation[6], true)
    end
    if card.Suit == "Diamonds" then
        return game.PlaceTop(card, game.Foundation[7], true) or game.PlaceTop(card, game.Foundation[8], true)
    end
    return false
end

------------------------

Sol.register_game(golf)
Sol.register_game(all_in_a_row)
Sol.register_game(black_hole)
Sol.register_game(bobby)
Sol.register_game(dead_king_golf)
Sol.register_game(dolphin)
Sol.register_game(double_dolphin)
Sol.register_game(double_golf)
Sol.register_game(double_putt)
Sol.register_game(double_uintah)
Sol.register_game(flake)
Sol.register_game(flake_2_decks)
Sol.register_game(putt_putt)
Sol.register_game(robert)
Sol.register_game(uintah)
Sol.register_game(wasatch)
