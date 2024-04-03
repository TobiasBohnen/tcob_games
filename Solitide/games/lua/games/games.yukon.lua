-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


ops.Initial.yukon                        = function(i)
    if i == 0 then
        return { true }
    else
        local t = {}
        for j = 1, i + 5 do
            t[j] = j > i
        end
        return t
    end
end
ops.Initial.double_yukon                 = function(i)
    local t = {}
    local ii = math.min(i, 8)
    for j = 1, ii + 6 do
        t[j] = j > ii
    end
    return t
end
ops.Initial.triple_yukon                 = function(i)
    local t = ops.Initial.face_down(i + 6)
    for j = #t - 5, #t do
        t[j] = true
    end
    return t
end
ops.Initial.quadruple_yukon              = function(i)
    if i == 0 then
        return { true }
    elseif i <= 12 then
        local t = ops.Initial.face_down(i + 6)
        for j = #t - 5, #t do
            t[j] = true
        end
        return t
    else
        local t = ops.Initial.face_down(i + 5)
        for j = #t - 4, #t do
            t[j] = true
        end
        return t
    end
end
ops.Initial.chinese_discipline           = function(i)
    if i < 3 then
        return ops.Initial.face_up(7)
    else
        local t = ops.Initial.face_down(7)
        for j = i, 7 do
            t[j] = true
        end
        return t
    end
end

------

local yukon                              = {
    Info             = {
        Name          = "Yukon",
        Family        = "Yukon",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation       = {
        Size = 4,
        Pile = { Rule = rules.ace_upsuit_none }
    },
    Tableau          = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = ops.Initial.yukon(i),
                Layout = "Column",
                Rule = rules.king_downac_faceup
            }
        end
    },
    on_piles_created = Sol.Layout.yukon
}

------

local double_yukon                       = Sol.copy(yukon)
double_yukon.Info.Name                   = "Double Yukon"
double_yukon.Info.DeckCount              = 2
double_yukon.Foundation.Size             = 8
double_yukon.Tableau                     = {
    Size = 10,
    Pile = function(i)
        return {
            Initial = ops.Initial.double_yukon(i),
            Layout = "Column",
            Rule = rules.king_downac_faceup
        }
    end
}

------

local triple_yukon                       = Sol.copy(yukon)
triple_yukon.Info.Name                   = "Triple Yukon"
triple_yukon.Info.DeckCount              = 3
triple_yukon.Foundation.Size             = 12
triple_yukon.Tableau                     = {
    Size = 13,
    Pile = function(i)
        return {
            Initial = ops.Initial.triple_yukon(i),
            Layout = "Column",
            Rule = rules.king_downac_faceup
        }
    end
}

------

local quadruple_yukon                    = Sol.copy(yukon)
quadruple_yukon.Info.Name                = "Quadruple Yukon"
quadruple_yukon.Info.DeckCount           = 4
quadruple_yukon.Foundation.Size          = 16
quadruple_yukon.Tableau                  = {
    Size = 16,
    Pile = function(i)
        return {
            Initial = ops.Initial.quadruple_yukon(i),
            Layout = "Column",
            Rule = rules.king_downac_faceup
        }
    end
}

------

local alaska                             = Sol.copy(yukon)
alaska.Info.Name                         = "Alaska"
alaska.Tableau.Pile                      = function(i)
    return {
        Initial = ops.Initial.yukon(i),
        Layout = "Column",
        Rule = { Base = rules.Base.King(), Build = rules.Build.UpOrDownInSuit(), Move = rules.Move.FaceUp() }
    }
end

------

local brisbane_initial                   = {
    { true,  true,  true,  true,  true },
    { false, true,  true,  true,  true,  true },
    { false, false, true,  true,  true,  true,  true },
    { false, false, false, true,  true,  true,  true },
    { false, false, false, false, true,  true,  true, true },
    { false, false, false, false, false, true,  true, true, true },
    { false, false, false, false, false, false, true, true, true, true }
}

local brisbane                           = Sol.copy(yukon)
brisbane.Info.Name                       = "Brisbane"
brisbane.Tableau.Pile                    = function(i)
    return {
        Initial = brisbane_initial[i + 1],
        Layout = "Column",
        Rule = { Base = rules.Base.King(), Build = rules.Build.DownByRank(), Move = rules.Move.FaceUp() }
    }
end

------

local geoffrey                           = Sol.copy(yukon)
geoffrey.Info.Name                       = "Geoffrey"
geoffrey.Tableau                         = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = i < 4 and ops.Initial.face_up(7) or { false, false, false, false, true, true },
            Layout = "Column",
            Rule = rules.king_downsuit_faceup
        }
    end
}
geoffrey.on_piles_created                = Sol.Layout.forty_thieves

------

local queensland                         = Sol.copy(yukon)
queensland.Info.Name                     = "Queensland"
queensland.Tableau.Pile                  = function(i)
    return {
        Initial = brisbane_initial[i + 1],
        Layout = "Column",
        Rule = rules.any_downsuit_faceup
    }
end

------

