-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local piles = require 'base/piles'


local klondike                           = {
    Info       = {
        Name          = "Klondike",
        Type          = "SimplePacker",
        Family        = "Klondike",
        DeckCount     = 1,
        CardDealCount = 1,
        Redeals       = -1
    },
    Stock      = { Initial = piles.Initial.face_down(24) },
    Waste      = {},
    Foundation = {
        Size   = 4,
        create = piles.ace_upsuit_top
    },
    Tableau    = {
        Size   = 7,
        create = function(i)
            return {
                Initial = piles.Initial.top_face_up(i + 1),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "King" }
            }
        end
    },
    on_redeal  = ops.Redeal.waste_to_stock,
    on_deal    = ops.Deal.stock_to_waste,
    on_created = Sol.Layout.klondike
}

------

local klondike_by_3s                     = Sol.copy(klondike)
klondike_by_3s.Info.Name                 = "Klondike by Threes"
klondike_by_3s.Info.CardDealCount        = 3
klondike_by_3s.Waste.Layout              = "Fan"

------

local double_klondike                    = Sol.copy(klondike)
double_klondike.Info.Name                = "Double Klondike"
double_klondike.Info.DeckCount           = 2
double_klondike.Stock.Initial            = piles.Initial.face_down(59)
double_klondike.Foundation.Size          = 8
double_klondike.Tableau.Size             = 9
double_klondike.on_created               = Sol.Layout.big_harp

------

local double_klondike_by_3s              = Sol.copy(double_klondike)
double_klondike_by_3s.Info.Name          = "Double Klondike by Threes"
double_klondike_by_3s.Info.CardDealCount = 3

------

local triple_klondike                    = Sol.copy(klondike)
triple_klondike.Info.Name                = "Triple Klondike"
triple_klondike.Info.DeckCount           = 3
triple_klondike.Stock.Initial            = piles.Initial.face_down(65)
triple_klondike.Foundation.Size          = 12
triple_klondike.Tableau.Size             = 13
triple_klondike.on_created               = Sol.Layout.big_harp

------

local triple_klondike_by_3s              = Sol.copy(triple_klondike)
triple_klondike_by_3s.Info.Name          = "Triple Klondike by Threes"
triple_klondike_by_3s.Info.CardDealCount = 3

------

local half_klondike                      = Sol.copy(klondike)
half_klondike.Info.Name                  = "Half Klondike"
half_klondike.Stock.Initial              = piles.Initial.face_down(16)
half_klondike.Foundation.Size            = 2
half_klondike.Tableau.Size               = 4
half_klondike.on_before_shuffle          = function(_, card) return card.Suit == "Clubs" or card.Suit == "Diamonds" end

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
        Initial = piles.Initial.face_down(130)
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
                Initial  = piles.Initial.top_face_up(i + 1),
                Layout   = "Column",
                Rule     = { Build = "DownAlternateColors", Move = "InSequence", Empty = "King" }
            }
        end
    },
    on_redeal  = ops.Redeal.waste_to_stock,
    on_deal    = ops.Deal.stock_to_waste,
}

------

local big_forty                          = Sol.copy(klondike)
big_forty.Info.Name                      = "Big Forty"
big_forty.Stock.Initial                  = piles.Initial.face_down(12)
big_forty.Tableau                        = {
    Size   = 10,
    create = {
        Initial = piles.Initial.top_face_up(4),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "InSequence", Empty = "Any" }
    }
}

------

local big_harp                           = Sol.copy(klondike)
big_harp.Info.Name                       = "Big Harp"
big_harp.Info.DeckCount                  = 2
big_harp.Info.Redeals                    = 0
big_harp.Stock.Initial                   = piles.Initial.face_down(49)
big_harp.Foundation.Size                 = 8
big_harp.Tableau                         = {
    Size   = 10,
    create = function(i)
        return {
            Initial = piles.Initial.top_face_up(10 - i),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
        }
    end
}
big_harp.on_created                      = Sol.Layout.big_harp

