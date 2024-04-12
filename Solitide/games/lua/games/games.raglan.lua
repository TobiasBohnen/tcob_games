-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local raglan                  = {
    Info              = {
        Name          = "Raglan",
        Family        = "Raglan",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Reserve           = {
        Size = 6,
        Pile = {
            Initial = Sol.Initial.face_up(1),
            Layout = "Squared",
            Rule = Sol.Rules.none_none_top
        }
    },
    Foundation        = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau           = {
        Size = 9,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 7 and i + 1 or 7),
                Layout = "Column",
                Rule = Sol.Rules.any_downac_top
            }
        end
    },
    on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation,
    on_init           = function(game) Sol.Layout.raglan(game, 2) end
}

------

local relaxed_raglan          = Sol.copy(raglan)
relaxed_raglan.Info.Name      = "Relaxed Raglan"
relaxed_raglan.Tableau.Pile   = function(i)
    return {
        Initial = Sol.Initial.face_up(i < 7 and i + 1 or 7),
        Layout = "Column",
        Rule = Sol.Rules.any_downac_inseq
    }
end

------

local king_albert             = Sol.copy(raglan)
king_albert.Info.Name         = "King Albert"
king_albert.Reserve.Size      = 7
king_albert.Tableau.Pile      = function(i)
    return {
        Initial = Sol.Initial.face_up(i + 1),
        Layout = "Column",
        Rule = Sol.Rules.any_downac_top
    }
end
king_albert.on_before_shuffle = nil

------

local queen_victoria          = Sol.copy(king_albert)
queen_victoria.Info.Name      = "Queen Victoria"
queen_victoria.Tableau.Pile   = function(i)
    return {
        Initial = Sol.Initial.face_up(i + 1),
        Layout = "Column",
        Rule = Sol.Rules.any_downac_inseq
    }
end

------------------------

Sol.register_game(raglan)
Sol.register_game(king_albert)
Sol.register_game(queen_victoria)
Sol.register_game(relaxed_raglan)
