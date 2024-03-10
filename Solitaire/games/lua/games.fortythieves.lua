-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local piles = require 'base/piles'


local forty_thieves        = {
    Info       = {
        Name          = "Forty Thieves",
        Type          = "SimplePacker",
        Family        = "FortyThieves",
        DeckCount     = 2,
        CardDealCount = 1,
        Redeals       = 0
    },
    Stock      = {
        Initial = piles.Initial.face_down(64)
    },
    Waste      = {},
    Foundation = {
        Size   = 8,
        create = piles.ace_upsuit_top
    },
    Tableau    = {
        Size   = 10,
        create = {
            Initial = piles.Initial.face_up(4),
            Layout = "Column",
            Rule = { Build = "DownInSuit", Move = "Top", Empty = "Any" }
        }
    },
    on_deal    = ops.Deal.stock_to_waste,
    on_created = Sol.Layout.forty_thieves
}

------

local alternation          = Sol.copy(forty_thieves)
alternation.Info.Name      = "Alternation"
alternation.Stock.Initial  = piles.Initial.face_down(55)
alternation.Tableau        = {
    Size   = 7,
    create = {
        Initial = piles.Initial.alternate(7, true),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "King" }
    }
}

------

local interchange          = Sol.copy(alternation)
interchange.Info.Name      = "Interchange"
interchange.Tableau.create = {
    Initial = piles.Initial.alternate(7, true),
    Layout = "Column",
    Rule = { Build = "DownInSuit", Move = "InSequence", Empty = "King" }
}

------

local busy_aces            = Sol.copy(forty_thieves)
busy_aces.Info.Name        = "Busy Aces"
busy_aces.Stock.Initial    = piles.Initial.face_down(92)
busy_aces.Tableau          = {
    Size   = 12,
    create = {
        Initial = piles.Initial.face_up(1),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "Top", Empty = "Any" }
    }
}

------

local corona               = Sol.copy(forty_thieves)
corona.Info.Name           = "Corona"
corona.Stock.Initial       = piles.Initial.face_down(68)
corona.Tableau             = {
    Size   = 12,
    create = {
        Initial = piles.Initial.face_up(3),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "Top", Empty = "Any" }
    }
}
corona.on_change           = function(game)
    return game.Waste[1]:deal_to_group(game.Tableau, true) or game.Stock[1]:deal_to_group(game.Tableau, true)
end

------

local courtyard            = Sol.copy(forty_thieves)
courtyard.Info.Name        = "Courtyard"
courtyard.Stock.Initial    = piles.Initial.face_down(92)
courtyard.Tableau          = {
    Size   = 12,
    create = {
        Initial = piles.Initial.face_up(1),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "InSequence", Empty = "Any" }
    }
}
courtyard.on_change        = corona.on_change

------------------------

Sol.register_game(forty_thieves)
Sol.register_game(alternation)
Sol.register_game(busy_aces)
Sol.register_game(corona)
Sol.register_game(courtyard)
Sol.register_game(interchange)
