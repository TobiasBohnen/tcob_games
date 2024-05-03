-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ten_across_initial <const> = {
    { true,  true,  true,  true,  true },
    { false, true,  true,  true,  true },
    { false, false, true,  true,  true },
    { false, false, false, true,  true },
    { false, false, false, false, true },
    { false, false, false, false, true },
    { false, false, false, true,  true },
    { false, false, true,  true,  true },
    { false, true,  true,  true,  true },
    { true,  true,  true,  true,  true }
}

local ten_across                 = {
    Info       = {
        Name      = "Ten Across",
        Family    = "Spider",
        --Family = "Yukon/Spider"
        DeckCount = 1
    },
    FreeCell   = {
        Size = 2,
        Pile = {
            Initial = Sol.Initial.face_up(1),
            Rule = Sol.Rules.any_none_top
        }
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.spider_foundation }
    },
    Tableau    = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = ten_across_initial[i + 1],
                Layout = "Column",
                Rule = Sol.Rules.king_downsuit_faceup
            }
        end
    },
    on_init    = Sol.Layout.free_cell
}

------

local panopticon_initial <const> = {
    { false, true,  true,  true,  true, true },
    { false, false, true,  true,  true, true },
    { false, false, false, true,  true, true },
    { false, false, false, false, true, true },
    { false, false, false, false, true, true },
    { false, false, false, true,  true, true },
    { false, false, true,  true,  true, true },
    { false, true,  true,  true,  true, true }
}

local panopticon                 = Sol.copy(ten_across)
panopticon.Info.Name             = "Panopticon"
panopticon.FreeCell.Size         = 4
panopticon.Tableau               = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = panopticon_initial[i + 1],
            Layout = "Column",
            Rule = Sol.Rules.king_downsuit_faceup
        }
    end
}

------------------------

Sol.register_game(ten_across)
Sol.register_game(panopticon)
