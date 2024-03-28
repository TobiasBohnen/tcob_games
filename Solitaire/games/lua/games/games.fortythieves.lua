-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local forty_thieves        = {
    Info             = {
        Name          = "Forty Thieves",
        Family        = "FortyThieves",
        DeckCount     = 2,
        CardDealCount = 1,
        Redeals       = 0
    },
    Stock            = {
        Initial = ops.Initial.face_down(64)
    },
    Waste            = {},
    Foundation       = {
        Size   = 8,
        Create = { Rule = rules.ace_upsuit_top }
    },
    Tableau          = {
        Size   = 10,
        Create = {
            Initial = ops.Initial.face_up(4),
            Layout = "Column",
            Rule = rules.any_downsuit_top
        }
    },
    on_deal          = ops.Deal.stock_to_waste,
    on_piles_created = Sol.Layout.forty_thieves
}

------

local alternation          = Sol.copy(forty_thieves)
alternation.Info.Name      = "Alternation"
alternation.Stock.Initial  = ops.Initial.face_down(55)
alternation.Tableau        = {
    Size   = 7,
    Create = {
        Initial = ops.Initial.alternate(7, true),
        Layout = "Column",
        Rule = rules.king_downac_inseq
    }
}

------

local interchange          = Sol.copy(alternation)
interchange.Info.Name      = "Interchange"
interchange.Tableau.Create = {
    Initial = ops.Initial.alternate(7, true),
    Layout = "Column",
    Rule = rules.king_downsuit_inseq
}

------

local busy_aces            = Sol.copy(forty_thieves)
busy_aces.Info.Name        = "Busy Aces"
busy_aces.Stock.Initial    = ops.Initial.face_down(92)
busy_aces.Tableau          = {
    Size   = 12,
    Create = {
        Initial = ops.Initial.face_up(1),
        Layout = "Column",
        Rule = rules.any_downsuit_top
    }
}

------

local corona               = Sol.copy(forty_thieves)
corona.Info.Name           = "Corona"
corona.Stock.Initial       = ops.Initial.face_down(68)
corona.Tableau             = {
    Size   = 12,
    Create = {
        Initial = ops.Initial.face_up(3),
        Layout = "Column",
        Rule = rules.any_downsuit_top
    }
}
corona.on_end_turn         = function(game)
    return ops.Deal.to_group(game.Waste[1], game.Tableau, true) or ops.Deal.to_group(game.Stock[1], game.Tableau, true)
end

------

local courtyard            = Sol.copy(forty_thieves)
courtyard.Info.Name        = "Courtyard"
courtyard.Stock.Initial    = ops.Initial.face_down(92)
courtyard.Tableau          = {
    Size   = 12,
    Create = {
        Initial = ops.Initial.face_up(1),
        Layout = "Column",
        Rule = rules.any_downsuit_inseq
    }
}
courtyard.on_end_turn      = corona.on_end_turn

------------------------

Sol.register_game(forty_thieves)
Sol.register_game(alternation)
Sol.register_game(busy_aces)
Sol.register_game(corona)
Sol.register_game(courtyard)
Sol.register_game(interchange)
