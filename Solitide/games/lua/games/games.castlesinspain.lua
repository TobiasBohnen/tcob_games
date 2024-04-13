-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local castles_in_spain            = {
    Info       = {
        Name      = "Castles In Spain",
        Family    = "BakersDozen",
        DeckCount = 1,
        Redeals   = 0
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 13,
        Pile = {
            Initial = Sol.Initial.top_face_up(4),
            Layout = "Column",
            Rule = Sol.Rules.any_downac_top
        }
    },
    on_init    = Sol.Layout.bakers_dozen
}

------

local martha                      = Sol.copy(castles_in_spain)
martha.Info.Name                  = "Martha"
martha.Tableau                    = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.alternate(4, false),
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.AnySingle(), Build = Sol.Rules.Build.DownAlternateColors(), Move = Sol.Rules.Move.InSeq() }
    }
}
martha.on_before_shuffle          = Sol.Ops.Shuffle.ace_to_foundation

------

local stewart                     = Sol.copy(martha)
stewart.Info.Name                 = "Stewart"
stewart.Tableau.Pile.Rule         = { Base = Sol.Rules.Base.AnySingle(), Build = Sol.Rules.Build.DownAlternateColors(), Move = Sol.Rules.Move.Top() }

------

local portuguese_solitaire        = Sol.copy(castles_in_spain)
portuguese_solitaire.Info.Name    = "Portuguese Solitaire"
portuguese_solitaire.Tableau.Pile = {
    Initial = Sol.Initial.face_up(4),
    Layout = "Column",
    Rule = Sol.Rules.king_downrank_top
}

------

local spanish_patience            = Sol.copy(castles_in_spain)
spanish_patience.Info.Name        = "Spanish Patience"
spanish_patience.Foundation.Pile  = { Rule = Sol.Rules.ace_upac_top }
spanish_patience.Tableau.Pile     = {
    Initial = Sol.Initial.face_up(4),
    Layout = "Column",
    Rule = Sol.Rules.none_downrank_top
}

------

local spanish_patience_2          = Sol.copy(castles_in_spain)
spanish_patience_2.Info.Name      = "Spanish Patience II"
spanish_patience_2.Tableau.Pile   = {
    Initial = Sol.Initial.face_up(4),
    Layout = "Column",
    Rule = Sol.Rules.any_downrank_top
}

------

local vineyard                    = Sol.copy(castles_in_spain)
vineyard.Info.Name                = "Vineyard"
vineyard.Tableau                  = {
    Size = 10,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 2 and 6 or 5),
            Layout = "Column",
            Rule = Sol.Rules.any_downac_top
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
Sol.register_game(stewart)
Sol.register_game(vineyard)
