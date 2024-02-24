-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local layout = require 'base/layout'
local ops    = require 'base/ops'
local piles  = require 'base/piles'
require 'base/common'

local klondike                           = {
    Info       = {
        Name          = "Klondike",
        Type          = "SimplePacker",
        Family        = "Klondike",
        DeckCount     = 1,
        CardDealCount = 1,
        Redeals       = -1
    },
    Stock      = { Initial = piles.initial.face_down(24) },
    Waste      = {},
    Foundation = {
        Size   = 4,
        create = piles.ace_upsuit_top
    },
    Tableau    = {
        Size   = 7,
        create = function(i)
            return {
                Initial = piles.initial.top_face_up(i + 1),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "King" }
            }
        end
    },
    on_redeal  = ops.redeal.waste_to_stock,
    on_deal    = ops.deal.stock_to_waste,
    on_created = layout.klondike
}

------

local klondike_by_3s                     = Copy(klondike)
klondike_by_3s.Info.Name                 = "Klondike by Threes"
klondike_by_3s.Info.CardDealCount        = 3
klondike_by_3s.Waste.Layout              = "Fan"

------

local double_klondike                    = Copy(klondike)
double_klondike.Info.Name                = "Double Klondike"
double_klondike.Info.DeckCount           = 2
double_klondike.Stock.Initial            = piles.initial.face_down(59)
double_klondike.Foundation.Size          = 8
double_klondike.Tableau.Size             = 9
double_klondike.on_created               = layout.big_harp

------

local double_klondike_by_3s              = Copy(double_klondike)
double_klondike_by_3s.Info.Name          = "Double Klondike by Threes"
double_klondike_by_3s.Info.CardDealCount = 3

------

local triple_klondike                    = Copy(klondike)
triple_klondike.Info.Name                = "Triple Klondike"
triple_klondike.Info.DeckCount           = 3
triple_klondike.Stock.Initial            = piles.initial.face_down(65)
triple_klondike.Foundation.Size          = 12
triple_klondike.Tableau.Size             = 13
triple_klondike.on_created               = layout.big_harp

------

local triple_klondike_by_3s              = Copy(triple_klondike)
triple_klondike_by_3s.Info.Name          = "Triple Klondike by Threes"
triple_klondike_by_3s.Info.CardDealCount = 3

------

local half_klondike                      = Copy(klondike)
half_klondike.Info.Name                  = "Half Klondike"
half_klondike.Stock.Initial              = piles.initial.face_down(16)
half_klondike.Foundation.Size            = 2
half_klondike.Tableau.Size               = 4
half_klondike.on_before_shuffle             = function(_, card) return card.Suit == "Clubs" or card.Suit == "Diamonds" end

------

local big_deal                           = {
    Info       = {
        Name = "Big Deal",
        Type = "SimplePacker",
        Family = "Klondike",
        DeckCount = 4,
        CardDealCount = 1,
        Redeals = 1
    },
    Stock      = {
        Position = { x = 0, y = 7 },
        Initial = piles.initial.face_down(130)
    },
    Waste      = { Position = { x = 1, y = 7 } },
    Foundation = {
        Size = 16,
        create = function(i)
            return {
                Position = { x = i % 2 + 12, y = i // 2 },
                Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" }
            }
        end
    },
    Tableau    = {
        Size = 12,
        create = function(i)
            return {
                Position = { x = i, y = 0 },
                Initial  = piles.initial.top_face_up(i + 1),
                Layout   = "Column",
                Rule     = { Build = "DownAlternateColors", Move = "InSequence", Empty = "King" }
            }
        end
    },
    on_redeal  = ops.redeal.waste_to_stock,
    on_deal    = ops.deal.stock_to_waste,
}

------

local big_forty                          = Copy(klondike)
big_forty.Info.Name                      = "Big Forty"
big_forty.Stock.Initial                  = piles.initial.face_down(12)
big_forty.Tableau                        = {
    Size   = 10,
    create = {
        Initial = piles.initial.top_face_up(4),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "InSequence", Empty = "Any" }
    }
}
------

local big_harp                           = Copy(klondike)
big_harp.Info.Name                       = "Big Harp"
big_harp.Info.DeckCount                  = 2
big_harp.Info.Redeals                    = 0
big_harp.Stock.Initial                   = piles.initial.face_down(49)
big_harp.Foundation.Size                 = 8
big_harp.Tableau                         = {
    Size   = 10,
    create = function(i)
        return {
            Initial = piles.initial.top_face_up(10 - i),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
        }
    end
}
big_harp.on_created                      = layout.big_harp

