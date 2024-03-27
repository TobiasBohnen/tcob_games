-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local canfield                      = {
    Info       = {
        Name          = "Canfield",
        Family        = "Canfield",
        DeckCount     = 1,
        CardDealCount = 3,
        Redeals       = -1
    },
    Stock      = { Initial = ops.Initial.face_down(34) },
    Waste      = {},
    Reserve    = {
        Initial = ops.Initial.top_face_up(13),
        Layout = "Column"
    },
    Foundation = {
        Size   = 4,
        Create = function(i)
            return {
                Initial = ops.Initial.face_up(i == 0 and 1 or 0),
                Rule = rules.ff_upsuit_none
            }
        end
    },
    Tableau    = {
        Size   = 4,
        Create = {
            Initial = ops.Initial.face_up(1),
            Layout = "Column",
            Rule = { Base = rules.Base.Any, Build = rules.Build.DownAlternateColors(true), Move = rules.Move.TopOrPile() }
        }
    },
    on_change  = function(game) ops.Deal.to_group(game.Reserve[1], game.Tableau, true) end,
    on_redeal  = ops.Redeal.waste_to_stock,
    on_deal    = ops.Deal.stock_to_waste,
    on_created = Sol.Layout.canfield
}

------

local canfield_rush                 = Sol.copy(canfield)
canfield_rush.Info.Name             = "Canfield Rush"
canfield_rush.Info.Redeals          = 2
canfield_rush.on_deal               = ops.Deal.stock_to_waste_by_redeals_left

------

local double_canfield               = Sol.copy(canfield)
double_canfield.Info.Name           = "Double Canfield"
double_canfield.Info.DeckCount      = 2
double_canfield.Stock.Initial       = ops.Initial.face_down(85)
double_canfield.Foundation.Size     = 8
double_canfield.Tableau.Size        = 5

------

local triple_canfield               = Sol.copy(canfield)
triple_canfield.Info.Name           = "Triple Canfield"
triple_canfield.Info.DeckCount      = 3
triple_canfield.Stock.Initial       = ops.Initial.face_down(122)
triple_canfield.Reserve.Initial     = ops.Initial.top_face_up(26)
triple_canfield.Foundation.Size     = 12
triple_canfield.Tableau.Size        = 7

------

local superior_canfield             = Sol.copy(canfield)
superior_canfield.Info.Name         = "Superior Canfield"
superior_canfield.Reserve.Initial   = ops.Initial.face_up(13)
superior_canfield.on_change         = nil

------

local variegated_canfield           = Sol.copy(double_canfield)
variegated_canfield.Info.Name       = "Variegated Canfield"
variegated_canfield.Info.Redeals    = 2
variegated_canfield.Reserve.Initial = ops.Initial.face_up(13)

------

local acme                          = Sol.copy(canfield)
acme.Info.Name                      = "Acme"
acme.Info.CardDealCount             = 1
acme.Info.Redeals                   = 1
acme.Stock.Initial                  = ops.Initial.face_down(31)
acme.Foundation.Create              = { Rule = rules.ace_upsuit_none }
acme.Tableau.Create                 = {
    Initial = ops.Initial.face_up(1),
    Layout = "Column",
    Rule = rules.any_downsuit_top
}
acme.on_before_shuffle              = ops.Shuffle.ace_to_foundation

------

local american_toad                 = Sol.copy(canfield)
american_toad.Info.Name             = "American Toad"
american_toad.Info.DeckCount        = 2
american_toad.Info.CardDealCount    = 1
american_toad.Info.Redeals          = 1
american_toad.Stock.Initial         = ops.Initial.face_down(75)
american_toad.Reserve               = {
    Initial = ops.Initial.face_up(20),
    Layout = "Column"
}
american_toad.Foundation.Size       = 8
american_toad.Tableau               = {
    Size   = 8,
    Create = {
        Initial = ops.Initial.face_up(1),
        Layout = "Column",
        Rule = { Base = rules.Base.Any, Build = rules.Build.DownInSuit(true), Move = rules.Move.TopOrPile() }
    }
}

------

local chameleon                     = Sol.copy(canfield)
chameleon.Info.Name                 = "Chameleon"
chameleon.Info.DeckCount            = 1
chameleon.Info.CardDealCount        = 1
chameleon.Info.Redeals              = 0
chameleon.Stock.Initial             = ops.Initial.face_down(36)
chameleon.Reserve.Create            = {
    Initial = ops.Initial.top_face_up(12),
    Layout = "Column"
}
chameleon.Tableau                   = {
    Size   = 3,
    Create = {
        Initial = ops.Initial.face_up(1),
        Layout = "Column",
        Rule = { Base = rules.Base.Any, Build = rules.Build.DownByRank(true), Move = rules.Move.TopOrPile() }
    }
}

------

local demon                         = Sol.copy(canfield)
demon.Info.Name                     = "Demon"
demon.Info.DeckCount                = 2
demon.Stock.Initial                 = ops.Initial.face_down(55)
demon.Reserve.Create                = {
    Initial = ops.Initial.top_face_up(40),
    Layout = "Column"
}
demon.Foundation.Size               = 8
demon.Tableau                       = {
    Size   = 8,
    Create = {
        Initial = ops.Initial.face_up(1),
        Layout = "Column",
        Rule = { Base = rules.Base.Any, Build = rules.Build.DownAlternateColors(true), Move = rules.Move.InSeq() }
    }
}

