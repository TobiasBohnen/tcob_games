-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local layout = require 'base/layout'
local ops    = require 'base/ops'
local piles  = require 'base/piles'
require 'base/common'

piles.initial.yukon                      = function(i)
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
piles.initial.double_yukon               = function(i)
    local t = {}
    local ii = math.min(i, 8)
    for j = 1, ii + 6 do
        t[j] = j > ii
    end
    return t
end
piles.initial.triple_yukon               = function(i)
    local t = piles.initial.face_down(i + 6)
    for j = #t - 5, #t do
        t[j] = true
    end
    return t
end
piles.initial.chinese_discipline         = function(i)
    if i < 3 then
        return piles.initial.face_up(7)
    else
        local t = piles.initial.face_down(7)
        for j = i, 7 do
            t[j] = true
        end
        return t
    end
end

------

local yukon                              = {
    Info       = {
        Name          = "Yukon",
        Type          = "Packer",
        Family        = "Yukon",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size   = 4,
        create = { Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" } }
    },
    Tableau    = {
        Size   = 7,
        create = function(i)
            return {
                Initial = piles.initial.yukon(i),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "FaceUp", Empty = "King" }
            }
        end
    },
    on_created = layout.yukon
}

------

local double_yukon                       = Copy(yukon)
double_yukon.Info.Name                   = "Double Yukon"
double_yukon.Info.DeckCount              = 2
double_yukon.Foundation.Size             = 8
double_yukon.Tableau                     = {
    Size   = 10,
    create = function(i)
        return {
            Initial = piles.initial.double_yukon(i),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "FaceUp", Empty = "King" }
        }
    end
}

------

local triple_yukon                       = Copy(yukon)
triple_yukon.Info.Name                   = "Triple Yukon"
triple_yukon.Info.DeckCount              = 3
triple_yukon.Foundation.Size             = 12
triple_yukon.Tableau                     = {
    Size   = 13,
    create = function(i)
        return {
            Initial = piles.initial.triple_yukon(i),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "FaceUp", Empty = "King" }
        }
    end
}

------

local alaska                             = Copy(yukon)
alaska.Info.Name                         = "Alaska"
alaska.Tableau.create                    = function(i)
    return {
        Initial = piles.initial.yukon(i),
        Layout = "Column",
        Rule = { Build = "UpOrDownInSuit", Move = "FaceUp", Empty = "King" }
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

local brisbane                           = Copy(yukon)
brisbane.Info.Name                       = "Brisbane"
brisbane.Tableau.create                  = function(i)
    return {
        Initial = brisbane_initial[i + 1],
        Layout = "Column",
        Rule = { Build = "DownByRank", Move = "FaceUp", Empty = "King" }
    }
end

------

local geoffrey                           = Copy(yukon)
geoffrey.Info.Name                       = "Geoffrey"
geoffrey.Tableau                         = {
    Size   = 8,
    create = function(i)
        return {
            Initial = i < 4 and piles.initial.face_up(7) or { false, false, false, false, true, true },
            Layout = "Column",
            Rule = { Build = "DownInSuit", Move = "FaceUp", Empty = "King" }
        }
    end
}
geoffrey.on_created                      = layout.forty_thieves

------

local queensland                         = Copy(yukon)
queensland.Info.Name                     = "Queensland"
queensland.Tableau.create                = function(i)
    return {
        Initial = brisbane_initial[i + 1],
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "FaceUp", Empty = "Any" }
    }
end

------

local roslin                             = Copy(yukon)
roslin.Info.Name                         = "Roslin"
roslin.Tableau.create                    = function(i)
    return {
        Initial = piles.initial.yukon(i),
        Layout = "Column",
        Rule = { Build = "UpOrDownAlternateColors", Move = "FaceUp", Empty = "King" }
    }
end

------

local moosehide                          = Copy(yukon)
moosehide.Info.Name                      = "Moosehide"
moosehide.Tableau.create                 = function(i)
    return {
        Initial = piles.initial.yukon(i),
        Layout = "Column",
        Rule = { Build = "DownAnyButOwnSuit", Move = "FaceUp", Empty = "King" }
    }
end

------

local russian_solitaire                  = Copy(yukon)
russian_solitaire.Info.Name              = "Russian Solitaire"
russian_solitaire.Tableau.create         = function(i)
    return {
        Initial = piles.initial.yukon(i),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "FaceUp", Empty = "King" }
    }
