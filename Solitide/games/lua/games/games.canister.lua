-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local canister                 = {
    Info             = {
        Name          = "Canister",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation       = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_none }
    },
    Tableau          = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up((i < 2 or i >= 6) and 6 or 7),
                Layout = "Column",
                Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.InSeq() }
            }
        end
    },
    on_piles_created = Sol.Layout.canister
}

------

local american_canister        = Sol.copy(canister)
american_canister.Info.Name    = "American Canister"
american_canister.Tableau.Pile = function(i)
    return {
        Initial = Sol.Initial.face_up((i < 2 or i >= 6) and 6 or 7),
        Layout = "Column",
        Rule = Sol.Rules.any_downac_inseq
    }
end

------

local british_canister         = Sol.copy(canister)
british_canister.Info.Name     = "British Canister"
british_canister.Tableau.Pile  = function(i)
    return {
        Initial = Sol.Initial.face_up((i < 2 or i >= 6) and 6 or 7),
        Layout = "Column",
        Rule = Sol.Rules.king_downac_top
    }
end

------

------------

Sol.register_game(canister)
Sol.register_game(american_canister)
Sol.register_game(british_canister)
