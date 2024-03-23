-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local castles_in_spain              = {
    Info       = {
        Name          = "Castles In Spain",
        Family        = "BakersDozen",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size   = 4,
        Create = { Rule = rules.ace_upsuit_top }
    },
    Tableau    = {
        Size   = 13,
        Create = {
            Initial = ops.Initial.top_face_up(4),
            Layout = "Column",
            Rule = rules.any_downac_top
        }
    },
    on_created = Sol.Layout.bakers_dozen
}

------

local martha                        = Sol.copy(castles_in_spain)
martha.Info.Name                    = "Martha"
martha.Tableau                      = {
    Size   = 12,
    Create = {
        Initial = ops.Initial.alternate(4, false),
        Layout = "Column",
        Rule = { Base = rules.Base.AnySingle, Build = rules.Build.DownAlternateColors(), Move = rules.Move.InSeq() }
    }
}
martha.on_before_shuffle            = ops.Shuffle.ace_to_foundation

------

local portuguese_solitaire          = Sol.copy(castles_in_spain)
portuguese_solitaire.Info.Name      = "Portuguese Solitaire"
portuguese_solitaire.Tableau.Create = {
    Initial = ops.Initial.face_up(4),
    Layout = "Column",
    Rule = rules.king_downrank_top
}

------

local spanish_patience              = Sol.copy(castles_in_spain)
spanish_patience.Info.Name          = "Spanish Patience"
spanish_patience.Foundation.Create  = { Rule = rules.ace_upac_top }
spanish_patience.Tableau.Create     = {
    Initial = ops.Initial.face_up(4),
    Layout = "Column",
    Rule = rules.none_downrank_top
}

------

local spanish_patience_2            = Sol.copy(castles_in_spain)
spanish_patience_2.Info.Name        = "Spanish Patience II"
spanish_patience_2.Tableau.Create   = {
    Initial = ops.Initial.face_up(4),
    Layout = "Column",
    Rule = rules.any_downrank_top
}

------

local vineyard                      = Sol.copy(castles_in_spain)
vineyard.Info.Name                  = "Vineyard"
vineyard.Tableau                    = {
    Size   = 10,
    Create = function(i)
        return {
            Initial = ops.Initial.face_up(i < 2 and 6 or 5),
            Layout = "Column",
            Rule = rules.any_downac_top
        }
    end
}

------

------------------------

Sol.register_game(castles_in_spain)
Sol.register_game(martha)
Sol.register_game(portuguese_solitaire)
Sol.register_game(spanish_patience)
Sol.register_game(spanish_patience_2)
Sol.register_game(vineyard)
