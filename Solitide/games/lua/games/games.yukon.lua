-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local initial <const> = {
    yukon           = function(i)
        if i == 0 then
            return { true }
        else
            local t = {}
            for j = 1, i + 5 do
                t[j] = j > i
            end
            return t
        end
    end,
    double_yukon    = function(i)
        local t = {}
        local ii = math.min(i, 8)
        for j = 1, ii + 6 do
            t[j] = j > ii
        end
        return t
    end,
    triple_yukon    = function(i)
        local t = Sol.Initial.face_down(i + 6)
        for j = #t - 5, #t do
            t[j] = true
        end
        return t
    end,
    quadruple_yukon = function(i)
        if i == 0 then
            return { true }
        elseif i <= 12 then
            local t = Sol.Initial.face_down(i + 6)
            for j = #t - 5, #t do
                t[j] = true
            end
            return t
        else
            local t = Sol.Initial.face_down(i + 5)
            for j = #t - 4, #t do
                t[j] = true
            end
            return t
        end
    end
}

------

local yukon           = {
    Info       = {
        Name      = "Yukon",
        Family    = "Yukon",
        DeckCount = 1
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_none }
    },
    Tableau    = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = initial.yukon(i),
                Layout = "Column",
                Rule = Sol.Rules.king_downac_faceup
            }
        end
    },
    on_init    = Sol.Layout.yukon
}


------

local double_yukon           = Sol.copy(yukon)
double_yukon.Info.Name       = "Double Yukon"
double_yukon.Info.DeckCount  = 2
double_yukon.Foundation.Size = 8
double_yukon.Tableau         = {
    Size = 10,
    Pile = function(i)
        return {
            Initial = initial.double_yukon(i),
            Layout = "Column",
            Rule = Sol.Rules.king_downac_faceup
        }
    end
}


------

local triple_yukon           = Sol.copy(yukon)
triple_yukon.Info.Name       = "Triple Yukon"
triple_yukon.Info.DeckCount  = 3
triple_yukon.Foundation.Size = 12
triple_yukon.Tableau         = {
    Size = 13,
    Pile = function(i)
        return {
            Initial = initial.triple_yukon(i),
            Layout = "Column",
            Rule = Sol.Rules.king_downac_faceup
        }
    end
}


------

local quadruple_yukon           = Sol.copy(yukon)
quadruple_yukon.Info.Name       = "Quadruple Yukon"
quadruple_yukon.Info.DeckCount  = 4
quadruple_yukon.Foundation.Size = 16
quadruple_yukon.Tableau         = {
    Size = 16,
    Pile = function(i)
        return {
            Initial = initial.quadruple_yukon(i),
            Layout = "Column",
            Rule = Sol.Rules.king_downac_faceup
        }
    end
}


------

local alaska        = Sol.copy(yukon)
alaska.Info.Name    = "Alaska"
alaska.Tableau.Pile = function(i)
    return {
        Initial = initial.yukon(i),
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.UpOrDownInSuit(), Move = Sol.Rules.Move.FaceUp() }
    }
end


------

local brisbane_initial = {
    { true,  true,  true,  true,  true },
    { false, true,  true,  true,  true,  true },
    { false, false, true,  true,  true,  true,  true },
    { false, false, false, true,  true,  true,  true },
    { false, false, false, false, true,  true,  true, true },
    { false, false, false, false, false, true,  true, true, true },
    { false, false, false, false, false, false, true, true, true, true }
}

local brisbane         = Sol.copy(yukon)
brisbane.Info.Name     = "Brisbane"
brisbane.Tableau.Pile  = function(i)
    return {
        Initial = brisbane_initial[i + 1],
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.FaceUp() }
    }
end


------

local geoffrey     = Sol.copy(yukon)
geoffrey.Info.Name = "Geoffrey"
geoffrey.Tableau   = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = i < 4 and Sol.Initial.face_up(7) or { false, false, false, false, true, true },
            Layout = "Column",
            Rule = Sol.Rules.king_downsuit_faceup
        }
    end
}
geoffrey.on_init   = Sol.Layout.forty_thieves


------

local queensland        = Sol.copy(yukon)
queensland.Info.Name    = "Queensland"
queensland.Tableau.Pile = function(i)
    return {
        Initial = brisbane_initial[i + 1],
        Layout = "Column",
        Rule = Sol.Rules.any_downsuit_faceup
    }
end


------

local roslin        = Sol.copy(yukon)
roslin.Info.Name    = "Roslin"
roslin.Tableau.Pile = function(i)
    return {
        Initial = initial.yukon(i),
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.UpOrDownAlternateColors(), Move = Sol.Rules.Move.FaceUp() }
    }
end


------

local moosehide        = Sol.copy(yukon)
moosehide.Info.Name    = "Moosehide"
moosehide.Tableau.Pile = function(i)
    return {
        Initial = initial.yukon(i),
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownAnyButOwnSuit(), Move = Sol.Rules.Move.FaceUp() }
    }
end


------

local russian_solitaire        = Sol.copy(yukon)
russian_solitaire.Info.Name    = "Russian Solitaire"
russian_solitaire.Tableau.Pile = function(i)
    return {
        Initial = initial.yukon(i),
        Layout = "Column",
        Rule = Sol.Rules.king_downsuit_faceup
    }
end


------

