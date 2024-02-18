-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local layout = require 'base/layout'
local ops    = require 'base/ops'
local piles  = require 'base/piles'
require 'base/common'

local raglan = {
    Info           = {
        Name          = "Raglan",
        Type          = "OpenPacker",
        Family        = "Raglan",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Reserve        = {
        Size   = 6,
        create = function(i)
            return {
                Initial = piles.initial.face_up(1),
                Layout = "Squared",
                Rule = { Build = "NoBuilding", Move = "Top", Empty = "None" }
            }
        end
    },
    Foundation     = {
        Size   = 4,
        create = function() return { Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" } } end
    },
    Tableau        = {
        Size   = 9,
        create = function(i)
            return {
                Initial = piles.initial.face_up(i < 7 and i + 1 or 7),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "Top", Empty = "Any" }
            }
        end
    },
    before_shuffle = ops.shuffle.ace_to_foundation,
    layout         = layout.raglan
}

------------------------

register_game(raglan)
