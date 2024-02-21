-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local layout = require 'base/layout'
local ops    = require 'base/ops'
local piles  = require 'base/piles'
require 'base/common'

local gypsy                    = {
    Info       = {
        Name          = "Gypsy",
        Type          = "Packer",
        Family        = "Gypsy",
        DeckCount     = 2,
        CardDealCount = 8,
        Redeals       = 0
    },
    Stock      = {
        Initial = piles.initial.face_down(80)
    },
    Foundation = {
        Size   = 8,
        create = piles.ace_upsuit_top
    },
    Tableau    = {
        Size   = 8,
        create = {
            Initial = piles.initial.top_face_up(3),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
        }
    },
    layout     = layout.gypsy,
    deal       = function(game) return game.Stock[1]:deal_to_group(game.Tableau, false) end
}

------

local agnes_sorel              = Copy(gypsy)
agnes_sorel.Info.Name          = "Agnes Sorel"
agnes_sorel.Info.DeckCount     = 1
agnes_sorel.Info.CardDealCount = 7
agnes_sorel.Stock.Initial      = piles.initial.face_down(23)
agnes_sorel.Foundation         = {
    Size   = 4,
    create = function(i)
        return {
            Initial = i == 0 and piles.initial.face_up(1) or {},
            Rule = { Build = "UpInSuit", Wrap = true, Move = "None", Empty = "FirstFoundation" }
        }
    end
}
agnes_sorel.Tableau            = {
    Size   = 7,
    create = function(i)
        return {
            Initial = piles.initial.face_up(i + 1),
            Layout = "Column",
            Rule = { Build = "DownInColor", Move = "InSequence", Empty = "None" }
        }
    end
}
agnes_sorel.layout             = layout.klondike

------

local blockade                 = Copy(gypsy)
blockade.Info.Name             = "Blockade"
blockade.Info.CardDealCount    = 12
blockade.Stock.Initial         = piles.initial.face_down(92)
blockade.Tableau               = {
    Size   = 12,
    create = {
        Initial = piles.initial.face_up(1),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "InSequence", Empty = "Any" }
    }
}
blockade.layout                = layout.klondike
blockade.before_layout         = function(game) game.Stock[1]:deal_to_group(game.Tableau, true) end

------

local lexington_harp           = Copy(gypsy)
lexington_harp.Info.Name       = "Lexington Harp"
--lexington_harp.Info.Family = "Gypsy/Yukon"
lexington_harp.Stock.Initial   = piles.initial.face_down(68)
lexington_harp.Foundation      = {
    Size   = 8,
    create = { Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" } }
}
lexington_harp.Tableau         = {
    Size   = 8,
    create = function(i)
        return {
            Initial = piles.initial.top_face_up(i + 1),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "FaceUp", Empty = "Any" }
        }
    end
}

------

local brunswick                = Copy(lexington_harp)
brunswick.Info.Name            = "Brunswick"
--brunswick.Info.Family = "Gypsy/Yukon"
brunswick.Tableau.create       = function(i)
    return {
        Initial = piles.initial.face_up(i + 1),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "FaceUp", Empty = "Any" }
    }
end

------

local milligan_harp            = Copy(lexington_harp)
milligan_harp.Info.Name        = "Milligan Harp"
--milligan_harp.Info.Family = "Gypsy/Yukon"
milligan_harp.Tableau.create   = function(i)
    return {
        Initial = piles.initial.top_face_up(i + 1),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
    }
end

------

local mississippi              = Copy(lexington_harp)
mississippi.Info.Name          = "Mississippi"
--milligan_harp.Info.Family = "Gypsy/Yukon"
mississippi.Stock.Initial      = piles.initial.face_down(76)
mississippi.Tableau            = {
    Size   = 7,
    create = function(i)
        return {
            Initial = piles.initial.top_face_up(i + 1),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "FaceUp", Empty = "Any" }
        }
    end
}

------

local cone                     = {
    Info       = {
        Name          = "Cone",
        Type          = "Packer",
        Family        = "Gypsy",
        DeckCount     = 2,
        CardDealCount = 7,
        Redeals       = 0
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = piles.initial.face_down(88)
    },
    Reserve    = {
        Size   = 4,
        create = function(i)
            return {
                Position = { x = 0, y = i + 1 },
                Rule = { Build = "NoBuilding", Move = "Top", Empty = "None" }
            }
        end
    },
    Foundation = {
        Size   = 4,
        create = function(i)
            return {
                Position = { x = 8, y = i + 1 },
                Rule = { Build = "UpInSuit", Wrap = true, Move = "Top", Empty = "Ace" }
            }
        end
    },
    Tableau    = {
        Size   = 7,
        create = function(i)
            return {
                Position = { x = i + 1, y = 0 },
                Initial = piles.initial.top_face_up(i < 4 and i + 1 or 7 - i),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
            }
        end
    },
    deal       = function(game)
        --check if tableau is empty
        local tableau = game.Tableau
        for k, v in pairs(tableau) do
            if v.Empty then return false end
        end
        --deal last 4 cards to reserve
        if game.Stock[1].CardCount == 4 then
            return game.Stock[1]:deal_to_group(game.Reserve, false)
        end

        return game.Stock[1]:deal_to_group(game.Tableau, false)
    end
}

------------------------

RegisterGame(gypsy)
RegisterGame(agnes_sorel)
RegisterGame(blockade)
RegisterGame(brunswick)
RegisterGame(cone)
RegisterGame(lexington_harp)
RegisterGame(milligan_harp)
RegisterGame(mississippi)
