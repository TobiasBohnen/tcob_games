-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local piles = require 'base/piles'


local canfield                      = {
    Info       = {
        Name          = "Canfield",
        Type          = "ReservedPacker",
        Family        = "Canfield",
        DeckCount     = 1,
        CardDealCount = 3,
        Redeals       = -1
    },
    Stock      = { Initial = piles.Initial.face_down(34) },
    Waste      = {},
    Reserve    = {
        Initial = piles.Initial.top_face_up(13),
        Layout = "Column"
    },
    Foundation = {
        Size   = 4,
        create = function(i)
            return {
                Initial = piles.Initial.face_up(i == 0 and 1 or 0),
                Rule = { Build = "UpInSuit", Wrap = true, Move = "None", Empty = "FirstFoundation" }
            }
        end
    },
    Tableau    = {
        Size   = 4,
        create = {
            Initial = piles.Initial.face_up(1),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Wrap = true, Move = "TopOrPile", Empty = "Any" }
        }
    },
    on_change  = function(game) game.Reserve[1]:deal_to_group(game.Tableau, true) end,
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
double_canfield.Stock.Initial       = piles.Initial.face_down(85)
double_canfield.Foundation.Size     = 8
double_canfield.Tableau.Size        = 5

------

local triple_canfield               = Sol.copy(canfield)
triple_canfield.Info.Name           = "Triple Canfield"
triple_canfield.Info.DeckCount      = 3
triple_canfield.Stock.Initial       = piles.Initial.face_down(122)
triple_canfield.Reserve.Initial     = piles.Initial.top_face_up(26)
triple_canfield.Foundation.Size     = 12
triple_canfield.Tableau.Size        = 7

------

local superior_canfield             = Sol.copy(canfield)
superior_canfield.Info.Name         = "Superior Canfield"
superior_canfield.Reserve.Initial   = piles.Initial.face_up(13)
superior_canfield.on_change         = nil

------

local variegated_canfield           = Sol.copy(double_canfield)
variegated_canfield.Info.Name       = "Variegated Canfield"
variegated_canfield.Info.Redeals    = 2
variegated_canfield.Reserve.Initial = piles.Initial.face_up(13)

------

local acme                          = Sol.copy(canfield)
acme.Info.Name                      = "Acme"
acme.Info.CardDealCount             = 1
acme.Info.Redeals                   = 1
acme.Stock.Initial                  = piles.Initial.face_down(31)
acme.Foundation.create              = { Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" } }
acme.Tableau.create                 = {
    Initial = piles.Initial.face_up(1),
    Layout = "Column",
    Rule = { Build = "DownInSuit", Move = "Top", Empty = "Any" }
}
acme.on_before_shuffle              = ops.Shuffle.ace_to_foundation

------

local american_toad                 = Sol.copy(canfield)
american_toad.Info.Name             = "American Toad"
american_toad.Info.DeckCount        = 2
american_toad.Info.CardDealCount    = 1
american_toad.Info.Redeals          = 1
american_toad.Stock.Initial         = piles.Initial.face_down(75)
american_toad.Reserve               = {
    Initial = piles.Initial.face_up(20),
    Layout = "Column"
}
american_toad.Foundation.Size       = 8
american_toad.Tableau               = {
    Size   = 8,
    create = {
        Initial = piles.Initial.face_up(1),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Wrap = true, Move = "TopOrPile", Empty = "Any" }
    }
}

------

local chameleon                     = Sol.copy(canfield)
chameleon.Info.Name                 = "Chameleon"
chameleon.Info.DeckCount            = 1
chameleon.Info.CardDealCount        = 1
chameleon.Info.Redeals              = 0
chameleon.Stock.Initial             = piles.Initial.face_down(36)
chameleon.Reserve.create            = {
    Initial = piles.Initial.top_face_up(12),
    Layout = "Column"
}
chameleon.Tableau                   = {
    Size   = 3,
    create = {
        Initial = piles.Initial.face_up(1),
        Layout = "Column",
        Rule = { Build = "DownByRank", Wrap = true, Move = "TopOrPile", Empty = "Any" }
    }
}

------

local demon                         = Sol.copy(canfield)
demon.Info.Name                     = "Demon"
demon.Info.DeckCount                = 2
demon.Stock.Initial                 = piles.Initial.face_down(55)
demon.Reserve.create                = {
    Initial = piles.Initial.top_face_up(40),
    Layout = "Column"
}
demon.Foundation.Size               = 8
demon.Tableau                       = {
    Size   = 8,
    create = {
        Initial = piles.Initial.face_up(1),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Wrap = true, Move = "InSequence", Empty = "Any" }
    }
}

------

local eagle_wing_pos                = {
    { 0, 1.75 }, { 1, 1.5 }, { 2, 1.25 }, { 3, 1.5 }, { 5, 1.5 }, { 6, 1.25 }, { 7, 1.5 }, { 8, 1.75 }
}

