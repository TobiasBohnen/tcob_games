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
        Create = {
            Initial = piles.Initial.face_up(1),
            Layout = "Squared",
            Rule = { Build = rules.Build.NoBuilding, Move = rules.Move.Top, Empty = rules.Empty.None }
        }
    },
    Foundation        = {
        Size   = 4,
        Create = { Rule = { Build = rules.Build.UpInSuit, Move = rules.Move.Top, Empty = rules.Empty.Ace } }
    },
    Tableau           = {
        Size   = 9,
        Create = function(i)
            return {
                Initial = piles.Initial.face_up(i < 7 and i + 1 or 7),
                Layout = "Column",
                Rule = { Build = rules.Build.DownAlternateColors, Move = rules.Move.Top, Empty = rules.Empty.Any }
            }
        end
    },
    on_before_shuffle = ops.Shuffle.ace_to_foundation,
    on_created        = Sol.Layout.raglan
}

------------------------

Sol.register_game(raglan)