local double_russian_solitaire           = Sol.copy(russian_solitaire)
double_russian_solitaire.Info.Name       = "Double Russian Solitaire"
double_russian_solitaire.Info.DeckCount  = 2
double_russian_solitaire.Foundation.Size = 8
double_russian_solitaire.Tableau         = {
    Size = 10,
    Pile = function(i)
        return {
            Initial = initial.double_yukon(i),
            Layout = "Column",
            Rule = Sol.Rules.king_downsuit_faceup
        }
    end
}


------

local triple_russian_solitaire           = Sol.copy(russian_solitaire)
triple_russian_solitaire.Info.Name       = "Triple Russian Solitaire"
triple_russian_solitaire.Info.DeckCount  = 3
triple_russian_solitaire.Foundation.Size = 12
triple_russian_solitaire.Tableau         = {
    Size = 13,
    Pile = function(i)
        return {
            Initial = initial.triple_yukon(i),
            Layout = "Column",
            Rule = Sol.Rules.king_downsuit_faceup
        }
    end
}


------

local hawaiian = {
    Info       = {
        Name      = "Hawaiian",
        Family    = "Yukon",
        DeckCount = 2
    },
    Reserve    = {
        Position = { x = 0, y = 0 },
        Initial  = Sol.Initial.face_up(54),
        Layout   = "Squared"
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Rule = Sol.Rules.ace_upsuit_none
            }
        end
    },
    Tableau    = {
        Size = 10,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial = Sol.Initial.face_up(5),
                Layout = "Column",
                Rule = Sol.Rules.any_downac_faceup
            }
        end
    }
}


------

local wave = {
    Info = {
        Name      = "Wave",
        Family    = "Yukon",
        --Family = "Gypsy/Yukon/Klondike"
        DeckCount = 2
    },
    Stock = { Initial = Sol.Initial.face_down(80) },
    Foundation = {
        Size = 8,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau = {
        Size = 8,
        Pile = {
            Initial = Sol.Initial.alternate(3, true),
            Layout = "Column",
            Rule = Sol.Rules.any_downac_faceup
        }
    },
    on_init = Sol.Layout.klondike,
    deal = function(game)
        local from = game.Stock[1]
        local to = game.Tableau
        if from.IsEmpty then return false end
        for _, toPile in ipairs(to) do
            from:move_cards(toPile, #from.Cards - 1, 2, false)
            toPile:flip_up_top_card()
        end
        return true
    end
}


------

local yukon_cells = {
    Info       = {
        Name      = "Yukon Cells",
        Family    = "Yukon",
        DeckCount = 1
    },
    FreeCell   = {
        Size = 2,
        Pile = { Rule = Sol.Rules.any_none_top }
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_none }
    },
    Tableau    = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = initial.yukon(i),
                Layout = "Column",
                Rule = Sol.Rules.king_downac_faceup
            }
        end
    },
    on_init    = Sol.Layout.free_cell
}


------

local yukon_kings = {
    Info       = {
        Name      = "Yukon Kings",
        Family    = "Yukon",
        DeckCount = 1
    },
    Tableau    = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = initial.yukon(i),
                Layout = "Column",
                Rule = Sol.Rules.king_downac_faceup
            }
        end
    },
    on_init    = Sol.Layout.yukon,
    get_status = function(game)
        local function check(tableau)
            local cards = tableau.Cards

            for i = 0, 11 do
                local card0 = cards[#cards - i]
                local card1 = cards[#cards - i - 1]
                if card0.IsFaceDown or card1.IsFaceDown then return false end
                if card0.Color == card1.Color then return false end
                if Sol.get_rank(card0.Rank, 1, false) ~= card1.Rank then return false end
            end

            return true
        end

        local count = 0
        for _, tableau in ipairs(game.Tableau) do
            if tableau.CardCount == 13 and check(tableau) then
                count = count + 1
            end
        end
        return count == 4 and "Success" or "Running"
    end
}


------

local yukonic_plaque = {
    Info       = {
        Name      = "Yukonic Plague",
        Family    = "Yukon",
        DeckCount = 1
    },
    Reserve    = {
        Position = { x = 0, y = 0 },
        Initial  = Sol.Initial.face_up(13),
        Layout   = "Squared"
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Rule = Sol.Rules.ace_upsuit_none
            }
        end
    },
    Tableau    = {
        Size = 7,
        Pile = function(i)
            local ini = {}
            if i == 0 then
                ini = { true }
            else
                ini = { false, false }
                for _ = 1, math.min(i + 1, 6) do table.insert(ini, true) end
            end
            return {
                Position = { x = i, y = 1 },
                Initial = ini,
                Layout = "Column",
                Rule = Sol.Rules.king_downac_faceup
            }
        end
    }
}


------

------------------------

Sol.register_game(yukon)
Sol.register_game(alaska)
Sol.register_game(brisbane)
Sol.register_game(double_russian_solitaire)
Sol.register_game(double_yukon)
Sol.register_game(geoffrey)
Sol.register_game(hawaiian)
Sol.register_game(moosehide)
Sol.register_game(quadruple_yukon)
Sol.register_game(queensland)
Sol.register_game(roslin)
Sol.register_game(russian_solitaire)
Sol.register_game(triple_russian_solitaire)
Sol.register_game(triple_yukon)
Sol.register_game(wave)
Sol.register_game(yukon_cells)
Sol.register_game(yukon_kings)
Sol.register_game(yukonic_plaque)
