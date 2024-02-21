-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local layout = require 'base/layout'
local ops    = require 'base/ops'
local piles  = require 'base/piles'
require 'base/common'

local canfield                   = {
    Info          = {
        Name          = "Canfield",
        Type          = "ReservedPacker",
        Family        = "Canfield",
        DeckCount     = 1,
        CardDealCount = 3,
        Redeals       = -1
    },
    Stock         = { Initial = piles.initial.face_down(34) },
    Waste         = {},
    Reserve       = {
        Initial = piles.initial.top_face_up(13),
        Layout = "Column"
    },
    Foundation    = {
        Size   = 4,
        create = function(i)
            return {
                Initial = piles.initial.face_up(i == 0 and 1 or 0),
                Rule = { Build = "UpInSuit", Wrap = true, Move = "None", Empty = "FirstFoundation" }
            }
        end
    },
    Tableau       = {
        Size   = 4,
        create = {
            Initial = piles.initial.face_up(1),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Wrap = true, Move = "TopOrPile", Empty = "Any" }
        }
    },
    before_layout = function(game) game.Reserve[1]:deal_to_group(game.Tableau, true) end,
    redeal        = ops.redeal.waste_to_stock,
    deal          = ops.deal.stock_to_waste,
    layout        = layout.canfield
}

------

local canfield_rush              = Copy(canfield)
canfield_rush.Info.Name          = "Canfield Rush"
canfield_rush.Info.Redeals       = 2
canfield_rush.deal               = ops.deal.stock_to_waste_by_redeals_left

------

local double_canfield            = Copy(canfield)
double_canfield.Info.Name        = "Double Canfield"
double_canfield.Info.DeckCount   = 2
double_canfield.Stock.Initial    = piles.initial.face_down(85)
double_canfield.Foundation.Size  = 8
double_canfield.Tableau.Size     = 5

------

local acme                       = Copy(canfield)
acme.Info.Name                   = "Acme"
acme.Info.CardDealCount          = 1
acme.Info.Redeals                = 1
acme.Stock.Initial               = piles.initial.face_down(31)
acme.Foundation.create           = { Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" } }
acme.Tableau.create              = {
    Initial = piles.initial.face_up(1),
    Layout = "Column",
    Rule = { Build = "DownInSuit", Move = "Top", Empty = "Any" }
}
acme.before_shuffle              = ops.shuffle.ace_to_foundation

------

local american_toad              = Copy(canfield)
american_toad.Info.Name          = "American Toad"
american_toad.Info.DeckCount     = 2
american_toad.Info.CardDealCount = 1
american_toad.Info.Redeals       = 1
american_toad.Stock.Initial      = piles.initial.face_down(75)
american_toad.Reserve            = {
    Initial = piles.initial.face_up(20),
    Layout = "Column"
}
american_toad.Foundation.Size    = 8
american_toad.Tableau            = {
    Size   = 8,
    create = {
        Initial = piles.initial.face_up(1),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Wrap = true, Move = "TopOrPile", Empty = "Any" }
    }
}

------

local chameleon                  = Copy(canfield)
chameleon.Info.Name              = "Chameleon"
chameleon.Info.DeckCount         = 1
chameleon.Info.CardDealCount     = 1
chameleon.Info.Redeals           = 0
chameleon.Stock.Initial          = piles.initial.face_down(36)
chameleon.Reserve.create         = {
    Initial = piles.initial.top_face_up(12),
    Layout = "Column"
}
chameleon.Tableau                = {
    Size   = 3,
    create = {
        Initial = piles.initial.face_up(1),
        Layout = "Column",
        Rule = { Build = "DownByRank", Wrap = true, Move = "TopOrPile", Empty = "Any" }
    }
}

------

local demon                      = Copy(canfield)
demon.Info.Name                  = "Demon"
demon.Info.DeckCount             = 2
demon.Stock.Initial              = piles.initial.face_down(55)
demon.Reserve.create             = {
    Initial = piles.initial.top_face_up(40),
    Layout = "Column"
}
demon.Foundation.Size            = 8
demon.Tableau                    = {
    Size   = 8,
    create = {
        Initial = piles.initial.face_up(1),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Wrap = true, Move = "InSequence", Empty = "Any" }
    }
}

------

local minerva                    = Copy(canfield)
minerva.Info.Name                = "Minerva"
minerva.Info.CardDealCount       = 1
minerva.Info.Redeals             = 1
minerva.Stock.Initial            = piles.initial.face_down(13)
minerva.Reserve.create           = {
    Initial = piles.initial.top_face_up(11),
    Layout = "Column"
}
minerva.Foundation.create        = { Rule = { Build = "UpInSuit", Wrap = true, Move = "None", Empty = "Ace" } }
minerva.Tableau                  = {
    Size   = 7,
    create = {
        Initial = piles.initial.alternate(4, false),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "King" }
    }
}
minerva.before_layout            = nil

------

local rainfall                   = Copy(canfield)
rainfall.Info.Name               = "Rainfall"
rainfall.Info.CardDealCount      = 1
rainfall.Info.Redeals            = 2

------

local duke                       = {
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
        Initial = piles.initial.face_down(36)
    },
    Waste      = { Position = { x = 1, y = 0 } },
    Reserve    = {
        Size   = 4,
        create = function(i)
            return {
                Position = { x = i % 2 * 2, y = i // 2 + 1 },
                Initial  = piles.initial.face_up(3),
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
                Initial = piles.initial.face_up(1),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
            }
        end
    },
    redeal     = ops.redeal.waste_to_stock,
    deal       = ops.deal.stock_to_waste
}

------

local dutchess                   = Copy(duke)
dutchess.Info.Name               = "Dutchess"
dutchess.Info.Redeals            = 1
dutchess.Foundation.create       = function(i)
    return {
        Position = { x = i + 3, y = 0 },
        Rule = { Build = "UpInSuit", Wrap = true, Move = "None", Empty = "FirstFoundation" }
    }
end
dutchess.Tableau.create          = function(i)
    return {
        Position = { x = i + 4, y = 1 },
        Initial = piles.initial.face_up(1),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Wrap = true, Move = "InSequence", Empty = "Any" }
    }
end
dutchess.can_drop                = function(game, targetPile, targetIndex, drop, numCards)
    local srcPile = game:find_pile(drop)
    if game.Foundation[1].Empty then
        return srcPile.Type == "Reserve" and targetPile == game.Foundation[1]
    end
    if targetPile.Type == "Tableau" and targetPile.Empty then
        local reserveEmpty = true
        for _, v in pairs(game.Reserve) do
            if v.Empty then
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
dutchess.deal                    = function(game)
    if game.Foundation[1].Empty then return false end
    return ops.deal.stock_to_waste(game)
end

------------------------

RegisterGame(canfield)
RegisterGame(double_canfield)
RegisterGame(canfield_rush)
RegisterGame(acme)
RegisterGame(american_toad)
RegisterGame(chameleon)
RegisterGame(demon)
RegisterGame(duke)
RegisterGame(dutchess)
RegisterGame(minerva)
RegisterGame(rainfall)