------

local ali_baba                           = Copy(klondike)
ali_baba.Info.Name                       = "Ali Baba"
ali_baba.Stock.Initial                   = piles.initial.face_down(8)
ali_baba.Tableau                         = {
    Size   = 10,
    create = {
        Initial = piles.initial.top_face_up(4),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "InSequence", Empty = "Any" }
    }
}
ali_baba.on_before_shuffle                  = ops.shuffle.ace_to_foundation

------

local blind_alleys                       = Copy(klondike)
blind_alleys.Info.Name                   = "Blind Alleys"
blind_alleys.Info.Redeals                = 1
blind_alleys.Stock.Initial               = piles.initial.face_down(30)
blind_alleys.Tableau                     = {
    Size   = 6,
    create = {
        Initial = piles.initial.top_face_up(3),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
    }
}
blind_alleys.on_before_shuffle              = ops.shuffle.ace_to_foundation

------

local cassim                             = Copy(klondike)
cassim.Info.Name                         = "Cassim"
cassim.Stock.Initial                     = piles.initial.face_down(20)
cassim.Tableau.create                    = {
    Initial = piles.initial.top_face_up(4),
    Layout = "Column",
    Rule = { Build = "DownInSuit", Move = "InSequence", Empty = "Any" }
}
cassim.on_before_shuffle                    = ops.shuffle.ace_to_foundation

------

local eight_by_eight                     = {
    Info       = {
        Name = "8 x 8",
        Type = "SimplePacker",
        Family = "Klondike",
        DeckCount = 2,
        CardDealCount = 1,
        Redeals = -1
    },
    Stock      = { Initial = piles.initial.face_down(40) },
    Waste      = {},
    Foundation = {
        Size = 8,
        create = piles.ace_upsuit_top
    },
    Tableau    = {
        Size = 8,
        create = function(i)
            return {
                Initial = piles.initial.face_up(8),
                Layout  = "Column",
                Rule    = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
            }
        end
    },
    on_redeal  = ops.redeal.waste_to_stock,
    on_deal    = ops.deal.stock_to_waste,
    on_created = layout.big_harp
}

------

local eight_by_eight2                    = Copy(klondike)
eight_by_eight2.Info.Name                = "Eight by Eight"
eight_by_eight2.Info.DeckCount           = 2
eight_by_eight2.Info.CardDealCount       = 3
eight_by_eight2.Info.Redeals             = 2
eight_by_eight2.Stock.Initial            = piles.initial.face_down(40)
eight_by_eight2.Foundation.Size          = 8
eight_by_eight2.Tableau                  = {
    Size   = 8,
    create = function(i)
        return {
            Initial = piles.initial.face_up(8),
            Layout = "Column",
            Rule = { Build = "DownByRank", Move = "InSequence", Empty = "AnySingle" }
        }
    end
}
eight_by_eight2.on_deal                  = ops.deal.stock_to_waste_by_redeals_left

------

local eight_by_eight3                    = Copy(eight_by_eight)
eight_by_eight3.Info.Name                = "Eight Times Eight"
eight_by_eight3.Info.Redeals             = 2
eight_by_eight3.Tableau.create           = {
    Initial = piles.initial.face_up(8),
    Layout = "Column",
    Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
}

------

local arabella                           = Copy(klondike)
arabella.Info.Name                       = "Arabella"
arabella.Info.DeckCount                  = 3
arabella.Stock.Initial                   = piles.initial.face_down(65)
arabella.Foundation.Size                 = 12
arabella.Tableau                         = {
    Size   = 13,
    create = function(i)
        return {
            Initial = piles.initial.top_face_up(i + 1),
            Layout = "Column",
            Rule = { Build = "DownByRank", Move = "InSequenceInSuit", Empty = "King" }
        }
    end
}
arabella.on_created                      = layout.big_harp

------

local athena                             = Copy(klondike)
athena.Info.Name                         = "Athena"
athena.Tableau.create                    = {
    Initial = piles.initial.alternate(4, false),
    Layout = "Column",
    Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
}

------

