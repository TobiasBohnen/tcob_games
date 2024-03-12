-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local piles = require 'base/piles'
local rules = require 'base/rules'


piles.Initial.yukon                      = function(i)
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
piles.Initial.double_yukon               = function(i)
    local t = {}
    local ii = math.min(i, 8)
    for j = 1, ii + 6 do
        t[j] = j > ii
    end
    return t
end
piles.Initial.triple_yukon               = function(i)
    local t = piles.Initial.face_down(i + 6)
    for j = #t - 5, #t do
        t[j] = true
    end
    return t
end
piles.Initial.chinese_discipline         = function(i)
    if i < 3 then
        return piles.Initial.face_up(7)
    else
        local t = piles.Initial.face_down(7)
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
        Create = { Rule = { Build = rules.Build.UpInSuit, Move = rules.Move.None, Empty = rules.Empty.Ace } }
    },
    Tableau    = {
        Size   = 7,
        Create = function(i)
            return {
                Initial = piles.Initial.yukon(i),
                Layout = "Column",
                Rule = { Build = rules.Build.DownAlternateColors, Move = rules.Move.FaceUp, Empty = rules.Empty.King }
            }
        end
    },
    on_created = Sol.Layout.yukon
}

------

local double_yukon                       = Sol.copy(yukon)
double_yukon.Info.Name                   = "Double Yukon"
double_yukon.Info.DeckCount              = 2
double_yukon.Foundation.Size             = 8
double_yukon.Tableau                     = {
    Size   = 10,
    Create = function(i)
        return {
            Initial = piles.Initial.double_yukon(i),
            Layout = "Column",
            Rule = { Build = rules.Build.DownAlternateColors, Move = rules.Move.FaceUp, Empty = rules.Empty.King }
        }
    end
}

------

local triple_yukon                       = Sol.copy(yukon)
triple_yukon.Info.Name                   = "Triple Yukon"
triple_yukon.Info.DeckCount              = 3
triple_yukon.Foundation.Size             = 12
triple_yukon.Tableau                     = {
    Size   = 13,
    Create = function(i)
        return {
            Initial = piles.Initial.triple_yukon(i),
            Layout = "Column",
            Rule = { Build = rules.Build.DownAlternateColors, Move = rules.Move.FaceUp, Empty = rules.Empty.King }
        }
    end
}

------

local alaska                             = Sol.copy(yukon)
alaska.Info.Name                         = "Alaska"
alaska.Tableau.create                    = function(i)
    return {
        Initial = piles.Initial.yukon(i),
        Layout = "Column",
        Rule = { Build = rules.Build.UpOrDownInSuit, Move = rules.Move.FaceUp, Empty = rules.Empty.King }
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
brisbane.Tableau.create                  = function(i)
    return {
        Initial = brisbane_initial[i + 1],
        Layout = "Column",
        Rule = { Build = rules.Build.DownByRank, Move = rules.Move.FaceUp, Empty = rules.Empty.King }
    }
end

------

local geoffrey                           = Sol.copy(yukon)
geoffrey.Info.Name                       = "Geoffrey"
geoffrey.Tableau                         = {
    Size   = 8,
    Create = function(i)
        return {
            Initial = i < 4 and piles.Initial.face_up(7) or { false, false, false, false, true, true },
            Layout = "Column",
            Rule = { Build = rules.Build.DownInSuit, Move = rules.Move.FaceUp, Empty = rules.Empty.King }
        }
    end
}
geoffrey.on_created                      = Sol.Layout.forty_thieves

------

local queensland                         = Sol.copy(yukon)
queensland.Info.Name                     = "Queensland"
queensland.Tableau.create                = function(i)
    return {
        Initial = brisbane_initial[i + 1],
        Layout = "Column",
        Rule = { Build = rules.Build.DownInSuit, Move = rules.Move.FaceUp, Empty = rules.Empty.Any }
    }
end

------

local roslin                             = Sol.copy(yukon)
roslin.Info.Name                         = "Roslin"
roslin.Tableau.create                    = function(i)
    return {
        Initial = piles.Initial.yukon(i),
        Layout = "Column",
        Rule = { Build = rules.Build.UpOrDownAlternateColors, Move = rules.Move.FaceUp, Empty = rules.Empty.King }
    }
end

------

local moosehide                          = Sol.copy(yukon)
moosehide.Info.Name                      = "Moosehide"
moosehide.Tableau.create                 = function(i)
    return {
        Initial = piles.Initial.yukon(i),
        Layout = "Column",
        Rule = { Build = rules.Build.DownAnyButOwnSuit, Move = rules.Move.FaceUp, Empty = rules.Empty.King }
    }
end

------

local russian_solitaire                  = Sol.copy(yukon)
russian_solitaire.Info.Name              = "Russian Solitaire"
russian_solitaire.Tableau.create         = function(i)
    return {
        Initial = piles.Initial.yukon(i),
        Layout = "Column",
        Rule = { Build = rules.Build.DownInSuit, Move = rules.Move.FaceUp, Empty = rules.Empty.King }
    }
end

------

local double_russian_solitaire           = Sol.copy(russian_solitaire)
double_russian_solitaire.Info.Name       = "Double Russian Solitaire"
double_russian_solitaire.Info.DeckCount  = 2
double_russian_solitaire.Foundation.Size = 8
double_russian_solitaire.Tableau         = {
    Size   = 10,
    Create = function(i)
        return {
            Initial = piles.Initial.double_yukon(i),
            Layout = "Column",
            Rule = { Build = rules.Build.DownInSuit, Move = rules.Move.FaceUp, Empty = rules.Empty.King }
        }
    end
}

------

local triple_russian_solitaire           = Sol.copy(russian_solitaire)
triple_russian_solitaire.Info.Name       = "Triple Russian Solitaire"
triple_russian_solitaire.Info.DeckCount  = 3
triple_russian_solitaire.Foundation.Size = 12
triple_russian_solitaire.Tableau         = {
    Size   = 13,
    Create = function(i)
        return {
            Initial = piles.Initial.triple_yukon(i),
            Layout = "Column",
            Rule = { Build = rules.Build.DownInSuit, Move = rules.Move.FaceUp, Empty = rules.Empty.King }
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
        Initial  = piles.Initial.face_up(54),
        Layout   = "Squared"
    },
    Foundation = {
        Size   = 8,
        Create = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Rule = { Build = rules.Build.UpInSuit, Move = rules.Move.None, Empty = rules.Empty.Ace }
            }
        end
    },
    Tableau    = {
        Size   = 10,
        Create = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial = piles.Initial.face_up(5),
                Layout = "Column",
                Rule = { Build = rules.Build.DownAlternateColors, Move = rules.Move.FaceUp, Empty = rules.Empty.Any }
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
        Initial = piles.Initial.face_down(3)
    },
    Foundation = {
        Size   = 4,
        Create = { Rule = { Build = rules.Build.UpInSuit, Move = rules.Move.None, Empty = rules.Empty.Ace } }
    },
    Tableau    = {
        Size   = 7,
        Create = function(i)
            return {
                Initial = piles.Initial.chinese_discipline(i),
                Layout = "Column",
                Rule = { Build = rules.Build.DownAlternateColors, Move = rules.Move.FaceUp, Empty = rules.Empty.King }
            }
        end
    },
    on_deal    = function(game) return game.Stock[1]:deal_to_group(game.Tableau, false) end,
    on_created = Sol.Layout.klondike
}

