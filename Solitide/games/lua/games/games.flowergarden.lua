-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local flower_garden           = {
    Info             = {
        Name          = "Flower Garden",
        Family        = "Raglan",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Reserve          = {
        Size = 16,
        Pile = {
            Initial = Sol.Initial.face_up(1),
            Layout = "Squared",
            Rule = Sol.Rules.none_none_top
        }
    },
    Foundation       = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau          = {
        Size = 6,
        Pile = {
            Initial = Sol.Initial.face_up(6),
            Layout = "Column",
            Rule = Sol.Rules.any_downrank_top
        }
    },
    on_piles_created = function(game) Sol.Layout.raglan(game, 4) end
}

------

local northwest_territory     = Sol.copy(flower_garden)
northwest_territory.Info.Name = "Northwest Territory"
northwest_territory.Tableau   = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = Sol.Initial.top_face_up(i + 1),
            Layout = "Column",
            Rule = Sol.Rules.king_downac_inseq
        }
    end
}

------

local artic_garden            = Sol.copy(northwest_territory)
artic_garden.Info.Name        = "Artic Garden"
artic_garden.Tableau.Pile     =
    function(i)
        return {
            Initial = Sol.Initial.face_up(i + 1),
            Layout = "Column",
            Rule = Sol.Rules.king_downac_inseq
        }
    end

------

local stonewall               = Sol.copy(flower_garden)
stonewall.Info.Name           = "Stonewall"
stonewall.Tableau.Pile        = {
    Initial = Sol.Initial.alternate(6, false),
    Layout = "Column",
    Rule = Sol.Rules.any_downac_inseq
}
------

local wildflower              = Sol.copy(flower_garden)
wildflower.Info.Name          = "Wildflower"
wildflower.Tableau.Pile.Rule  = Sol.Rules.spider

------

------------------------

Sol.register_game(flower_garden)
Sol.register_game(artic_garden)
Sol.register_game(northwest_territory)
Sol.register_game(stonewall)
Sol.register_game(wildflower)