local roslin                             = Sol.copy(yukon)
roslin.Info.Name                         = "Roslin"
roslin.Tableau.Pile                      = function(i)
    return {
        Initial = ops.Initial.yukon(i),
        Layout = "Column",
        Rule = { Base = rules.Base.King(), Build = rules.Build.UpOrDownAlternateColors(), Move = rules.Move.FaceUp() }
    }
end

------

local moosehide                          = Sol.copy(yukon)
moosehide.Info.Name                      = "Moosehide"
moosehide.Tableau.Pile                   = function(i)
    return {
        Initial = ops.Initial.yukon(i),
        Layout = "Column",
        Rule = { Base = rules.Base.King(), Build = rules.Build.DownAnyButOwnSuit(), Move = rules.Move.FaceUp() }
    }
end

------

local russian_solitaire                  = Sol.copy(yukon)
russian_solitaire.Info.Name              = "Russian Solitaire"
russian_solitaire.Tableau.Pile           = function(i)
    return {
        Initial = ops.Initial.yukon(i),
        Layout = "Column",
        Rule = rules.king_downsuit_faceup
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
            Initial = ops.Initial.double_yukon(i),
            Layout = "Column",
            Rule = rules.king_downsuit_faceup
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
            Initial = ops.Initial.triple_yukon(i),
            Layout = "Column",
            Rule = rules.king_downsuit_faceup
        }
    end
}

------

local hawaiian                           = {
    Info       = {
        Name          = "Hawaiian",
        Family        = "Yukon",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 0
    },
    Reserve    = {
        Position = { x = 0, y = 0 },
        Initial  = ops.Initial.face_up(54),
        Layout   = "Squared"
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Rule = rules.ace_upsuit_none
            }
        end
    },
    Tableau    = {
        Size = 10,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial = ops.Initial.face_up(5),
                Layout = "Column",
                Rule = rules.any_downac_faceup
            }
        end
    }
}

------

local chinese_discipline                 = {
    Info             = {
        Name          = "Chinese Discipline",
        Family        = "Yukon",
        DeckCount     = 1,
        CardDealCount = 3,
        Redeals       = 0
    },
    Stock            = {
        Initial = ops.Initial.face_down(3)
    },
    Foundation       = {
        Size = 4,
        Pile = { Rule = rules.ace_upsuit_none }
    },
    Tableau          = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = ops.Initial.chinese_discipline(i),
                Layout = "Column",
                Rule = rules.king_downac_faceup
            }
        end
    },
    on_deal          = function(game) return ops.Deal.to_group(game.Stock[1], game.Tableau, false) end,
    on_piles_created = Sol.Layout.klondike
}

------

local chinese_solitaire                  = Sol.copy(chinese_discipline)
chinese_solitaire.Info.Name              = "Chinese Solitaire"
chinese_solitaire.Tableau.Pile           = function(i)
    return {
        Initial = ops.Initial.chinese_discipline(i),
        Layout = "Column",
        Rule = rules.any_downac_faceup
    }
end

------

local rushdike                           = {
    Info             = {
        Name          = "Rushdike",
        Family        = "Yukon",
        --Family = "Gypsy/Yukon/Klondike"
        DeckCount     = 1,
        CardDealCount = 7,
        Redeals       = 0
    },
    Stock            = {
        Initial = ops.Initial.face_down(24)
    },
    Foundation       = {
        Size = 4,
        Pile = { Rule = rules.ace_upsuit_none }
    },
    Tableau          = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = ops.Initial.top_face_up(i + 1),
                Layout = "Column",
                Rule = rules.king_downsuit_faceup
            }
        end
    },
    on_deal          = function(game) return ops.Deal.to_group(game.Stock[1], game.Tableau, false) end,
    on_piles_created = Sol.Layout.klondike
}

------

local queenie                            = {
    Info             = {
        Name          = "Queenie",
        Family        = "Yukon",
        --queenie.Info.Family = "Yukon/Gypsy"
        DeckCount     = 1,
        CardDealCount = 7,
        Redeals       = 0
    },
    Stock            = {
        Initial = ops.Initial.face_down(24)
    },
    Foundation       = {
        Size = 4,
        Pile = { Rule = rules.ace_upsuit_none }
    },
    Tableau          = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = ops.Initial.face_up(i + 1),
                Layout = "Column",
                Rule = rules.king_downac_faceup
            }
        end
    },
    on_deal          = function(game) return ops.Deal.to_group(game.Stock[1], game.Tableau, false) end,
    on_piles_created = Sol.Layout.klondike
}

------

------------------------

Sol.register_game(yukon)
Sol.register_game(alaska)
Sol.register_game(brisbane)
Sol.register_game(chinese_discipline)
Sol.register_game(chinese_solitaire)
Sol.register_game(double_russian_solitaire)
Sol.register_game(double_yukon)
Sol.register_game(geoffrey)
Sol.register_game(hawaiian)
Sol.register_game(moosehide)
Sol.register_game(quadruple_yukon)
Sol.register_game(queenie)
Sol.register_game(queensland)
Sol.register_game(roslin)
Sol.register_game(russian_solitaire)
Sol.register_game(triple_russian_solitaire)
Sol.register_game(triple_yukon)
Sol.register_game(rushdike)