------

local eagle_wing_pos                = {
    { 0, 1.75 }, { 1, 1.5 }, { 2, 1.25 }, { 3, 1.5 }, { 5, 1.5 }, { 6, 1.25 }, { 7, 1.5 }, { 8, 1.75 }
}

local eagle_wing                    = {
    Info       = {
        Name          = "Eagle Wing",
        Family        = "Canfield",
        DeckCount     = 1,
        CardDealCount = 1,
        Redeals       = 2
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = ops.Initial.face_down(30)
    },
    Waste      = { Position = { x = 1, y = 0 } },
    Reserve    = {
        Position = { x = 4, y = 2 },
        Initial = ops.Initial.face_down(13),
        Layout = "Squared"
    },
    Foundation = {
        Size   = 4,
        Create = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Initial = ops.Initial.face_up(i == 0 and 1 or 0),
                Rule = rules.ff_upsuit_none
            }
        end
    },
    Tableau    = {
        Size   = 8,
        Create = function(i)
            return {
                Position = { x = eagle_wing_pos[i + 1][1], y = eagle_wing_pos[i + 1][2] },
                Initial = ops.Initial.face_up(1),
                Layout = "Column",
                Rule = { Base = rules.Base.None, Build = rules.Build.DownInSuit(true), Move = rules.Move.Top(), Limit = 3 }
            }
        end
    },
    on_change  = function(game)
        ops.Deal.to_group(game.Reserve[1], game.Tableau, true)
        game.Reserve[1]:flip_down_top_card()
    end,
    on_redeal  = ops.Redeal.waste_to_stock,
    on_deal    = ops.Deal.stock_to_waste
}

------

local minerva                       = Sol.copy(canfield)
minerva.Info.Name                   = "Minerva"
minerva.Info.CardDealCount          = 1
minerva.Info.Redeals                = 1
minerva.Stock.Initial               = ops.Initial.face_down(13)
minerva.Reserve.Create              = {
    Initial = ops.Initial.top_face_up(11),
    Layout = "Column"
}
minerva.Foundation.Create           = { Rule = { Base = rules.Base.Ace, Build = rules.Build.UpInSuit(), Move = rules.Move.None() } }
minerva.Tableau                     = {
    Size   = 7,
    Create = {
        Initial = ops.Initial.alternate(4, false),
        Layout = "Column",
        Rule = rules.king_downac_inseq
    }
}
minerva.on_change                   = nil

------

local munger                        = Sol.copy(minerva)
munger.Info.Name                    = "Munger"
munger.Info.Redeals                 = 0
munger.Stock.Initial                = ops.Initial.face_down(17)
munger.Reserve.Create               = {
    Initial = ops.Initial.top_face_up(7),
    Layout = "Column"
}

------

local mystique                      = Sol.copy(minerva)
mystique.Info.Name                  = "Mystique"
mystique.Info.Redeals               = 0
mystique.Stock.Initial              = ops.Initial.face_down(15)
mystique.Reserve.Create             = {
    Initial = ops.Initial.top_face_up(9),
    Layout = "Column"
}
mystique.Tableau                    = {
    Size   = 7,
    Create = {
        Initial = ops.Initial.alternate(4, false),
        Layout = "Column",
        Rule = rules.any_downac_inseq
    }
}

------

local rainbow                       = Sol.copy(canfield)
rainbow.Info.Name                   = "Rainbow"
rainbow.Info.CardDealCount          = 1
rainbow.Info.Redeals                = 0
rainbow.Tableau.Create              = {
    Initial = ops.Initial.face_up(1),
    Layout = "Column",
    Rule = { Base = rules.Base.Any, Build = rules.Build.DownByRank(true), Move = rules.Move.TopOrPile() }
}

------

local rainfall                      = Sol.copy(canfield)
rainfall.Info.Name                  = "Rainfall"
rainfall.Info.CardDealCount         = 1
rainfall.Info.Redeals               = 2

------

local storehouse                    = Sol.copy(canfield)
storehouse.Info.Name                = "Storehouse"
storehouse.Info.CardDealCount       = 1
storehouse.Info.Redeals             = 2
storehouse.Tableau.Create           = {
    Initial = ops.Initial.face_up(1),
    Layout = "Column",
    Rule = { Base = rules.Base.Any, Build = rules.Build.DownInSuit(true), Move = rules.Move.TopOrPile() }
}

------

------------------------

Sol.register_game(canfield)
Sol.register_game(double_canfield)
Sol.register_game(triple_canfield)
Sol.register_game(canfield_rush)
Sol.register_game(acme)
Sol.register_game(american_toad)
Sol.register_game(chameleon)
Sol.register_game(demon)
Sol.register_game(eagle_wing)
Sol.register_game(minerva)
Sol.register_game(munger)
Sol.register_game(mystique)
Sol.register_game(rainbow)
Sol.register_game(rainfall)
Sol.register_game(storehouse)
Sol.register_game(superior_canfield)
Sol.register_game(variegated_canfield)
