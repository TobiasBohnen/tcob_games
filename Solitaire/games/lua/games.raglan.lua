-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local piles = require 'base/piles'
local rules = require 'base/rules'


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
            Initial = piles.Initial.face_up(1),
            Layout = "Squared",
            Rule = { Build = "NoBuilding", Move = "Top", Empty = rules.Empty.none }
        }
    },
    Foundation        = {
        Size   = 4,
        create = { Rule = { Build = "UpInSuit", Move = "Top", Empty = rules.Empty.ace } }
    },
    Tableau           = {
        Size   = 9,
        create = function(i)
            return {
                Initial = piles.Initial.face_up(i < 7 and i + 1 or 7),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "Top", Empty = rules.Empty.any }
            }
        end
    },
    on_before_shuffle = ops.Shuffle.ace_to_foundation,
    on_created        = Sol.Layout.raglan
}

------------------------

Sol.register_game(raglan)