------

local ali_baba                           = Sol.copy(klondike)
ali_baba.Info.Name                       = "Ali Baba"
ali_baba.Stock.Initial                   = piles.Initial.face_down(8)
ali_baba.Tableau                         = {
    Size   = 10,
    create = {
        Initial = piles.Initial.top_face_up(4),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "InSequence", Empty = "Any" }
    }
}
ali_baba.on_before_shuffle               = ops.Shuffle.ace_to_foundation

------

local blind_alleys                       = Sol.copy(klondike)
blind_alleys.Info.Name                   = "Blind Alleys"
blind_alleys.Info.Redeals                = 1
blind_alleys.Stock.Initial               = piles.Initial.face_down(30)
blind_alleys.Tableau                     = {
    Size   = 6,
    create = {
        Initial = piles.Initial.top_face_up(3),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
    }
}
blind_alleys.on_before_shuffle           = ops.Shuffle.ace_to_foundation

------

local cassim                             = Sol.copy(klondike)
cassim.Info.Name                         = "Cassim"
cassim.Stock.Initial                     = piles.Initial.face_down(20)
cassim.Tableau.create                    = {
    Initial = piles.Initial.top_face_up(4),
    Layout = "Column",
    Rule = { Build = "DownInSuit", Move = "InSequence", Empty = "Any" }
}
cassim.on_before_shuffle                 = ops.Shuffle.ace_to_foundation

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
    Stock      = { Initial = piles.Initial.face_down(40) },
    Waste      = {},
    Foundation = {
        Size = 8,
        create = piles.ace_upsuit_top
    },
    Tableau    = {
        Size = 8,
        create = function(i)
            return {
                Initial = piles.Initial.face_up(8),
                Layout  = "Column",
                Rule    = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
            }
        end
    },
    on_redeal  = ops.Redeal.waste_to_stock,
    on_deal    = ops.Deal.stock_to_waste,
    on_created = Sol.Layout.big_harp
}

------

local eight_by_eight2                    = Sol.copy(klondike)
eight_by_eight2.Info.Name                = "Eight by Eight"
eight_by_eight2.Info.DeckCount           = 2
eight_by_eight2.Info.CardDealCount       = 3
eight_by_eight2.Info.Redeals             = 2
eight_by_eight2.Stock.Initial            = piles.Initial.face_down(40)
eight_by_eight2.Foundation.Size          = 8
eight_by_eight2.Tableau                  = {
    Size   = 8,
    create = function(i)
        return {
            Initial = piles.Initial.face_up(8),
            Layout = "Column",
            Rule = { Build = "DownByRank", Move = "InSequence", Empty = "AnySingle" }
        }
    end
}
eight_by_eight2.on_deal                  = ops.Deal.stock_to_waste_by_redeals_left

------

local eight_by_eight3                    = Sol.copy(eight_by_eight)
eight_by_eight3.Info.Name                = "Eight Times Eight"
eight_by_eight3.Info.Redeals             = 2
eight_by_eight3.Tableau.create           = {
    Initial = piles.Initial.face_up(8),
    Layout = "Column",
    Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
}

------

local arabella                           = Sol.copy(klondike)
arabella.Info.Name                       = "Arabella"
arabella.Info.DeckCount                  = 3
arabella.Stock.Initial                   = piles.Initial.face_down(65)
arabella.Foundation.Size                 = 12
arabella.Tableau                         = {
    Size   = 13,
    create = function(i)
        return {
            Initial = piles.Initial.top_face_up(i + 1),
            Layout = "Column",
            Rule = { Build = "DownByRank", Move = "InSequenceInSuit", Empty = "King" }
        }
    end
}
arabella.on_created                      = Sol.Layout.big_harp

------

local athena                             = Sol.copy(klondike)
athena.Info.Name                         = "Athena"
athena.Tableau.create                    = {
    Initial = piles.Initial.alternate(4, false),
    Layout = "Column",
    Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
}

