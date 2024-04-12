-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local canfield                      = {
    Info        = {
        Name          = "Canfield",
        Family        = "Canfield",
        DeckCount     = 1,
        CardDealCount = 3,
        Redeals       = -1
    },
    Stock       = { Initial = Sol.Initial.face_down(34) },
    Waste       = {},
    Reserve     = {
        Initial = Sol.Initial.top_face_up(13),
        Layout = "Column"
    },
    Foundation  = {
        Size = 4,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i == 0 and 1 or 0),
                Rule = Sol.Rules.ff_upsuit_none_l13
            }
        end
    },
    Tableau     = {
        Size = 4,
        Pile = {
            Initial = Sol.Initial.face_up(1),
            Layout = "Column",
            Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownAlternateColors(true), Move = Sol.Rules.Move.TopOrPile() }
        }
    },
    on_end_turn = function(game) Sol.Ops.Deal.to_group(game.Reserve[1], game.Tableau, true) end,
    do_redeal   = Sol.Ops.Redeal.waste_to_stock,
    do_deal     = Sol.Ops.Deal.stock_to_waste,
    on_init     = Sol.Layout.canfield
}

------

local canfield_rush                 = Sol.copy(canfield)
canfield_rush.Info.Name             = "Canfield Rush"
canfield_rush.Info.Redeals          = 2
canfield_rush.do_deal               = Sol.Ops.Deal.stock_to_waste_by_redeals_left

------

local double_canfield               = Sol.copy(canfield)
double_canfield.Info.Name           = "Double Canfield"
double_canfield.Info.DeckCount      = 2
double_canfield.Stock.Initial       = Sol.Initial.face_down(85)
double_canfield.Foundation.Size     = 8
double_canfield.Tableau.Size        = 5

------

local triple_canfield               = Sol.copy(canfield)
triple_canfield.Info.Name           = "Triple Canfield"
triple_canfield.Info.DeckCount      = 3
triple_canfield.Stock.Initial       = Sol.Initial.face_down(122)
triple_canfield.Reserve.Initial     = Sol.Initial.top_face_up(26)
triple_canfield.Foundation.Size     = 12
triple_canfield.Tableau.Size        = 7

------

local quadruple_canfield            = Sol.copy(canfield)
quadruple_canfield.Info.Name        = "Quadruple Canfield"
quadruple_canfield.Info.DeckCount   = 4
quadruple_canfield.Stock.Initial    = Sol.Initial.face_down(160)
quadruple_canfield.Reserve.Initial  = Sol.Initial.top_face_up(39)
quadruple_canfield.Foundation.Size  = 16
quadruple_canfield.Tableau.Size     = 8

------

local superior_canfield             = Sol.copy(canfield)
superior_canfield.Info.Name         = "Superior Canfield"
superior_canfield.Reserve.Initial   = Sol.Initial.face_up(13)
superior_canfield.on_end_turn       = nil

------

local variegated_canfield           = Sol.copy(double_canfield)
variegated_canfield.Info.Name       = "Variegated Canfield"
variegated_canfield.Info.Redeals    = 2
variegated_canfield.Reserve.Initial = Sol.Initial.face_up(13)

------

local acme                          = Sol.copy(canfield)
acme.Info.Name                      = "Acme"
acme.Info.CardDealCount             = 1
acme.Info.Redeals                   = 1
acme.Stock.Initial                  = Sol.Initial.face_down(31)
acme.Foundation.Pile                = { Rule = Sol.Rules.ace_upsuit_none }
acme.Tableau.Pile                   = {
    Initial = Sol.Initial.face_up(1),
    Layout = "Column",
    Rule = Sol.Rules.any_downsuit_top
}
acme.on_before_shuffle              = Sol.Ops.Shuffle.ace_to_foundation

------

local american_toad                 = Sol.copy(canfield)
american_toad.Info.Name             = "American Toad"
american_toad.Info.DeckCount        = 2
american_toad.Info.CardDealCount    = 1
american_toad.Info.Redeals          = 1
american_toad.Stock.Initial         = Sol.Initial.face_down(75)
american_toad.Reserve               = {
    Initial = Sol.Initial.face_up(20),
    Layout = "Column"
}
american_toad.Foundation.Size       = 8
american_toad.Tableau               = {
    Size = 8,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownInSuit(true), Move = Sol.Rules.Move.TopOrPile() }
    }
}

------

local beehive                       = Sol.copy(canfield)
beehive.Info.Name                   = "Beehive"
beehive.Stock.Initial               = Sol.Initial.face_down(33)
beehive.Foundation                  = { Rule = Sol.Rules.none_none_none }
beehive.Tableau                     = {
    Size = 6,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.InSeq() }
    }
}
beehive.on_end_turn                 = function(game)
    local tableau = game.Tableau
    for _, tab in ipairs(game.Tableau) do
        if tab.CardCount == 4 then
            tab:move_cards(game.Foundation[1], 1, 4, false)
        end
    end

    Sol.Ops.Deal.to_group(game.Reserve[1], tableau, true)
end

------

local chameleon                     = Sol.copy(canfield)
chameleon.Info.Name                 = "Chameleon"
chameleon.Info.CardDealCount        = 1
chameleon.Info.Redeals              = 0
chameleon.Stock.Initial             = Sol.Initial.face_down(36)
chameleon.Reserve.Pile              = {
    Initial = Sol.Initial.top_face_up(12),
    Layout = "Column"
}
chameleon.Tableau                   = {
    Size = 3,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(true), Move = Sol.Rules.Move.TopOrPile() }
    }
}

------

