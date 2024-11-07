-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local scorpion_initial = function(x, i)
    return i < x and { false, false, false, true, true, true, true } or Sol.Initial.face_up(7)
end

local scorpion         = {
    Info       = {
        Name      = "Scorpion",
        Family    = "Spider",
        DeckCount = 1
    },
    Stock      = {
        Initial = Sol.Initial.face_down(3)
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.spider_foundation }
    },
    Tableau    = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = scorpion_initial(4, i),
                Layout  = Sol.Pile.Layout.Column,
                Rule    = Sol.Rules.king_downsuit_faceup
            }
        end
    },
    deal       = function(game)
        for _, tableau in ipairs(game.Tableau) do
            if tableau.IsEmpty then return false end
        end

        return Sol.Ops.Deal.stock_to_tableau(game)
    end,
    on_init    = Sol.Layout.klondike
}


------

local double_scorpion           = Sol.copy(scorpion)
double_scorpion.Info.Name       = "Double Scorpion"
double_scorpion.Info.DeckCount  = 2
double_scorpion.Stock           = nil
double_scorpion.Foundation.Size = 8
double_scorpion.Tableau         = {
    Size = 10,
    Pile = function(i)
        local initial = Sol.Initial.face_up(i < 4 and 11 or 10)
        if i < 5 then
            initial[1], initial[2], initial[3], initial[4] = false, false, false, false
        end
        return {
            Initial = initial,
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.king_downsuit_faceup
        }
    end
}


------

local triple_scorpion           = Sol.copy(scorpion)
triple_scorpion.Info.Name       = "Triple Scorpion"
triple_scorpion.Info.DeckCount  = 3
triple_scorpion.Stock           = nil
triple_scorpion.Foundation.Size = 12
triple_scorpion.Tableau         = {
    Size = 13,
    Pile = function(i)
        local initial = Sol.Initial.face_up(12)
        if i < 5 then
            initial[1], initial[2], initial[3], initial[4], initial[5] = false, false, false, false, false
        end
        return {
            Initial = initial,
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.king_downsuit_faceup
        }
    end
}
triple_scorpion.on_init         = Sol.Layout.canister


------

local scorpion_2        = Sol.copy(scorpion)
scorpion_2.Info.Name    = "Scorpion II"
scorpion_2.Tableau.Pile = function(i)
    return {
        Initial = scorpion_initial(3, i),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.king_downsuit_faceup
    }
end


------

local scorpion_tail        = Sol.copy(scorpion)
scorpion_tail.Info.Name    = "Scorpion Tail"
scorpion_tail.Tableau.Pile = function(i)
    return {
        Initial = scorpion_initial(3, i),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.king_downac_faceup
    }
end


------

local wasp        = Sol.copy(scorpion)
wasp.Info.Name    = "Wasp"
wasp.Tableau.Pile = function(i)
    return {
        Initial = scorpion_initial(4, i),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downsuit_faceup
    }
end


------

local three_blind_mice = {
    Info       = {
        Name      = "Three Blind Mice",
        Family    = "Spider",
        DeckCount = 1
    },
    Reserve    = {
        Size = 2,
        Pile = function(i)
            return {
                Position = { x = i, y = 0 },
                Initial = Sol.Initial.face_up(1)
            }
        end
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 6, y = 0 },
                Rule = Sol.Rules.spider_foundation
            }
        end
    },
    Tableau    = {
        Size = 10,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial  = i < 7 and Sol.Initial.face_up(5) or { false, false, false, true, true },
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.king_downsuit_faceup
            }
        end
    }
}


------

local farmers_wife = Sol.copy(three_blind_mice)
farmers_wife.Info.Name = "Farmer's Wife"
farmers_wife.Tableau.Pile = function(i)
    return {
        Position = { x = i, y = 1 },
        Initial  = i < 7 and Sol.Initial.face_up(5) or { false, false, false, true, true },
        Layout   = Sol.Pile.Layout.Column,
        Rule     = Sol.Rules.king_downac_faceup
    }
end


------

local how_they_tun     = Sol.copy(three_blind_mice)
how_they_tun.Info.Name = "How They Run"
how_they_tun.Reserve   = nil
how_they_tun.FreeCell  = {
    Size = 2,
    Pile = function(i)
        return {
            Position = { x = i, y = 0 },
            Initial = Sol.Initial.face_up(1),
            Rule = Sol.Rules.any_none_top
        }
    end
}


------

------------------------

Sol.register_game(scorpion)
Sol.register_game(double_scorpion)
Sol.register_game(farmers_wife)
Sol.register_game(how_they_tun)
Sol.register_game(scorpion_2)
Sol.register_game(scorpion_tail)
Sol.register_game(three_blind_mice)
Sol.register_game(triple_scorpion)
Sol.register_game(wasp)
