-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local piles = require 'base/piles'


local raglan = {
    Info              = {
        Name          = "Raglan",
        Type          = "OpenPacker",
        Family        = "Raglan",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Reserve           = {
        Size   = 6,
        create = {
            Initial = piles.initial.face_up(1),
            Layout = "Squared",
            Rule = { Build = "NoBuilding", Move = "Top", Empty = "None" }
        }
    },
    Foundation        = {
        Size   = 4,
        create = { Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" } }
    },
    Tableau           = {
        Size   = 9,
        create = function(i)
            return {
                Initial = piles.initial.face_up(i < 7 and i + 1 or 7),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "Top", Empty = "Any" }
            }
        end
    },
    on_before_shuffle = ops.shuffle.ace_to_foundation,
    on_created        = Layout.raglan
}

------------------------

RegisterGame(raglan)