local aunt_mary                          = Copy(klondike)
aunt_mary.Info.Name                      = "Aunt Mary"
aunt_mary.Info.Redeals                   = 0
aunt_mary.Stock.Initial                  = piles.initial.face_down(16)
aunt_mary.Tableau                        = {
    Size   = 6,
    create = function(i)
        return {
            Initial = { i < 1, i < 2, i < 3, i < 4, i < 5, i < 6 },
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
        }
    end
}

------

local boost                              = Copy(klondike)
boost.Info.Name                          = "Boost"
boost.Info.Redeals                       = 2
boost.Stock.Initial                      = piles.initial.face_down(42)
boost.Tableau.Size                       = 4

------

local chinaman                           = Copy(klondike)
chinaman.Info.Name                       = "Chinaman"
chinaman.Info.CardDealCount              = 3
chinaman.Info.Redeals                    = 2
chinaman.Waste.Layout                    = "Fan"
chinaman.Tableau                         = {
    Size   = 7,
    create = function(i)
        return {
            Initial = piles.initial.top_face_up(i + 1),
            Layout = "Column",
            Rule = { Build = "DownAnyButOwnSuit", Move = "InSequence", Empty = "King" }
        }
    end
}

------

local chinese_klondike                   = Copy(klondike)
chinese_klondike.Info.Name               = "Chinese Klondike"
chinese_klondike.Info.DeckCount          = 3
chinese_klondike.Stock.Initial           = piles.initial.face_down(39)
chinese_klondike.Foundation.Size         = 9
chinese_klondike.Tableau.Size            = 12
chinese_klondike.on_before_shuffle          = function(game, card) return card.Suit == "Diamonds" end
chinese_klondike.on_created              = layout.big_harp

------

local eastcliff                          = Copy(klondike)
eastcliff.Info.Name                      = "Eastcliff"
eastcliff.Info.Redeals                   = 0
eastcliff.Stock.Initial                  = piles.initial.face_down(31)
eastcliff.Tableau                        = {
    Size   = 7,
    create = {
        Initial = piles.initial.top_face_up(3),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
    }
}

------

local eight_sages                        = Copy(klondike)
eight_sages.Info.Name                    = "Eight Sages"
eight_sages.Info.DeckCount               = 2
eight_sages.Info.Redeals                 = 1
eight_sages.Stock.Initial                = piles.initial.face_down(96)
eight_sages.Foundation.Size              = 8
eight_sages.Tableau                      = {
    Size   = 8,
    create = {
        Initial = piles.initial.face_up(1),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "Top", Empty = "Any" }
    }
}
eight_sages.check_drop                   = function(game, targetPile, targetIndex, drop, numCards)
    if targetPile.Type == "Tableau" then
        local srcPile = game:find_pile(drop)
        if srcPile.Type ~= "Waste" then
            return false
        end
    end

    return game:can_drop(targetPile, targetIndex, drop, numCards)
end

------

local gargantua                          = Copy(double_klondike)
gargantua.Info.Name                      = "Gargantua"
gargantua.Info.Redeals                   = 1

------

local guardian                           = {
    Info       = {
        Name = "Guardian",
        Type = "SimplePacker",
        Family = "Klondike",
        DeckCount = 1,
        CardDealCount = 3,
        Redeals = -1
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = piles.initial.face_down(40)
    },
    Waste      = { Position = { x = 1, y = 0 } },
    Foundation = {
        Size = 4,
        create = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" }
            }
        end
    },
    Tableau    = {
        Size = 12,
        create = function(i)
            local tab = {
                HasMarker = i < 3,
                Layout    = "Column",
                Rule      = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
            }
            if i < 7 then
                tab.Initial = piles.initial.face_down(1)
            else
                tab.Initial = piles.initial.face_up(1)
            end
            if i < 3 then
                tab.Position = { x = i + 2, y = 1 }
            elseif (i < 7) then
                tab.Position = { x = i - 1.5, y = 1.5 }
            else
                tab.Position = { x = i - 6, y = 2 }
            end

            return tab
        end
    },
    on_redeal  = ops.redeal.waste_to_stock,
    on_deal    = ops.deal.stock_to_waste,
    on_change  = function(game)
        for tabIdx = 1, 7 do
            local pile = game.Tableau[tabIdx]
            if not pile.IsEmpty then
                local pileL = game.Tableau[tabIdx + 4]
                local pileR = game.Tableau[tabIdx + 5]
                if pileL.IsEmpty and pileR.IsEmpty then
                    pile:flip_up_top_card()
                end
            end
        end
    end
}