local demon                         = Sol.copy(canfield)
demon.Info.Name                     = "Demon"
demon.Info.DeckCount                = 2
demon.Stock.Initial                 = Sol.Initial.face_down(55)
demon.Reserve.Pile                  = {
    Initial = Sol.Initial.top_face_up(40),
    Layout = "Column"
}
demon.Foundation.Size               = 8
demon.Tableau                       = {
    Size = 8,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownAlternateColors(true), Move = Sol.Rules.Move.InSeq() }
    }
}

------

local eagle_wing_pos                = {
    { 0, 1.75 }, { 1, 1.5 }, { 2, 1.25 }, { 3, 1.5 }, { 5, 1.5 }, { 6, 1.25 }, { 7, 1.5 }, { 8, 1.75 }
}

local eagle_wing                    = {
    Info        = {
        Name          = "Eagle Wing",
        Family        = "Canfield",
        DeckCount     = 1,
        CardDealCount = 1,
        Redeals       = 2
    },
    Stock       = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(30)
    },
    Waste       = { Position = { x = 1, y = 0 } },
    Reserve     = {
        Position = { x = 4, y = 2 },
        Initial = Sol.Initial.face_down(13),
        Layout = "Squared"
    },
    Foundation  = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Initial = Sol.Initial.face_up(i == 0 and 1 or 0),
                Rule = Sol.Rules.ff_upsuit_none_l13
            }
        end
    },
    Tableau     = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = eagle_wing_pos[i + 1][1], y = eagle_wing_pos[i + 1][2] },
                Initial = Sol.Initial.face_up(1),
                Layout = "Column",
                Rule = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.DownInSuit(true), Move = Sol.Rules.Move.Top(), Limit = 3 }
            }
        end
    },
    on_end_turn = function(game)
        Sol.Ops.Deal.to_group(game.Reserve[1], game.Tableau, true)
        game.Reserve[1]:flip_down_top_card()
    end,
    do_redeal   = Sol.Ops.Redeal.waste_to_stock,
    do_deal     = Sol.Ops.Deal.stock_to_waste
}

------

local lafayette                     = Sol.copy(canfield)
lafayette.Info.Name                 = "Lafayette"
lafayette.Stock.Initial             = Sol.Initial.face_down(35)
lafayette.Foundation                = {
    Size = 8,
    Pile = function(i)
        if i < 4 then
            return {
                Rule = Sol.Rules.ace_upsuit_none
            }
        else
            return {
                Rule = Sol.Rules.king_downsuit_none
            }
        end
    end
}
lafayette.Tableau                   = {
    Size = 4,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout = "Column",
        Rule = {
            Base = Sol.Rules.Base.Ranks({ "Seven" }),
            Build = Sol.Rules.Build.DownAlternateColors(),
            Move = Sol.Rules.Move.InSeq()
        }
    }
}

------

local minerva                       = Sol.copy(canfield)
minerva.Info.Name                   = "Minerva"
minerva.Info.CardDealCount          = 1
minerva.Info.Redeals                = 1
minerva.Stock.Initial               = Sol.Initial.face_down(13)
minerva.Reserve.Pile                = {
    Initial = Sol.Initial.top_face_up(11),
    Layout = "Column"
}
minerva.Foundation.Pile             = { Rule = { Base = Sol.Rules.Base.Ace(), Build = Sol.Rules.Build.UpInSuit(), Move = Sol.Rules.Move.None() } }
minerva.Tableau                     = {
    Size = 7,
    Pile = {
        Initial = Sol.Initial.alternate(4, false),
        Layout = "Column",
        Rule = Sol.Rules.king_downac_inseq
    }
}
minerva.on_end_turn                 = nil

------

local munger                        = Sol.copy(minerva)
munger.Info.Name                    = "Munger"
munger.Info.Redeals                 = 0
munger.Stock.Initial                = Sol.Initial.face_down(17)
munger.Reserve.Pile                 = {
    Initial = Sol.Initial.top_face_up(7),
    Layout = "Column"
}

------

local mystique                      = Sol.copy(minerva)
mystique.Info.Name                  = "Mystique"
mystique.Info.Redeals               = 0
mystique.Stock.Initial              = Sol.Initial.face_down(15)
mystique.Reserve.Pile               = {
    Initial = Sol.Initial.top_face_up(9),
    Layout = "Column"
}
mystique.Tableau                    = {
    Size = 7,
    Pile = {
        Initial = Sol.Initial.alternate(4, false),
        Layout = "Column",
        Rule = Sol.Rules.any_downac_inseq
    }
}

------

local rainbow                       = Sol.copy(canfield)
rainbow.Info.Name                   = "Rainbow"
rainbow.Info.CardDealCount          = 1
rainbow.Info.Redeals                = 0
rainbow.Tableau.Pile                = {
    Initial = Sol.Initial.face_up(1),
    Layout = "Column",
    Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(true), Move = Sol.Rules.Move.TopOrPile() }
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
storehouse.Tableau.Pile             = {
    Initial = Sol.Initial.face_up(1),
    Layout = "Column",
    Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownInSuit(true), Move = Sol.Rules.Move.TopOrPile() }
}

------

------------------------

Sol.register_game(canfield)
Sol.register_game(acme)
Sol.register_game(american_toad)
Sol.register_game(beehive)
Sol.register_game(canfield_rush)
Sol.register_game(chameleon)
Sol.register_game(demon)
Sol.register_game(double_canfield)
Sol.register_game(eagle_wing)
Sol.register_game(lafayette)
Sol.register_game(minerva)
Sol.register_game(munger)
Sol.register_game(mystique)
Sol.register_game(quadruple_canfield)
Sol.register_game(rainbow)
Sol.register_game(rainfall)
Sol.register_game(storehouse)
Sol.register_game(superior_canfield)
Sol.register_game(triple_canfield)
Sol.register_game(variegated_canfield)
