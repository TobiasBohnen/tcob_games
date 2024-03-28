-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local forty_thieves                   = {
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

local sixty_thieves                   = Sol.copy(forty_thieves)
sixty_thieves.Info.Name               = "Sixty Thieves"
sixty_thieves.Info.DeckCount          = 3
sixty_thieves.Stock.Initial           = ops.Initial.face_down(96)
sixty_thieves.Foundation.Size         = 12
sixty_thieves.Tableau.Size            = 12
sixty_thieves.Tableau.Create.Initial  = ops.Initial.face_up(5)

------

local eighty_thieves                  = Sol.copy(forty_thieves)
eighty_thieves.Info.Name              = "Eighty Thieves"
eighty_thieves.Info.DeckCount         = 4
eighty_thieves.Stock.Initial          = ops.Initial.face_down(128)
eighty_thieves.Foundation.Size        = 16
eighty_thieves.Tableau.Create.Initial = ops.Initial.face_up(8)

------

local alternation                     = Sol.copy(forty_thieves)
alternation.Info.Name                 = "Alternation"
alternation.Stock.Initial             = ops.Initial.face_down(55)
alternation.Tableau                   = {
    Size   = 7,
    Create = {
        Initial = ops.Initial.alternate(7, true),
        Layout = "Column",
        Rule = rules.king_downac_inseq
    }
}

------

local breakwater                      = Sol.copy(alternation)
breakwater.Info.Name                  = "Breakwater"
breakwater.Tableau.Create.Rule        = rules.king_downrank_inseq

------

local interchange                     = Sol.copy(alternation)
interchange.Info.Name                 = "Interchange"
interchange.Tableau.Create.Rule       = rules.king_downsuit_inseq

------

local unlimited                       = Sol.copy(interchange)
unlimited.Info.Name                   = "Unlimited"
unlimited.Info.Redeals                = -1

------

local forty_nine                      = Sol.copy(interchange)
forty_nine.Info.Name                  = "Forty Nine"
forty_nine.Tableau.Create.Initial     = ops.Initial.face_up(7)

------

local busy_aces                       = Sol.copy(forty_thieves)
busy_aces.Info.Name                   = "Busy Aces"
busy_aces.Stock.Initial               = ops.Initial.face_down(92)
busy_aces.Tableau                     = {
    Size   = 12,
    Create = {
        Initial = ops.Initial.face_up(1),
        Layout = "Column",
        Rule = rules.any_downsuit_top
    }
}

------

local corona                          = Sol.copy(forty_thieves)
corona.Info.Name                      = "Corona"
corona.Stock.Initial                  = ops.Initial.face_down(68)
corona.Tableau                        = {
    Size   = 12,
    Create = {
        Initial = ops.Initial.face_up(3),
        Layout = "Column",
        Rule = rules.any_downsuit_top
    }
}
corona.on_end_turn                    = function(game)
    return ops.Deal.to_group(game.Waste[1], game.Tableau, true) or ops.Deal.to_group(game.Stock[1], game.Tableau, true)
end

------

local courtyard                       = Sol.copy(forty_thieves)
courtyard.Info.Name                   = "Courtyard"
courtyard.Stock.Initial               = ops.Initial.face_down(92)
courtyard.Tableau                     = {
    Size   = 12,
    Create = {
        Initial = ops.Initial.face_up(1),
        Layout = "Column",
        Rule = rules.any_downsuit_inseq
    }
}
courtyard.on_end_turn                 = corona.on_end_turn

------

local big_courtyard                   = Sol.copy(courtyard)
big_courtyard.Info.Name               = "Big Courtyard"
big_courtyard.Info.DeckCount          = 3
big_courtyard.Stock.Initial           = ops.Initial.face_down(144)
big_courtyard.Foundation.Size         = 12

------

local limited                         = Sol.copy(forty_thieves)
limited.Info.Name                     = "Limited"
limited.Stock.Initial                 = ops.Initial.face_down(68)
limited.Tableau.Size                  = 12
limited.Tableau.Create.Initial        = ops.Initial.face_up(3)

------

local express                         = Sol.copy(limited)
express.Info.Name                     = "Express"
express.Info.DeckCount                = 3
express.Stock.Initial                 = ops.Initial.face_down(114)
express.Foundation.Size               = 12
express.Tableau.Size                  = 14

------

local carnation                       = Sol.copy(limited)
carnation.Info.Name                   = "Carnation"
carnation.Info.DeckCount              = 4
carnation.Stock.Initial               = ops.Initial.face_down(160)
carnation.Foundation.Size             = 16
carnation.Tableau.Size                = 16

------

local diplomat                        = Sol.copy(forty_thieves)
diplomat.Info.Name                    = "Diplomat"
diplomat.Stock.Initial                = ops.Initial.face_down(72)
diplomat.Tableau                      = {
    Size   = 8,
    Create = {
        Initial = ops.Initial.face_up(4),
        Layout = "Column",
        Rule = rules.any_downrank_top
    }
}

------

local double_rail                     = Sol.copy(forty_thieves)
double_rail.Info.Name                 = "Double Rail"
double_rail.Stock.Initial             = ops.Initial.face_down(99)
double_rail.Tableau                   = {
    Size   = 5,
    Create = {
        Initial = ops.Initial.face_up(1),
        Layout = "Column",
        Rule = rules.any_downrank_inseq
    }
}

------

local triple_rail                     = Sol.copy(double_rail)
triple_rail.Info.Name                 = "Triple Rail"
triple_rail.Info.DeckCount            = 3
triple_rail.Stock.Initial             = ops.Initial.face_down(147)
triple_rail.Foundation.Size           = 12
triple_rail.Tableau.Size              = 9

------

local final_battle                    = Sol.copy(double_rail)
final_battle.Info.Name                = "Final Battle"
final_battle.Stock.Initial            = ops.Initial.face_down(98)
final_battle.Tableau.Size             = 6

------

local famous_fifty                    = Sol.copy(forty_thieves)
famous_fifty.Info.Name                = "Famous Fifty"
famous_fifty.Stock.Initial            = ops.Initial.face_down(54)
famous_fifty.Tableau.Create.Initial   = ops.Initial.face_up(5)

------

local streets                         = Sol.copy(forty_thieves)
streets.Info.Name                     = "Streets"
streets.Tableau.Create.Rule           = rules.any_downac_top

------

local big_streets                     = Sol.copy(streets)
big_streets.Info.Name                 = "Big Streets"
big_streets.Info.DeckCount            = 3
big_streets.Stock.Initial             = ops.Initial.face_down(108)
big_streets.Foundation.Size           = 12
big_streets.Tableau.Size              = 12

------

local emperor                         = Sol.copy(streets)
emperor.Info.Name                     = "Emperor"
emperor.Tableau.Create.Initial        = ops.Initial.top_face_up(4)

------

local forty_and_eight                 = Sol.copy(forty_thieves)
forty_and_eight.Info.Name             = "Forty and Eight"
forty_and_eight.Info.Redeal           = 1
forty_and_eight.Stock.Initial         = ops.Initial.face_down(72)
forty_and_eight.Tableau.Size          = 8

------

local waning_moon                     = Sol.copy(forty_thieves)
waning_moon.Info.Name                 = "Waning Moon"
waning_moon.Stock.Initial             = ops.Initial.face_down(52)
waning_moon.Tableau.Size              = 13

------

local lucas                           = Sol.copy(waning_moon)
lucas.Info.Name                       = "Lucas"
lucas.Tableau.Create.Rule             = rules.any_downsuit_inseq

------------------------

Sol.register_game(forty_thieves)
Sol.register_game(sixty_thieves)
Sol.register_game(eighty_thieves)
Sol.register_game(alternation)
Sol.register_game(breakwater)
Sol.register_game(busy_aces)
Sol.register_game(carnation)
Sol.register_game(corona)
Sol.register_game(courtyard)
Sol.register_game(big_courtyard)
Sol.register_game(diplomat)
Sol.register_game(double_rail)
Sol.register_game(triple_rail)
Sol.register_game(emperor)
Sol.register_game(express)
Sol.register_game(famous_fifty)
Sol.register_game(final_battle)
Sol.register_game(forty_and_eight)
Sol.register_game(forty_nine)
Sol.register_game(interchange)
Sol.register_game(limited)
Sol.register_game(lucas)
Sol.register_game(streets)
Sol.register_game(big_streets)
Sol.register_game(unlimited)
Sol.register_game(waning_moon)
