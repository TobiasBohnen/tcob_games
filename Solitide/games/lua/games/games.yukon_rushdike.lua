-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

-- Yukon games with Stock

local initial <const>            = {
    chinese_discipline = function(i)
        if i < 3 then
            return Sol.Initial.face_up(7)
        else
            local t = Sol.Initial.face_down(7)
            for j = i, 7 do
                t[j] = true
            end
            return t
        end
    end
}

local rushdike                   = {
    Info = {
        Name      = "Rushdike",
        Family    = "Yukon",
        --Family = "Gypsy/Yukon/Klondike"
        DeckCount = 1
    },
    Stock = { Initial = Sol.Initial.face_down(24) },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_none }
    },
    Tableau = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = Sol.Initial.top_face_up(i + 1),
                Layout = "Column",
                Rule = Sol.Rules.king_downsuit_faceup
            }
        end
    },
    on_init = Sol.Layout.klondike,
    do_deal = Sol.Ops.Deal.stock_to_tableau
}

------

local chinese_discipline         = Sol.copy(rushdike)
chinese_discipline.Info.Name     = "Chinese Discipline"
chinese_discipline.Stock.Initial = Sol.Initial.face_down(3)
chinese_discipline.Tableau.Pile  = function(i)
    return {
        Initial = initial.chinese_discipline(i),
        Layout = "Column",
        Rule = Sol.Rules.king_downac_faceup
    }
end

------

local chinese_solitaire          = Sol.copy(chinese_discipline)
chinese_solitaire.Info.Name      = "Chinese Solitaire"
chinese_solitaire.Tableau.Pile   = function(i)
    return {
        Initial = initial.chinese_discipline(i),
        Layout = "Column",
        Rule = Sol.Rules.any_downac_faceup
    }
end


------

local queenie               = Sol.copy(rushdike)
queenie.Info.Name           = "Queenie"
queenie.Tableau.Pile        = function(i)
    return {
        Initial = Sol.Initial.face_up(i + 1),
        Layout = "Column",
        Rule = Sol.Rules.king_downac_faceup
    }
end

------

local russian_point         = Sol.copy(rushdike)
russian_point.Info.Name     = "Russian Point"
russian_point.Stock.Initial = Sol.Initial.face_down(27)
russian_point.Tableau.Pile  = function(i)
    return {
        Initial = Sol.Initial.top_face_up(i < 4 and i * 2 + 1 or (6 - i) * 2 + 1),
        Layout = "Column",
        Rule = Sol.Rules.king_downsuit_faceup
    }
end

------

------------------------

Sol.register_game(rushdike)
Sol.register_game(chinese_discipline)
Sol.register_game(chinese_solitaire)
Sol.register_game(queenie)
Sol.register_game(russian_point)