end

------

local double_russian_solitaire           = Copy(russian_solitaire)
double_russian_solitaire.Info.Name       = "Double Russian Solitaire"
double_russian_solitaire.Info.DeckCount  = 2
double_russian_solitaire.Foundation.Size = 8
double_russian_solitaire.Tableau         = {
    Size   = 10,
    create = function(i)
        return {
            Initial = piles.initial.double_yukon(i),
            Layout = "Column",
            Rule = { Build = "DownInSuit", Move = "FaceUp", Empty = "King" }
        }
    end
}

------

local triple_russian_solitaire           = Copy(russian_solitaire)
triple_russian_solitaire.Info.Name       = "Triple Russian Solitaire"
triple_russian_solitaire.Info.DeckCount  = 3
triple_russian_solitaire.Foundation.Size = 12
triple_russian_solitaire.Tableau         = {
    Size   = 13,
    create = function(i)
        return {
            Initial = piles.initial.triple_yukon(i),
            Layout = "Column",
            Rule = { Build = "DownInSuit", Move = "FaceUp", Empty = "King" }
        }
    end
}

------

local hawaiian                           = {
    Info       = {
        Name          = "Hawaiian",
        Type          = "Packer",
        Family        = "Yukon",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 0
    },
    Reserve    = {
        Position = { x = 0, y = 0 },
        Initial  = piles.initial.face_up(54),
        Layout   = "Squared"
    },
    Foundation = {
        Size   = 8,
        create = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" }
            }
        end
    },
    Tableau    = {
        Size   = 10,
        create = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial = piles.initial.face_up(5),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "FaceUp", Empty = "Any" }
            }
        end
    }
}

------

local chinese_discipline                 = {
    Info       = {
        Name          = "Chinese Discipline",
        Type          = "Packer",
        Family        = "Yukon",
        DeckCount     = 1,
        CardDealCount = 3,
        Redeals       = 0
    },
    Stock      = {
        Initial = piles.initial.face_down(3)
    },
    Foundation = {
        Size   = 4,
        create = { Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" } }
    },
    Tableau    = {
        Size   = 7,
        create = function(i)
            return {
                Initial = piles.initial.chinese_discipline(i),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "FaceUp", Empty = "King" }
            }
        end
    },
    on_deal    = function(game) return game.Stock[1]:deal_to_group(game.Tableau, false) end,
    on_created = layout.klondike
}

------

local chinese_solitaire                  = Copy(chinese_discipline)
chinese_solitaire.Info.Name              = "Chinese Solitaire"
chinese_solitaire.Tableau.create         = function(i)
    return {
        Initial = piles.initial.chinese_discipline(i),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "FaceUp", Empty = "Any" }
    }
end

------

local rushdike                           = {
    Info       = {
        Name          = "Rushdike",
        Type          = "Packer",
        Family        = "Yukon",
        --Family = "Gypsy/Yukon/Klondike"
        DeckCount     = 1,
        CardDealCount = 7,
        Redeals       = 0
    },
    Stock      = {
        Initial = piles.initial.face_down(24)
    },
    Foundation = {
        Size   = 4,
        create = { Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" } }
    },
    Tableau    = {
        Size   = 7,
        create = function(i)
            return {
                Initial = piles.initial.top_face_up(i + 1),
                Layout = "Column",
                Rule = { Build = "DownInSuit", Move = "FaceUp", Empty = "King" }
            }
        end
    },
    on_deal    = function(game) return game.Stock[1]:deal_to_group(game.Tableau, false) end,
    on_created = layout.klondike
}

------

------------------------

RegisterGame(yukon)
RegisterGame(double_yukon)
RegisterGame(triple_yukon)
RegisterGame(alaska)
RegisterGame(brisbane)
RegisterGame(chinese_discipline)
RegisterGame(chinese_solitaire)
RegisterGame(geoffrey)
RegisterGame(hawaiian)
RegisterGame(moosehide)
RegisterGame(queensland)
RegisterGame(roslin)
RegisterGame(russian_solitaire)
RegisterGame(double_russian_solitaire)
RegisterGame(triple_russian_solitaire)
RegisterGame(rushdike)
