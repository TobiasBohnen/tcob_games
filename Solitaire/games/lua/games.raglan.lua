-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
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
            Initial = ops.Initial.face_up(1),
            Layout = "Squared",
            Rule = rules.none_none_top
        }
    },
    Foundation        = {
        Size   = 4,
        Create = { Rule = rules.ace_upsuit_top }
    },
    Tableau           = {
        Size   = 9,
        Create = function(i)
            return {
                Initial = ops.Initial.face_up(i < 7 and i + 1 or 7),
                Layout = "Column",
                Rule = rules.any_downac_top
            }
        end
    },
    on_before_shuffle = ops.Shuffle.ace_to_foundation,
    on_created        = Sol.Layout.raglan
}

------------------------

Sol.register_game(raglan)