------

local gold_rush                          = Copy(klondike)
gold_rush.Info.Name                      = "Gold Rush"
gold_rush.Info.CardDealCount             = 3
gold_rush.Info.Redeals                   = 2
gold_rush.on_deal                        = ops.deal.stock_to_waste_by_redeals_left

------

local kingsley                           = Copy(klondike)
kingsley.Info.Name                       = "Kingsley"
kingsley.Info.Redeals                    = 0
kingsley.Foundation.create               = { Rule = { Build = "DownInSuit", Move = "Top", Empty = "King" } }
kingsley.Tableau.create                  = function(i)
    return {
        Initial = piles.initial.top_face_up(i + 1),
        Layout = "Column",
        Rule = { Build = "UpAlternateColors", Move = "InSequence", Empty = "Ace" }
    }
end

------

local double_kingsley                    = Copy(kingsley)
double_kingsley.Info.Name                = "Double Kingsley"
double_kingsley.Info.DeckCount           = 2
double_kingsley.Stock.Initial            = piles.initial.face_down(59)
double_kingsley.Foundation.Size          = 8
double_kingsley.Tableau.Size             = 9
double_kingsley.on_created               = layout.big_harp

------

local qc                                 = Copy(klondike)
qc.Info.Name                             = "Q.C."
qc.Info.DeckCount                        = 2
qc.Info.Redeals                          = 1
qc.Stock.Initial                         = piles.initial.face_down(80)
qc.Foundation                            = {
    Size   = 8,
    create = { Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" } }
}
qc.Tableau                               = {
    Size   = 6,
    create = {
        Initial = piles.initial.top_face_up(4),
        Layout  = "Column",
        Rule    = { Build = "DownInSuit", Move = "Top", Empty = "Any" }
    }
}
qc.on_change                             = function(game)
    return game.Waste[1]:deal_to_group(game.Tableau, true) or game.Stock[1]:deal_to_group(game.Tableau, true)
end

------

local saratoga                           = Copy(klondike_by_3s)
saratoga.Info.Name                       = "Saratoga"
saratoga.Tableau.create                  = function(i)
    return {
        Initial = piles.initial.face_up(i + 1),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "King" }
    }
end

------

local thirty_six                         = Copy(klondike)
thirty_six.Info.Name                     = "Thirty Six"
thirty_six.Info.Redeals                  = 0
thirty_six.Stock.Initial                 = piles.initial.face_down(16)
thirty_six.Foundation                    = {
    Size = 4,
    create = { Rule = { Build = "UpInColor", Move = "Top", Empty = "Ace" } }
}
thirty_six.Tableau                       = {
    Size = 6,
    create = {
        Initial = piles.initial.face_up(6),
        Layout  = "Column",
        Rule    = { Build = "DownByRank", Move = "InSequence", Empty = "Any" }
    }
}
thirty_six.on_shuffle                    = function(game, card, pileType)
    if pileType == "Tableau" and card.Rank == "Ace" then
        return game.PlaceTop(card, game.Foundation, true)
    end

    return false
end

------

local trigon                             = Copy(klondike)
trigon.Info.Name                         = "Trigon"
trigon.Tableau.create                    = function(i)
    return {
        Initial = piles.initial.top_face_up(i + 1),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "InSequence", Empty = "King" }
    }
end

------

local double_trigon                      = Copy(trigon)
double_trigon.Info.Name                  = "Double Trigon"
double_trigon.Info.DeckCount             = 2
double_trigon.Stock.Initial              = piles.initial.face_down(59)
double_trigon.Foundation.Size            = 8
double_trigon.Tableau.Size               = 9
double_trigon.on_created                 = layout.big_harp

------

local whitehorse                         = Copy(klondike)
whitehorse.Info.Name                     = "Whitehorse"
whitehorse.Info.CardDealCount            = 3
whitehorse.Info.Redeals                  = 0
whitehorse.Stock.Initial                 = piles.initial.face_down(45)
whitehorse.Waste.Layout                  = "Fan"
whitehorse.Foundation                    = {
    Size = 4,
    create = { Rule = { Build = "UpInColor", Move = "Top", Empty = "Ace" } }
}
whitehorse.Tableau                       = {
    Size = 7,
    create = {
        Initial = piles.initial.face_up(1),
        Layout  = "Column",
        Rule    = { Build = "DownAlternateColors", Move = "InSequence", Empty = "King" }
    }
}
whitehorse.on_change                     = function(game)
    return game.Waste[1]:deal_to_group(game.Tableau, true) or game.Stock[1]:deal_to_group(game.Tableau, true)
