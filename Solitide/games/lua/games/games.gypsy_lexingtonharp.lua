-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local lexington_harp       = {
    Info       = {
        Name      = "Lexington Harp",
        Family    = "Gypsy",
        --Family = "Gypsy/Yukon"
        DeckCount = 2
    },
    Stock      = {
        Initial = Sol.Initial.face_down(68)
    },
    Foundation = {
        Size = 8,
        Pile = { Rule = Sol.Rules.ace_upsuit_none }
    },
    Tableau    = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = Sol.Initial.top_face_up(i + 1),
                Layout = "Column",
                Rule = Sol.Rules.any_downac_faceup
            }
        end
    },
    on_init    = Sol.Layout.gypsy,
    deal       = Sol.Ops.Deal.stock_to_tableau
}

------

local brunswick            = Sol.copy(lexington_harp)
brunswick.Info.Name        = "Brunswick"
brunswick.Tableau.Pile     = function(i)
    return {
        Initial = Sol.Initial.face_up(i + 1),
        Layout = "Column",
        Rule = Sol.Rules.any_downac_faceup
    }
end

------

local griffon              = Sol.copy(brunswick)
griffon.Info.Name          = "Griffon"
griffon.Stock.Initial      = Sol.Initial.face_down(76)
griffon.Tableau.Size       = 7

------

local milligan_harp        = Sol.copy(lexington_harp)
milligan_harp.Info.Name    = "Milligan Harp"
milligan_harp.Tableau.Pile = function(i)
    return {
        Initial = Sol.Initial.top_face_up(i + 1),
        Layout = "Column",
        Rule = Sol.Rules.any_downac_inseq
    }
end

------

local steve                = Sol.copy(lexington_harp)
steve.Info.Name            = "Steve"
steve.Tableau.Pile         = function(i)
    return {
        Initial = Sol.Initial.top_face_up(i + 1),
        Layout = "Column",
        Rule = Sol.Rules.spider_tableau
    }
end

------

local carlton              = Sol.copy(lexington_harp)
carlton.Info.Name          = "Carlton"
carlton.Tableau.Pile       = function(i)
    return {
        Initial = Sol.Initial.face_up(i + 1),
        Layout = "Column",
        Rule = Sol.Rules.any_downac_inseq
    }
end

------

local mississippi          = Sol.copy(lexington_harp)
mississippi.Info.Name      = "Mississippi"
mississippi.Stock.Initial  = Sol.Initial.face_down(76)
mississippi.Tableau        = {
    Size = 7,
    Pile = function(i)
        return {
            Initial = Sol.Initial.top_face_up(i + 1),
            Layout = "Column",
            Rule = Sol.Rules.any_downac_faceup
        }
    end
}

------

------------------------

Sol.register_game(lexington_harp)
Sol.register_game(brunswick)
Sol.register_game(carlton)
Sol.register_game(griffon)
Sol.register_game(milligan_harp)
Sol.register_game(mississippi)
Sol.register_game(steve)