------

local chinese_solitaire                  = Sol.copy(chinese_discipline)
chinese_solitaire.Info.Name              = "Chinese Solitaire"
chinese_solitaire.Tableau.create         = function(i)
    return {
        Initial = piles.Initial.chinese_discipline(i),
        Layout = "Column",
        Rule = { Build = rules.Build.DownAlternateColors, Move = rules.Move.FaceUp, Empty = rules.Empty.Any }
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
        Initial = piles.Initial.face_down(24)
    },
    Foundation = {
        Size   = 4,
        Create = { Rule = { Build = rules.Build.UpInSuit, Move = rules.Move.None, Empty = rules.Empty.Ace } }
    },
    Tableau    = {
        Size   = 7,
        Create = function(i)
            return {
                Initial = piles.Initial.top_face_up(i + 1),
                Layout = "Column",
                Rule = { Build = rules.Build.DownInSuit, Move = rules.Move.FaceUp, Empty = rules.Empty.King }
            }
        end
    },
    on_deal    = function(game) return game.Stock[1]:deal_to_group(game.Tableau, false) end,
    on_created = Sol.Layout.klondike
}

------

local queenie                            = {
    Info       = {
        Name          = "Queenie",
        Type          = "Packer",
        Family        = "Yukon",
        --queenie.Info.Family = "Yukon/Gypsy"
        DeckCount     = 1,
        CardDealCount = 7,
        Redeals       = 0
    },
    Stock      = {
        Initial = piles.Initial.face_down(24)
    },
    Foundation = {
        Size   = 4,
        Create = { Rule = { Build = rules.Build.UpInSuit, Move = rules.Move.None, Empty = rules.Empty.Ace } }
    },
    Tableau    = {
        Size   = 7,
        Create = function(i)
            return {
                Initial = piles.Initial.face_up(i + 1),
                Layout = "Column",
                Rule = { Build = rules.Build.DownAlternateColors, Move = rules.Move.FaceUp, Empty = rules.Empty.King }
            }
        end
    },
    on_deal    = function(game) return game.Stock[1]:deal_to_group(game.Tableau, false) end,
    on_created = Sol.Layout.klondike
}

------

------------------------

Sol.register_game(yukon)
Sol.register_game(double_yukon)
Sol.register_game(triple_yukon)
Sol.register_game(alaska)
Sol.register_game(brisbane)
Sol.register_game(chinese_discipline)
Sol.register_game(chinese_solitaire)
Sol.register_game(geoffrey)
Sol.register_game(hawaiian)
Sol.register_game(moosehide)
Sol.register_game(queenie)
Sol.register_game(queensland)
Sol.register_game(roslin)
Sol.register_game(russian_solitaire)
Sol.register_game(double_russian_solitaire)
Sol.register_game(triple_russian_solitaire)
Sol.register_game(rushdike)