local eagle_wing                    = {
    Info       = {
        Name          = "Eagle Wing",
        Type          = "ReservedPacker",
        Family        = "Canfield",
        DeckCount     = 1,
        CardDealCount = 1,
        Redeals       = 2
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = piles.Initial.face_down(30)
    },
    Waste      = { Position = { x = 1, y = 0 } },
    Reserve    = {
        Position = { x = 4, y = 2 },
        Initial = piles.Initial.face_down(13),
        Layout = "Squared"
    },
    Foundation = {
        Size   = 4,
        create = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Initial = piles.Initial.face_up(i == 0 and 1 or 0),
                Rule = { Build = "UpInSuit", Wrap = true, Move = "None", Empty = "FirstFoundation" }
            }
        end
    },
    Tableau    = {
        Size   = 8,
        create = function(i)
            return {
                Position = { x = eagle_wing_pos[i + 1][1], y = eagle_wing_pos[i + 1][2] },
                Initial = piles.Initial.face_up(1),
                Layout = "Column",
                Rule = { Build = "DownInSuit", Wrap = true, Move = "Top", Empty = "None", Limit = 3 }
            }
        end
    },
    on_change  = function(game)
        game.Reserve[1]:deal_to_group(game.Tableau, true)
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
minerva.Stock.Initial               = piles.Initial.face_down(13)
minerva.Reserve.create              = {
    Initial = piles.Initial.top_face_up(11),
    Layout = "Column"
}
minerva.Foundation.create           = { Rule = { Build = "UpInSuit", Wrap = true, Move = "None", Empty = "Ace" } }
minerva.Tableau                     = {
    Size   = 7,
    create = {
        Initial = piles.Initial.alternate(4, false),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "King" }
    }
}
minerva.on_change                   = nil

------

local rainfall                      = Sol.copy(canfield)
rainfall.Info.Name                  = "Rainfall"
rainfall.Info.CardDealCount         = 1
rainfall.Info.Redeals               = 2

------

local duke                          = {
    Info       = {
        Name          = "Duke",
        Type          = "ReservedPacker",
        Family        = "Canfield",
        DeckCount     = 1,
        CardDealCount = 1,
        Redeals       = 2
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = piles.Initial.face_down(36)
    },
    Waste      = { Position = { x = 1, y = 0 } },
    Reserve    = {
        Size   = 4,
        create = function(i)
            return {
                Position = { x = i % 2 * 2, y = i // 2 + 1 },
                Initial  = piles.Initial.face_up(3),
                Layout   = "Row",
                Rule     = { Build = "NoBuilding", Move = "Top", Empty = "None" },
            }
        end
    },
    Foundation = {
        Size   = 4,
        create = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" }
            }
        end
    },
    Tableau    = {
        Size   = 4,
        create = function(i)
            return {
                Position = { x = i + 4, y = 1 },
                Initial = piles.Initial.face_up(1),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
            }
        end
    },
    on_redeal  = ops.Redeal.waste_to_stock,
    on_deal    = ops.Deal.stock_to_waste
}

------

local dutchess                      = Sol.copy(duke)
dutchess.Info.Name                  = "Dutchess"
dutchess.Info.Redeals               = 1
dutchess.Foundation.create          = function(i)
    return {
        Position = { x = i + 3, y = 0 },
        Rule = { Build = "UpInSuit", Wrap = true, Move = "None", Empty = "FirstFoundation" }
    }
end
dutchess.Tableau.create             = function(i)
    return {
        Position = { x = i + 4, y = 1 },
        Initial = piles.Initial.face_up(1),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Wrap = true, Move = "InSequence", Empty = "Any" }
    }
end
dutchess.check_drop                 = function(game, targetPile, targetIndex, drop, numCards)
    local srcPile = game:find_pile(drop)
    if game.Foundation[1].IsEmpty then
        return srcPile.Type == "Reserve" and targetPile == game.Foundation[1]
    end
    if targetPile.Type == "Tableau" and targetPile.IsEmpty then
        local reserveEmpty = true
        for _, v in ipairs(game.Reserve) do
            if v.IsEmpty then
                reserveEmpty = false
                break
            end
        end
        if (reserveEmpty and srcPile.Type ~= "Waste")
            or (not reserveEmpty and srcPile.Type ~= "Reserve") then
            return false
        end
    end

    return game:can_drop(targetPile, targetIndex, drop, numCards)
end
dutchess.on_deal                    = function(game)
    if game.Foundation[1].IsEmpty then return false end
    return ops.Deal.stock_to_waste(game)
end

------------------------

Sol.register_game(canfield)
Sol.register_game(double_canfield)
Sol.register_game(triple_canfield)
Sol.register_game(canfield_rush)
Sol.register_game(acme)
Sol.register_game(american_toad)
Sol.register_game(chameleon)
Sol.register_game(demon)
Sol.register_game(duke)
Sol.register_game(dutchess)
Sol.register_game(eagle_wing)
Sol.register_game(minerva)
Sol.register_game(rainfall)
Sol.register_game(superior_canfield)
Sol.register_game(variegated_canfield)