end

------

local australian_patience                = Copy(klondike)
australian_patience.Info.Name            = "Australian Patience"
--australian_patience.Info.Family = "Klondike/Yukon"
australian_patience.Info.Redeals         = 0
australian_patience.Stock.Initial        = piles.initial.face_down(24)
australian_patience.Tableau              = {
    Size = 7,
    create = {
        Initial = piles.initial.face_up(4),
        Layout  = "Column",
        Rule    = { Build = "DownInSuit", Move = "FaceUp", Empty = "King" }
    }
}

------

local bim_bom                            = Copy(klondike)
bim_bom.Info.Name                        = "Bim Bom"
bim_bom.Info.DeckCount                   = 2
bim_bom.Info.Redeals                     = 0
bim_bom.Stock.Initial                    = piles.initial.face_down(64)
bim_bom.Foundation.Size                  = 8
bim_bom.Tableau                          = {
    Size = 8,
    create = {
        Initial = piles.initial.face_up(5),
        Layout  = "Column",
        Rule    = { Build = "DownInSuit", Move = "FaceUp", Empty = "Any" }
    }
}

------

local somerset                           = {
    Info       = {
        Name          = "Somerset",
        Type          = "OpenPacker",
        Family        = "Klondike",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size   = 4,
        create = piles.ace_upsuit_top
    },
    Tableau    = {
        Size   = 10,
        create = function(i)
            return {
                Initial = piles.initial.face_up(i < 7 and i + 1 or 8),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "Top", Empty = "Any" }
            }
        end
    },
    on_created = layout.canister
}

------
local function usk_redeal(game)
    local cards = ShuffleTableau(game)
    if #cards == 0 then return false end

    local tableau = game.Tableau
    local tabIdx = 1
    while #cards > 0 do
        local targetCount = tabIdx < 8 and tabIdx or 8

        local tab = tableau[tabIdx]
        if targetCount > tab.CardCount then
            game.PlaceTop(table.remove(cards), tab, false)
        end

        tabIdx = tabIdx + 1
        if tabIdx > #tableau then tabIdx = 1 end
    end

    return true
end

local usk = {
    Info       = {
        Name          = "Usk",
        Type          = "OpenPacker",
        Family        = "Klondike",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 1
    },
    Stock      = {},
    Foundation = {
        Size   = 4,
        create = piles.ace_upsuit_top
    },
    Tableau    = {
        Size   = 10,
        create = function(i)
            return {
                Initial = piles.initial.face_up(i < 7 and i + 1 or 8),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "King" }
            }
        end
    },
    on_redeal  = usk_redeal,
    on_created = layout.canister
}

------------------------

RegisterGame(klondike)
RegisterGame(klondike_by_3s)
RegisterGame(double_klondike)
RegisterGame(double_klondike_by_3s)
RegisterGame(triple_klondike)
RegisterGame(triple_klondike_by_3s)
RegisterGame(half_klondike)
RegisterGame(ali_baba)
RegisterGame(arabella)
RegisterGame(athena)
RegisterGame(aunt_mary)
RegisterGame(australian_patience)
RegisterGame(big_deal)
RegisterGame(big_forty)
RegisterGame(big_harp)
RegisterGame(bim_bom)
RegisterGame(blind_alleys)
RegisterGame(boost)
RegisterGame(cassim)
RegisterGame(chinaman)
RegisterGame(chinese_klondike)
RegisterGame(eastcliff)
RegisterGame(eight_by_eight)
RegisterGame(eight_by_eight2)
RegisterGame(eight_by_eight3)
RegisterGame(eight_sages)
RegisterGame(gargantua)
RegisterGame(gold_rush)
RegisterGame(guardian)
RegisterGame(kingsley)
RegisterGame(double_kingsley)
RegisterGame(qc)
RegisterGame(saratoga)
RegisterGame(somerset)
RegisterGame(thirty_six)
RegisterGame(trigon)
RegisterGame(double_trigon)
RegisterGame(usk)
RegisterGame(whitehorse)