------

local aunt_mary                          = Sol.copy(klondike)
aunt_mary.Info.Name                      = "Aunt Mary"
aunt_mary.Info.Redeals                   = 0
aunt_mary.Stock.Initial                  = piles.Initial.face_down(16)
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

local boost                              = Sol.copy(klondike)
boost.Info.Name                          = "Boost"
boost.Info.Redeals                       = 2
boost.Stock.Initial                      = piles.Initial.face_down(42)
boost.Tableau.Size                       = 4

------

local chinaman                           = Sol.copy(klondike)
chinaman.Info.Name                       = "Chinaman"
chinaman.Info.CardDealCount              = 3
chinaman.Info.Redeals                    = 2
chinaman.Waste.Layout                    = "Fan"
chinaman.Tableau                         = {
    Size   = 7,
    create = function(i)
        return {
            Initial = piles.Initial.top_face_up(i + 1),
            Layout = "Column",
            Rule = { Build = "DownAnyButOwnSuit", Move = "InSequence", Empty = "King" }
        }
    end
}

------

local chinese_klondike                   = Sol.copy(klondike)
chinese_klondike.Info.Name               = "Chinese Klondike"
chinese_klondike.Info.DeckCount          = 3
chinese_klondike.Stock.Initial           = piles.Initial.face_down(39)
chinese_klondike.Foundation.Size         = 9
chinese_klondike.Tableau.Size            = 12
chinese_klondike.on_before_shuffle       = function(game, card) return card.Suit == "Diamonds" end
chinese_klondike.on_created              = Sol.Layout.big_harp

------

local eastcliff                          = Sol.copy(klondike)
eastcliff.Info.Name                      = "Eastcliff"
eastcliff.Info.Redeals                   = 0
eastcliff.Stock.Initial                  = piles.Initial.face_down(31)
eastcliff.Tableau                        = {
    Size   = 7,
    create = {
        Initial = piles.Initial.top_face_up(3),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
    }
}

------

