-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local bastion = {
    Info       = {
        Name      = "Bastion",
        Family    = "BeleagueredCastle",
        DeckCount = 1
    },
    FreeCell   = {
        Size = 2,
        Pile = {
            Initial = Sol.Initial.face_up(1),
            Rule    = Sol.Rules.any_none_top
        }
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 10,
        Pile = {
            Initial = Sol.Initial.face_up(5),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.any_updownsuit_top
        }
    },
    on_init    = Sol.Layout.free_cell
}


------

local castles_end           = Sol.copy(bastion)
castles_end.Info.Name       = "Castles End"
castles_end.Foundation.Pile = { Rule = Sol.Rules.ff_upsuit_top }
castles_end.Tableau.Pile    = {
    Initial = Sol.Initial.face_up(5),
    Layout  = Sol.Pile.Layout.Column,
    Rule    = Sol.Rules.any_updownac_top
}
castles_end.can_play        = function(game, targetPile, targetCardIndex, card, numCards)
    local foundation1 = game.Foundation[1]
    if foundation1.IsEmpty then -- block card drops if foundation is empty
        return targetPile == foundation1
    end

    return game:can_play(targetPile, targetCardIndex, card, numCards)
end


------

local ten_by_one        = Sol.copy(bastion)
ten_by_one.Info.Name    = "Ten by One"
ten_by_one.FreeCell     = { Rule = Sol.Rules.any_none_top }
ten_by_one.Tableau.Pile = function(i)
    return {
        Initial = Sol.Initial.face_up(i < 2 and 6 or 5),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_updownsuit_top
    }
end


------

------------

Sol.register_game(bastion)
Sol.register_game(castles_end)
Sol.register_game(ten_by_one)
