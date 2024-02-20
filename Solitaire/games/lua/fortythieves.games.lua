-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local layout = require 'base/layout'
local ops    = require 'base/ops'
local piles  = require 'base/piles'
require 'base/common'

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
        Initial = piles.initial.face_down(64)
    },
    Waste      = {},
    Foundation = {
        Size   = 8,
        create = piles.ace_upsuit_top
    },
    Tableau    = {
        Size   = 10,
        create = {
            Initial = piles.initial.face_up(4),
            Layout = "Column",
            Rule = { Build = "DownInSuit", Move = "Top", Empty = "Any" }
        }
    },
    deal       = ops.deal.stock_to_waste,
    layout     = layout.forty_thieves
}

------

local alternation          = Copy(forty_thieves)
alternation.Info.Name      = "Alternation"
alternation.Stock.Initial  = piles.initial.face_down(55)
alternation.Tableau        = {
    Size   = 7,
    create = {
        Initial = piles.initial.alternate(7, true),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "King" }
    }
}

------

local interchange          = Copy(alternation)
interchange.Info.Name      = "Interchange"
interchange.Tableau.create = {
    Initial = piles.initial.alternate(7, true),
    Layout = "Column",
    Rule = { Build = "DownInSuit", Move = "InSequence", Empty = "King" }
}

------

local busy_aces            = Copy(forty_thieves)
busy_aces.Info.Name        = "Busy Aces"
busy_aces.Stock.Initial    = piles.initial.face_down(92)
busy_aces.Tableau          = {
    Size   = 12,
    create = {
        Initial = piles.initial.face_up(1),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "Top", Empty = "Any" }
    }
}

------

local corona               = Copy(forty_thieves)
corona.Info.Name           = "Corona"
corona.Stock.Initial       = piles.initial.face_down(68)
corona.Tableau             = {
    Size   = 12,
    create = {
        Initial = piles.initial.face_up(3),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "Top", Empty = "Any" }
    }
}
corona.before_layout       = function(game)
    return game.Waste[1]:fill_group(game.Tableau) or game.Stock[1]:fill_group(game.Tableau)
end

------

local courtyard            = Copy(forty_thieves)
courtyard.Info.Name        = "Courtyard"
courtyard.Stock.Initial    = piles.initial.face_down(92)
courtyard.Tableau          = {
    Size   = 12,
    create = {
        Initial = piles.initial.face_up(1),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "InSequence", Empty = "Any" }
    }
}
courtyard.before_layout    = corona.before_layout

------------------------

register_game(forty_thieves)
register_game(alternation)
register_game(busy_aces)
register_game(corona)
register_game(courtyard)
register_game(interchange)