local eight_sages                        = Sol.copy(klondike)
eight_sages.Info.Name                    = "Eight Sages"
eight_sages.Info.DeckCount               = 2
eight_sages.Info.Redeals                 = 1
eight_sages.Stock.Initial                = piles.Initial.face_down(96)
eight_sages.Foundation.Size              = 8
eight_sages.Tableau                      = {
    Size   = 8,
    create = {
        Initial = piles.Initial.face_up(1),
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

local gargantua                          = Sol.copy(double_klondike)
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
        Initial = piles.Initial.face_down(40)
    },
    Waste      = {
        Position = { x = 1, y = 0 },
        Layout = "Fan"
    },
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
                tab.Initial = piles.Initial.face_down(1)
            else
                tab.Initial = piles.Initial.face_up(1)
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
    on_redeal  = ops.Redeal.waste_to_stock,
    on_deal    = ops.Deal.stock_to_waste,
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

local gold_rush                          = Sol.copy(klondike)
gold_rush.Info.Name                      = "Gold Rush"
gold_rush.Info.CardDealCount             = 3
gold_rush.Info.Redeals                   = 2
gold_rush.on_deal                        = ops.Deal.stock_to_waste_by_redeals_left

------

local kingsley                           = Sol.copy(klondike)
kingsley.Info.Name                       = "Kingsley"
kingsley.Info.Redeals                    = 0
kingsley.Foundation.create               = { Rule = { Build = "DownInSuit", Move = "Top", Empty = "King" } }
kingsley.Tableau.create                  = function(i)
    return {
        Initial = piles.Initial.top_face_up(i + 1),
        Layout = "Column",
        Rule = { Build = "UpAlternateColors", Move = "InSequence", Empty = "Ace" }
    }
end

------

local double_kingsley                    = Sol.copy(kingsley)
double_kingsley.Info.Name                = "Double Kingsley"
double_kingsley.Info.DeckCount           = 2
double_kingsley.Stock.Initial            = piles.Initial.face_down(59)
double_kingsley.Foundation.Size          = 8
double_kingsley.Tableau.Size             = 9
double_kingsley.on_created               = Sol.Layout.big_harp

------

local qc                                 = Sol.copy(klondike)
qc.Info.Name                             = "Q.C."
qc.Info.DeckCount                        = 2
qc.Info.Redeals                          = 1
qc.Stock.Initial                         = piles.Initial.face_down(80)
qc.Foundation                            = {
    Size   = 8,
    create = { Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" } }
}
qc.Tableau                               = {
    Size   = 6,
    create = {
        Initial = piles.Initial.top_face_up(4),
        Layout  = "Column",
        Rule    = { Build = "DownInSuit", Move = "Top", Empty = "Any" }
    }
}
qc.on_change                             = function(game)
    return game.Waste[1]:deal_to_group(game.Tableau, true) or game.Stock[1]:deal_to_group(game.Tableau, true)
end

------

local saratoga                           = Sol.copy(klondike_by_3s)
saratoga.Info.Name                       = "Saratoga"
saratoga.Tableau.create                  = function(i)
    return {
        Initial = piles.Initial.face_up(i + 1),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "King" }
    }
end

------

local thirty_six                         = Sol.copy(klondike)
thirty_six.Info.Name                     = "Thirty Six"
thirty_six.Info.Redeals                  = 0
thirty_six.Stock.Initial                 = piles.Initial.face_down(16)
thirty_six.Foundation                    = {
    Size = 4,
    create = { Rule = { Build = "UpInColor", Move = "Top", Empty = "Ace" } }
}
thirty_six.Tableau                       = {
    Size = 6,
    create = {
        Initial = piles.Initial.face_up(6),
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

local trigon                             = Sol.copy(klondike)
trigon.Info.Name                         = "Trigon"
trigon.Tableau.create                    = function(i)
    return {
        Initial = piles.Initial.top_face_up(i + 1),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "InSequence", Empty = "King" }
    }
end

------

local double_trigon                      = Sol.copy(trigon)
double_trigon.Info.Name                  = "Double Trigon"
double_trigon.Info.DeckCount             = 2
double_trigon.Stock.Initial              = piles.Initial.face_down(59)
double_trigon.Foundation.Size            = 8
double_trigon.Tableau.Size               = 9
double_trigon.on_created                 = Sol.Layout.big_harp

------

local whitehorse                         = Sol.copy(klondike)
whitehorse.Info.Name                     = "Whitehorse"
whitehorse.Info.CardDealCount            = 3
whitehorse.Info.Redeals                  = 0
whitehorse.Stock.Initial                 = piles.Initial.face_down(45)
whitehorse.Waste.Layout                  = "Fan"
whitehorse.Foundation                    = {
    Size = 4,
    create = { Rule = { Build = "UpInColor", Move = "Top", Empty = "Ace" } }
}
whitehorse.Tableau                       = {
    Size = 7,
    create = {
        Initial = piles.Initial.face_up(1),
        Layout  = "Column",
        Rule    = { Build = "DownAlternateColors", Move = "InSequence", Empty = "King" }
    }
}
whitehorse.on_change                     = function(game)
    return game.Waste[1]:deal_to_group(game.Tableau, true) or game.Stock[1]:deal_to_group(game.Tableau, true)
end

------

local australian_patience                = Sol.copy(klondike)
australian_patience.Info.Name            = "Australian Patience"
--australian_patience.Info.Family = "Klondike/Yukon"
australian_patience.Info.Redeals         = 0
australian_patience.Stock.Initial        = piles.Initial.face_down(24)
australian_patience.Tableau              = {
    Size = 7,
    create = {
        Initial = piles.Initial.face_up(4),
        Layout  = "Column",
        Rule    = { Build = "DownInSuit", Move = "FaceUp", Empty = "King" }
    }
}

------

local bim_bom                            = Sol.copy(klondike)
bim_bom.Info.Name                        = "Bim Bom"
bim_bom.Info.DeckCount                   = 2
bim_bom.Info.Redeals                     = 0
bim_bom.Stock.Initial                    = piles.Initial.face_down(64)
bim_bom.Foundation.Size                  = 8
bim_bom.Tableau                          = {
    Size = 8,
    create = {
        Initial = piles.Initial.face_up(5),
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
                Initial = piles.Initial.face_up(i < 7 and i + 1 or 8),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "Top", Empty = "Any" }
            }
        end
    },
    on_created = Sol.Layout.canister
}

