-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local la_belle_lucie             = {
    Info       = {
        Name      = "La Belle Lucie",
        Family    = "Fan",
        DeckCount = 1,
        Redeals   = 2
    },
    Stock      = {},
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 18,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i == 17 and 1 or 3),
                Layout  = Sol.Pile.Layout.Row,
                Rule    = Sol.Rules.none_downsuit_top
            }
        end
    },
    redeal     = function(game)
        local tableau = game.Tableau

        -- shuffle
        local cards = Sol.shuffle_piles(game, { tableau, game.Stock })
        if #cards == 0 then return false end

        -- redeal
        for _, tab in ipairs(tableau) do
            local i = 1
            while i <= 3 do
                if #cards == 0 then break end

                local card = table.remove(cards)
                game.PlaceTop(card, tab, false)
                i = i + 1
            end
            tab:flip_up_cards()
        end

        return true
    end,
    on_init    = function(game) Sol.Layout.fan(game, 5) end,
}

------

local super_flower_garden        = Sol.copy(la_belle_lucie)
super_flower_garden.Info.Name    = "Super Flower Garden"
super_flower_garden.Tableau.Pile = function(i)
    return {
        Initial = Sol.Initial.face_up(i == 17 and 1 or 3),
        Layout  = Sol.Pile.Layout.Row,
        Rule    = Sol.Rules.none_downrank_top
    }
end


------

local trefoil           = Sol.copy(la_belle_lucie)
trefoil.Info.Name       = "Trefoil"
trefoil.Tableau         = {
    Size = 16,
    Pile = {
        Initial = Sol.Initial.face_up(3),
        Layout  = Sol.Pile.Layout.Row,
        Rule    = Sol.Rules.none_downsuit_top
    }
}
trefoil.on_before_setup = Sol.Ops.Setup.ace_to_foundation


------

local school     = Sol.copy(trefoil)
school.Info.Name = "School"
school.Tableau   = {
    Size = 16,
    Pile = {
        Initial = Sol.Initial.face_up(3),
        Layout  = Sol.Pile.Layout.Row,
        Rule    = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.Top() }
    }
}


------

------------------------

Sol.register_game(la_belle_lucie)
Sol.register_game(school)
Sol.register_game(super_flower_garden)
Sol.register_game(trefoil)