------

local steps                              = Sol.copy(klondike)
steps.Info.Name                          = "Steps"
steps.Info.DeckCount                     = 2
steps.Info.Redeals                       = 1
steps.Stock.Initial                      = piles.Initial.face_down(76)
steps.Foundation.Size                    = 8
steps.Tableau                            = {
    Size   = 7,
    create = function(i)
        return {
            Initial = piles.Initial.top_face_up(i + 1),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
        }
    end
}
steps.on_created                         = Sol.Layout.klondike

------
local function usk_redeal(game)
    local cards = Sol.shuffle_tableau(game)
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

local usk                   = {
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
                Initial = piles.Initial.face_up(i < 7 and i + 1 or 8),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "King" }
            }
        end
    },
    on_redeal  = usk_redeal,
    on_created = Sol.Layout.canister
}

------

local westcliff             = Sol.copy(klondike)
westcliff.Info.Name         = "Westcliff"
westcliff.Info.Redeals      = 0
westcliff.Stock.Initial     = piles.Initial.face_down(22)
westcliff.Foundation.create = { Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" } }
westcliff.Tableau           = {
    Size = 10,
    create = {
        Initial = piles.Initial.top_face_up(3),
        Layout  = "Column",
        Rule    = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
    }
}

------

local whitehead             = Sol.copy(klondike)
whitehead.Info.Name         = "Whitehead"
whitehead.Info.Redeals      = 0
whitehead.Tableau.create    = function(i)
    return {
        Initial = piles.Initial.face_up(i + 1),
        Layout = "Column",
        Rule = { Build = "DownInColor", Move = "InSequenceInSuit", Empty = "Any" }
    }
end

------------------------

Sol.register_game(klondike)
Sol.register_game(klondike_by_3s)
Sol.register_game(double_klondike)
Sol.register_game(double_klondike_by_3s)
Sol.register_game(triple_klondike)
Sol.register_game(triple_klondike_by_3s)
Sol.register_game(half_klondike)
Sol.register_game(ali_baba)
Sol.register_game(arabella)
Sol.register_game(athena)
Sol.register_game(aunt_mary)
Sol.register_game(australian_patience)
Sol.register_game(big_deal)
Sol.register_game(big_forty)
Sol.register_game(big_harp)
Sol.register_game(bim_bom)
Sol.register_game(blind_alleys)
Sol.register_game(boost)
Sol.register_game(cassim)
Sol.register_game(chinaman)
Sol.register_game(chinese_klondike)
Sol.register_game(eastcliff)
Sol.register_game(eight_by_eight)
Sol.register_game(eight_by_eight2)
Sol.register_game(eight_by_eight3)
Sol.register_game(eight_sages)
Sol.register_game(gargantua)
Sol.register_game(gold_rush)
Sol.register_game(guardian)
Sol.register_game(kingsley)
Sol.register_game(double_kingsley)
Sol.register_game(qc)
Sol.register_game(saratoga)
Sol.register_game(somerset)
Sol.register_game(steps)
Sol.register_game(thirty_six)
Sol.register_game(trigon)
Sol.register_game(double_trigon)
Sol.register_game(usk)
Sol.register_game(westcliff)
Sol.register_game(whitehead)
Sol.register_game(whitehorse)
