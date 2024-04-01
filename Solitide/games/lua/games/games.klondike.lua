-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local klondike                           = {
    Info             = {
        Name          = "Klondike",
        Family        = "Klondike",
        DeckCount     = 1,
        CardDealCount = 1,
        Redeals       = -1
    },
    Stock            = { Initial = ops.Initial.face_down(24) },
    Waste            = {},
    Foundation       = {
        Size = 4,
        Pile = { Rule = rules.ace_upsuit_top }
    },
    Tableau          = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = ops.Initial.top_face_up(i + 1),
                Layout = "Column",
                Rule = rules.king_downac_inseq
            }
        end
    },
    on_redeal        = ops.Redeal.waste_to_stock,
    on_deal          = ops.Deal.stock_to_waste,
    on_piles_created = Sol.Layout.klondike
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
double_klondike.Stock.Initial            = ops.Initial.face_down(59)
double_klondike.Foundation.Size          = 8
double_klondike.Tableau.Size             = 9
double_klondike.on_piles_created         = Sol.Layout.big_harp

------

local double_klondike_by_3s              = Sol.copy(double_klondike)
double_klondike_by_3s.Info.Name          = "Double Klondike by Threes"
double_klondike_by_3s.Info.CardDealCount = 3

------

local triple_klondike                    = Sol.copy(klondike)
triple_klondike.Info.Name                = "Triple Klondike"
triple_klondike.Info.DeckCount           = 3
triple_klondike.Stock.Initial            = ops.Initial.face_down(65)
triple_klondike.Foundation.Size          = 12
triple_klondike.Tableau.Size             = 13
triple_klondike.on_piles_created         = Sol.Layout.big_harp

------

local triple_klondike_by_3s              = Sol.copy(triple_klondike)
triple_klondike_by_3s.Info.Name          = "Triple Klondike by Threes"
triple_klondike_by_3s.Info.CardDealCount = 3

------

local half_klondike                      = Sol.copy(klondike)
half_klondike.Info.Name                  = "Half Klondike"
half_klondike.Info.DeckSuits             = { "Spades", "Hearts" }
half_klondike.Stock.Initial              = ops.Initial.face_down(16)
half_klondike.Foundation.Size            = 2
half_klondike.Tableau.Size               = 4

------

local big_deal                           = {
    Info       = {
        Name = "Big Deal",
        Family = "Klondike",
        DeckCount = 4,
        CardDealCount = 1,
        Redeals = 1
    },
    Stock      = {
        Position = { x = 0, y = 7 },
        Initial = ops.Initial.face_down(130)
    },
    Waste      = { Position = { x = 1, y = 7 } },
    Foundation = {
        Size = 16,
        Pile = function(i)
            return {
                Position = { x = i % 2 + 12, y = i // 2 },
                Rule = rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 12,
        Pile = function(i)
            return {
                Position = { x = i, y = 0 },
                Initial  = ops.Initial.top_face_up(i + 1),
                Layout   = "Column",
                Rule     = rules.king_downac_inseq
            }
        end
    },
    on_redeal  = ops.Redeal.waste_to_stock,
    on_deal    = ops.Deal.stock_to_waste,
}

------

local big_forty                          = Sol.copy(klondike)
big_forty.Info.Name                      = "Big Forty"
big_forty.Stock.Initial                  = ops.Initial.face_down(12)
big_forty.Tableau                        = {
    Size = 10,
    Pile = {
        Initial = ops.Initial.top_face_up(4),
        Layout = "Column",
        Rule = rules.any_downsuit_inseq
    }
}

------

local big_harp                           = Sol.copy(klondike)
big_harp.Info.Name                       = "Big Harp"
big_harp.Info.DeckCount                  = 2
big_harp.Info.Redeals                    = 0
big_harp.Stock.Initial                   = ops.Initial.face_down(49)
big_harp.Foundation.Size                 = 8
big_harp.Tableau                         = {
    Size = 10,
    Pile = function(i)
        return {
            Initial = ops.Initial.top_face_up(10 - i),
            Layout = "Column",
            Rule = rules.any_downac_inseq
        }
    end
}
big_harp.on_piles_created                = Sol.Layout.big_harp

------

local inquisitor                         = Sol.copy(big_harp)
inquisitor.Info.Name                     = "Inquisitor"
inquisitor.Info.Redeals                  = 2
inquisitor.Info.CardDealCount            = 3
inquisitor.Tableau.Pile                  = function(i)
    return {
        Initial = ops.Initial.face_up(i + 1),
        Layout = "Column",
        Rule = rules.any_downsuit_inseq
    }
end

------

local lady_jane                          = Sol.copy(big_harp)
lady_jane.Info.Name                      = "Lady Jane"
lady_jane.Info.Redeals                   = 1
lady_jane.Info.CardDealCount             = 3
lady_jane.Tableau.Pile                   = function(i)
    return {
        Initial = ops.Initial.face_up(i + 1),
        Layout = "Column",
        Rule = { Base = rules.Base.Any(), Build = rules.Build.DownByRank(), Move = rules.Move.InSeqInSuit() }
    }
end

------

local ali_baba                           = Sol.copy(klondike)
ali_baba.Info.Name                       = "Ali Baba"
ali_baba.Stock.Initial                   = ops.Initial.face_down(8)
ali_baba.Tableau                         = {
    Size = 10,
    Pile = {
        Initial = ops.Initial.top_face_up(4),
        Layout = "Column",
        Rule = rules.any_downsuit_inseq
    }
}
ali_baba.on_before_shuffle               = ops.Shuffle.ace_to_foundation

------

local australian_patience                = Sol.copy(klondike)
australian_patience.Info.Name            = "Australian Patience"
--australian_patience.Info.Family = "Klondike/Yukon"
australian_patience.Info.Redeals         = 0
australian_patience.Stock.Initial        = ops.Initial.face_down(24)
australian_patience.Tableau              = {
    Size = 7,
    Pile = {
        Initial = ops.Initial.face_up(4),
        Layout  = "Column",
        Rule    = rules.king_downsuit_faceup
    }
}

------

local arabella                           = Sol.copy(klondike)
arabella.Info.Name                       = "Arabella"
arabella.Info.DeckCount                  = 3
arabella.Stock.Initial                   = ops.Initial.face_down(65)
arabella.Foundation.Size                 = 12
arabella.Tableau                         = {
    Size = 13,
    Pile = function(i)
        return {
            Initial = ops.Initial.top_face_up(i + 1),
            Layout = "Column",
            Rule = { Base = rules.Base.King(), Build = rules.Build.DownByRank(), Move = rules.Move.InSeqInSuit() }
        }
    end
}
arabella.on_piles_created                = Sol.Layout.big_harp

------

local athena                             = Sol.copy(klondike)
athena.Info.Name                         = "Athena"
athena.Tableau.Pile                      = {
    Initial = ops.Initial.alternate(4, false),
    Layout = "Column",
    Rule = rules.any_downac_inseq
}

------

local aunt_mary                          = Sol.copy(klondike)
aunt_mary.Info.Name                      = "Aunt Mary"
aunt_mary.Info.Redeals                   = 0
aunt_mary.Stock.Initial                  = ops.Initial.face_down(16)
aunt_mary.Tableau                        = {
    Size = 6,
    Pile = function(i)
        return {
            Initial = { i < 1, i < 2, i < 3, i < 4, i < 5, i < 6 },
            Layout = "Column",
            Rule = rules.any_downac_inseq
        }
    end
}

------

local bim_bom                            = Sol.copy(klondike)
bim_bom.Info.Name                        = "Bim Bom"
bim_bom.Info.DeckCount                   = 2
bim_bom.Info.Redeals                     = 0
bim_bom.Stock.Initial                    = ops.Initial.face_down(64)
bim_bom.Foundation.Size                  = 8
bim_bom.Tableau                          = {
    Size = 8,
    Pile = {
        Initial = ops.Initial.face_up(5),
        Layout  = "Column",
        Rule    = rules.any_downsuit_faceup
    }
}

------

local blind_alleys                       = Sol.copy(klondike)
blind_alleys.Info.Name                   = "Blind Alleys"
blind_alleys.Info.Redeals                = 1
blind_alleys.Stock.Initial               = ops.Initial.face_down(30)
blind_alleys.Tableau                     = {
    Size = 6,
    Pile = {
        Initial = ops.Initial.top_face_up(3),
        Layout = "Column",
        Rule = rules.any_downac_inseq
    }
}
blind_alleys.on_before_shuffle           = ops.Shuffle.ace_to_foundation

------

local boost                              = Sol.copy(klondike)
boost.Info.Name                          = "Boost"
boost.Info.Redeals                       = 2
boost.Stock.Initial                      = ops.Initial.face_down(42)
boost.Tableau.Size                       = 4

------

local cassim                             = Sol.copy(klondike)
cassim.Info.Name                         = "Cassim"
cassim.Stock.Initial                     = ops.Initial.face_down(20)
cassim.Tableau.Pile                      = {
    Initial = ops.Initial.top_face_up(4),
    Layout = "Column",
    Rule = rules.any_downsuit_inseq
}
cassim.on_before_shuffle                 = ops.Shuffle.ace_to_foundation

------

local chinaman                           = Sol.copy(klondike_by_3s)
chinaman.Info.Name                       = "Chinaman"
chinaman.Info.Redeals                    = 2
chinaman.Tableau                         = {
    Size = 7,
    Pile = function(i)
        return {
            Initial = ops.Initial.top_face_up(i + 1),
            Layout = "Column",
            Rule = { Base = rules.Base.King(), Build = rules.Build.DownAnyButOwnSuit(), Move = rules.Move.InSeq() }
        }
    end
}

------

local chinese_klondike                   = Sol.copy(klondike)
chinese_klondike.Info.Name               = "Chinese Klondike"
chinese_klondike.Info.DeckCount          = 3
chinese_klondike.Info.DeckSuits          = { "Clubs", "Spades", "Hearts" }
chinese_klondike.Stock.Initial           = ops.Initial.face_down(39)
chinese_klondike.Foundation.Size         = 9
chinese_klondike.Tableau.Size            = 12
chinese_klondike.on_piles_created        = Sol.Layout.big_harp

------

local eastcliff                          = Sol.copy(klondike)
eastcliff.Info.Name                      = "Eastcliff"
eastcliff.Info.Redeals                   = 0
eastcliff.Stock.Initial                  = ops.Initial.face_down(31)
eastcliff.Tableau                        = {
    Size = 7,
    Pile = {
        Initial = ops.Initial.top_face_up(3),
        Layout = "Column",
        Rule = rules.any_downac_inseq
    }
}

------

local eight_by_eight                     = {
    Info             = {
        Name = "8 x 8",
        Family = "Klondike",
        DeckCount = 2,
        CardDealCount = 1,
        Redeals = -1
    },
    Stock            = { Initial = ops.Initial.face_down(40) },
    Waste            = {},
    Foundation       = {
        Size = 8,
        Pile = { Rule = rules.ace_upsuit_top }
    },
    Tableau          = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = ops.Initial.face_up(8),
                Layout  = "Column",
                Rule    = rules.any_downac_inseq
            }
        end
    },
    on_redeal        = ops.Redeal.waste_to_stock,
    on_deal          = ops.Deal.stock_to_waste,
    on_piles_created = Sol.Layout.big_harp
}

------

local eight_by_eight2                    = Sol.copy(klondike_by_3s)
eight_by_eight2.Info.Name                = "Eight by Eight"
eight_by_eight2.Info.DeckCount           = 2
eight_by_eight2.Info.Redeals             = 2
eight_by_eight2.Stock.Initial            = ops.Initial.face_down(40)
eight_by_eight2.Foundation.Size          = 8
eight_by_eight2.Tableau                  = {
    Size = 8,
    Pile = {
        Initial = ops.Initial.face_up(8),
        Layout = "Column",
        Rule = { Base = rules.Base.AnySingle(), Build = rules.Build.DownByRank(), Move = rules.Move.InSeq() }
    }
}
eight_by_eight2.on_deal                  = ops.Deal.stock_to_waste_by_redeals_left

------

local eight_by_eight3                    = Sol.copy(eight_by_eight)
eight_by_eight3.Info.Name                = "Eight Times Eight"
eight_by_eight3.Info.Redeals             = 2
eight_by_eight3.Tableau.Pile             = {
    Initial = ops.Initial.face_up(8),
    Layout = "Column",
    Rule = rules.any_downac_inseq
}

------

local eight_sages                        = Sol.copy(klondike)
eight_sages.Info.Name                    = "Eight Sages"
eight_sages.Info.DeckCount               = 2
eight_sages.Info.Redeals                 = 1
eight_sages.Stock.Initial                = ops.Initial.face_down(96)
eight_sages.Foundation.Size              = 8
eight_sages.Tableau                      = {
    Size = 8,
    Pile = {
        Initial = ops.Initial.face_up(1),
        Layout = "Column",
        Rule = rules.any_downac_top
    }
}
eight_sages.check_playable               = function(game, targetPile, targetIndex, drop, numCards)
    if targetPile.Type == "Tableau" then
        local srcPile = game:find_pile(drop)
        if srcPile.Type ~= "Waste" then
            return false
        end
    end

    return game:can_play(targetPile, targetIndex, drop, numCards)
end

------

local gargantua                          = Sol.copy(double_klondike)
gargantua.Info.Name                      = "Gargantua"
gargantua.Info.Redeals                   = 1

------

local open_gargantua                     = Sol.copy(gargantua)
open_gargantua.Info.Name                 = "Open Gargantua"
open_gargantua.Tableau.Pile              = function(i)
    return {
        Initial = ops.Initial.face_up(i + 1),
        Layout = "Column",
        Rule = rules.king_downac_inseq
    }
end

------

local guardian                           = {
    Info        = {
        Name = "Guardian",
        Family = "Klondike",
        DeckCount = 1,
        CardDealCount = 3,
        Redeals = -1
    },
    Stock       = {
        Position = { x = 0, y = 0 },
        Initial = ops.Initial.face_down(40)
    },
    Waste       = {
        Position = { x = 1, y = 0 },
        Layout = "Fan"
    },
    Foundation  = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Rule = rules.ace_upsuit_top
            }
        end
    },
    Tableau     = {
        Size = 12,
        Pile = function(i)
            local tab = {
                HasMarker = i < 3,
                Layout    = "Column",
                Rule      = rules.any_downac_inseq
            }
            if i < 7 then
                tab.Initial = ops.Initial.face_down(1)
            else
                tab.Initial = ops.Initial.face_up(1)
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
    on_redeal   = ops.Redeal.waste_to_stock,
    on_deal     = ops.Deal.stock_to_waste,
    on_end_turn = function(game)
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

local gold_rush                          = Sol.copy(klondike_by_3s)
gold_rush.Info.Name                      = "Gold Rush"
gold_rush.Info.Redeals                   = 2
gold_rush.on_deal                        = ops.Deal.stock_to_waste_by_redeals_left

------

local kingsley                           = Sol.copy(klondike)
kingsley.Info.Name                       = "Kingsley"
kingsley.Info.Redeals                    = 0
kingsley.Foundation.Pile                 = { Rule = rules.king_downsuit_top }
kingsley.Tableau.Pile                    = function(i)
    return {
        Initial = ops.Initial.top_face_up(i + 1),
        Layout = "Column",
        Rule = { Base = rules.Base.Ace(), Build = rules.Build.UpAlternateColors(), Move = rules.Move.InSeq() }
    }
end

------

local double_kingsley                    = Sol.copy(kingsley)
double_kingsley.Info.Name                = "Double Kingsley"
double_kingsley.Info.DeckCount           = 2
double_kingsley.Stock.Initial            = ops.Initial.face_down(59)
double_kingsley.Foundation.Size          = 8
double_kingsley.Tableau.Size             = 9
double_kingsley.on_piles_created         = Sol.Layout.big_harp

------

local lanes                              = Sol.copy(klondike)
lanes.Info.Name                          = "Lanes"
lanes.Info.Redeals                       = 1
lanes.Stock.Initial                      = ops.Initial.face_down(30)
lanes.Foundation.Pile                    = { Rule = rules.ace_upsuit_none }
lanes.Tableau                            = {
    Size = 6,
    Pile = {
        Initial = ops.Initial.face_up(3),
        Layout = "Column",
        Rule = rules.any_downac_top
    }
}
lanes.on_before_shuffle                  = ops.Shuffle.ace_to_foundation

------

local legion                             = Sol.copy(klondike)
legion.Info.Name                         = "Legion"
legion.Info.Redeals                      = 0
legion.Stock.Initial                     = ops.Initial.face_down(20)
legion.Tableau                           = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = ops.Initial.alternate(i < 4 and i * 2 + 1 or (7 - i) * 2 + 1, true),
            Layout = "Column",
            Rule = rules.king_downac_top
        }
    end
}

------

local moving_left                        = Sol.copy(double_klondike)
moving_left.Info.Name                    = "Moving Left"
moving_left.Info.Redeals                 = 0
moving_left.Stock.Initial                = ops.Initial.face_down(49)
moving_left.Tableau.Size                 = 10
moving_left.on_end_turn                  = function(game)
    local tableau = game.Tableau
    for i = 1, #tableau - 1 do
        local to = tableau[i]
        local from = tableau[i + 1]
        if to.IsEmpty then
            local cards = from.Cards
            for j = 1, #cards do
                if cards[j].IsFaceUp then
                    from:move_cards(to, j, #cards - j + 1, false)
                    from:flip_up_top_card()
                    break
                end
            end
        end
    end
end

------

local souter                             = Sol.copy(moving_left)
souter.Info.Name                         = "Souter"
souter.Info.Redeals                      = 1

------

local pantagruel                         = Sol.copy(double_klondike)
pantagruel.Info.Name                     = "Pantagruel"
pantagruel.Info.Redeals                  = 0
pantagruel.Tableau.Pile                  = function(i)
    return {
        Initial = ops.Initial.top_face_up(i + 1),
        Layout = "Column",
        Rule = rules.any_downac_inseq
    }
end

------

local pas_seul                           = Sol.copy(klondike)
pas_seul.Info.Name                       = "Pas Seul"
pas_seul.Info.Redeals                    = 0
pas_seul.Stock.Initial                   = ops.Initial.face_down(46)
pas_seul.Tableau                         = {
    Size = 6,
    Pile = {
        Initial = ops.Initial.top_face_up(1),
        Layout = "Column",
        Rule = rules.any_downac_inseq
    }
}

------

local qc                                 = Sol.copy(klondike)
qc.Info.Name                             = "Q.C."
qc.Info.DeckCount                        = 2
qc.Info.Redeals                          = 1
qc.Stock.Initial                         = ops.Initial.face_down(80)
qc.Foundation                            = {
    Size = 8,
    Pile = { Rule = rules.ace_upsuit_none }
}
qc.Tableau                               = {
    Size = 6,
    Pile = {
        Initial = ops.Initial.top_face_up(4),
        Layout  = "Column",
        Rule    = rules.any_downsuit_top
    }
}
qc.on_end_turn                           = function(game)
    return ops.Deal.to_group(game.Waste[1], game.Tableau, true) or ops.Deal.to_group(game.Stock[1], game.Tableau, true)
end

------

local saratoga                           = Sol.copy(klondike_by_3s)
saratoga.Info.Name                       = "Saratoga"
saratoga.Tableau.Pile                    = function(i)
    return {
        Initial = ops.Initial.face_up(i + 1),
        Layout = "Column",
        Rule = rules.king_downac_inseq
    }
end

------

local smokey                             = Sol.copy(klondike)
smokey.Info.Name                         = "Smokey"
smokey.Info.Redeals                      = 2
smokey.Tableau.Pile                      = function(i)
    return {
        Initial = ops.Initial.face_up(i + 1),
        Layout = "Column",
        Rule = { Base = rules.Base.Any(), Build = rules.Build.DownInColor(), Move = rules.Move.InSeqInSuit() }
    }
end

------

local somerset                           = {
    Info             = {
        Name          = "Somerset",
        Family        = "Klondike",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation       = {
        Size = 4,
        Pile = { Rule = rules.ace_upsuit_top }
    },
    Tableau          = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = ops.Initial.face_up(i < 7 and i + 1 or 8),
                Layout = "Column",
                Rule = rules.any_downac_top
            }
        end
    },
    on_piles_created = Sol.Layout.canister
}

------

local steps                              = Sol.copy(klondike)
steps.Info.Name                          = "Steps"
steps.Info.DeckCount                     = 2
steps.Info.Redeals                       = 1
steps.Stock.Initial                      = ops.Initial.face_down(76)
steps.Foundation.Size                    = 8
steps.Tableau                            = {
    Size = 7,
    Pile = function(i)
        return {
            Initial = ops.Initial.top_face_up(i + 1),
            Layout = "Column",
            Rule = rules.any_downac_inseq
        }
    end
}
steps.on_piles_created                   = Sol.Layout.klondike

------

local spike                              = {
    Info       = {
        Name          = "Spike",
        Family        = "Klondike",
        DeckCount     = 1,
        CardDealCount = 3,
        Redeals       = 0
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = ops.Initial.face_down(24)
    },
    Waste      = {
        Size = 3,
        Pile = function(i)
            return {
                Position = { x = 0, y = i + 1 },
                Layout = "Squared",
                Rule = rules.none_none_top
            }
        end
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = 3.5 + i, y = 0 },
                Rule = rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 7,
        Pile = function(i)
            return {
                Position = { x = 1.5 + i, y = 1 },
                Initial = ops.Initial.top_face_up(i + 1),
                Layout = "Column",
                Rule = rules.king_downac_inseq
            }
        end
    },
    on_deal    = function(game)
        local stock1 = game.Stock[1]
        local waste = game.Waste
        return ops.Deal.to_pile(stock1, waste[1], 1) and
            ops.Deal.to_pile(stock1, waste[2], 1) and
            ops.Deal.to_pile(stock1, waste[3], 1)
    end
}

------

local thirty_six                         = Sol.copy(klondike)
thirty_six.Info.Name                     = "Thirty Six"
thirty_six.Info.Redeals                  = 0
thirty_six.Stock.Initial                 = ops.Initial.face_down(16)
thirty_six.Foundation                    = {
    Size = 4,
    Pile = { Rule = { Base = rules.Base.Ace(), Build = rules.Build.UpInColor(), Move = rules.Move.Top() } }
}
thirty_six.Tableau                       = {
    Size = 6,
    Pile = {
        Initial = ops.Initial.face_up(6),
        Layout  = "Column",
        Rule    = { Base = rules.Base.Any(), Build = rules.Build.DownByRank(), Move = rules.Move.InSeq() }
    }
}
thirty_six.on_shuffle                    = function(game, card, pileType)
    if pileType == "Tableau" and card.Rank == "Ace" then
        return game.PlaceTop(card, game.Foundation, true)
    end

    return false
end

------

local taking_silk                        = Sol.copy(thirty_six)
taking_silk.Info.Name                    = "Taking Silk"
taking_silk.Info.DeckCount               = 2
taking_silk.Stock.Initial                = ops.Initial.face_down(68)
taking_silk.Foundation.Size              = 8

------

local thieves_of_egypt                   = Sol.copy(klondike)
thieves_of_egypt.Info.Name               = "Thieves of Egypt"
thieves_of_egypt.Info.DeckCount          = 2
thieves_of_egypt.Info.Redeals            = 1
thieves_of_egypt.Stock.Initial           = ops.Initial.face_down(49)
thieves_of_egypt.Foundation.Size         = 8
thieves_of_egypt.Tableau                 = {
    Size = 10,
    Pile = function(i)
        local cardCount <const> = { 1, 3, 5, 7, 9, 10, 8, 6, 4, 2 }
        return {
            Initial = ops.Initial.face_up(cardCount[i + 1]),
            Layout = "Column",
            Rule = rules.king_downac_inseq
        }
    end
}

------

local thumb_and_pouch                    = Sol.copy(klondike)
thumb_and_pouch.Info.Name                = "Thumb and Pouch"
thumb_and_pouch.Info.Redeals             = 0
thumb_and_pouch.Tableau.Pile             = function(i)
    return {
        Initial = ops.Initial.top_face_up(i + 1),
        Layout = "Column",
        Rule = { Base = rules.Base.Any(), Build = rules.Build.DownAnyButOwnSuit(), Move = rules.Move.InSeq() }
    }
end


------

local trigon                   = Sol.copy(klondike)
trigon.Info.Name               = "Trigon"
trigon.Tableau.Pile            = function(i)
    return {
        Initial = ops.Initial.top_face_up(i + 1),
        Layout = "Column",
        Rule = rules.king_downsuit_inseq
    }
end

------

local double_trigon            = Sol.copy(trigon)
double_trigon.Info.Name        = "Double Trigon"
double_trigon.Info.DeckCount   = 2
double_trigon.Stock.Initial    = ops.Initial.face_down(59)
double_trigon.Foundation.Size  = 8
double_trigon.Tableau.Size     = 9
double_trigon.on_piles_created = Sol.Layout.big_harp

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
    Info             = {
        Name          = "Usk",
        Family        = "Klondike",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 1
    },
    Stock            = {},
    Foundation       = {
        Size = 4,
        Pile = { Rule = rules.ace_upsuit_top }
    },
    Tableau          = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = ops.Initial.face_up(i < 7 and i + 1 or 8),
                Layout = "Column",
                Rule = rules.king_downac_inseq
            }
        end
    },
    on_redeal        = usk_redeal,
    on_piles_created = Sol.Layout.canister
}

------

local westcliff             = Sol.copy(klondike)
westcliff.Info.Name         = "Westcliff"
westcliff.Info.Redeals      = 0
westcliff.Stock.Initial     = ops.Initial.face_down(22)
westcliff.Foundation.Pile   = { Rule = rules.ace_upsuit_none }
westcliff.Tableau           = {
    Size = 10,
    Pile = {
        Initial = ops.Initial.top_face_up(3),
        Layout  = "Column",
        Rule    = rules.any_downac_inseq
    }
}

------

local whitehead             = Sol.copy(klondike)
whitehead.Info.Name         = "Whitehead"
whitehead.Info.Redeals      = 0
whitehead.Tableau.Pile      = function(i)
    return {
        Initial = ops.Initial.face_up(i + 1),
        Layout = "Column",
        Rule = { Base = rules.Base.Any(), Build = rules.Build.DownInColor(), Move = rules.Move.InSeqInSuit() }
    }
end

------

local whitehorse            = Sol.copy(klondike_by_3s)
whitehorse.Info.Name        = "Whitehorse"
whitehorse.Info.Redeals     = 0
whitehorse.Stock.Initial    = ops.Initial.face_down(45)
whitehorse.Waste.Layout     = "Fan"
whitehorse.Foundation       = {
    Size = 4,
    Pile = { Rule = { Base = rules.Base.Ace(), Build = rules.Build.UpInColor(), Move = rules.Move.Top() } }
}
whitehorse.Tableau          = {
    Size = 7,
    Pile = {
        Initial = ops.Initial.face_up(1),
        Layout  = "Column",
        Rule    = rules.king_downac_inseq
    }
}
whitehorse.on_end_turn      = function(game)
    return ops.Deal.to_group(game.Waste[1], game.Tableau, true) or ops.Deal.to_group(game.Stock[1], game.Tableau, true)
end

--with freecells:
------

local batsford              = Sol.copy(double_klondike)
batsford.Info.Name          = "Batsford"
batsford.Info.Redeals       = 0
batsford.Stock.Initial      = ops.Initial.face_down(49)
batsford.FreeCell           = {
    Position = { x = 0, y = 3 },
    Layout = "Squared",
    Rule = { Base = rules.Base.King(), Build = rules.Build.InRank(), Move = rules.Move.Top(), Limit = 3 }
}
batsford.Tableau.Size       = 10

------

local batsford_again        = Sol.copy(batsford)
batsford_again.Info.Name    = "Batsford Again"
batsford_again.Info.Redeals = 1

------

local doorway               = {
    Info       = {
        Name          = "Doorway",
        Family        = "Klondike",
        DeckCount     = 1,
        CardDealCount = 1,
        Redeals       = 0
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = ops.Initial.face_down(47)
    },
    Waste      = { Position = { x = 1, y = 0 } },
    FreeCell   = {
        Size = 2,
        Pile = function(i)
            if i == 0 then
                return {
                    Position = { x = 0, y = 1 },
                    Rule = { Base = rules.Base.King(), Build = rules.Base.None(), Move = rules.Move.Top() }
                }
            else
                return {
                    Position = { x = 7, y = 1 },
                    Rule = {
                        Base = rules.Base.Ranks({ "Queen" }), Build = rules.Base.None(), Move = rules.Move.Top()
                    }
                }
            end
        end
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 4, y = 0 },
                Rule = rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 5,
        Pile = function(i)
            return {
                Position = { x = i + 1.5, y = 1 },
                Initial = ops.Initial.top_face_up(1),
                Layout = "Column",
                Rule = rules.any_downrank_top
            }
        end
    },
    on_redeal  = ops.Redeal.waste_to_stock,
    on_deal    = ops.Deal.stock_to_waste
}


------------------------

Sol.register_game(klondike)
Sol.register_game(ali_baba)
Sol.register_game(arabella)
Sol.register_game(athena)
Sol.register_game(aunt_mary)
Sol.register_game(australian_patience)
Sol.register_game(batsford)
Sol.register_game(batsford_again)
Sol.register_game(big_deal)
Sol.register_game(big_forty)
Sol.register_game(big_harp)
Sol.register_game(bim_bom)
Sol.register_game(blind_alleys)
Sol.register_game(boost)
Sol.register_game(cassim)
Sol.register_game(chinaman)
Sol.register_game(chinese_klondike)
Sol.register_game(doorway)
Sol.register_game(double_kingsley)
Sol.register_game(double_klondike)
Sol.register_game(double_klondike_by_3s)
Sol.register_game(double_trigon)
Sol.register_game(eastcliff)
Sol.register_game(eight_by_eight)
Sol.register_game(eight_by_eight2)
Sol.register_game(eight_by_eight3)
Sol.register_game(eight_sages)
Sol.register_game(gargantua)
Sol.register_game(gold_rush)
Sol.register_game(guardian)
Sol.register_game(half_klondike)
Sol.register_game(inquisitor)
Sol.register_game(kingsley)
Sol.register_game(klondike_by_3s)
Sol.register_game(lady_jane)
Sol.register_game(lanes)
Sol.register_game(legion)
Sol.register_game(moving_left)
Sol.register_game(open_gargantua)
Sol.register_game(pantagruel)
Sol.register_game(qc)
Sol.register_game(saratoga)
Sol.register_game(somerset)
Sol.register_game(smokey)
Sol.register_game(souter)
Sol.register_game(spike)
Sol.register_game(steps)
Sol.register_game(taking_silk)
Sol.register_game(thieves_of_egypt)
Sol.register_game(thirty_six)
Sol.register_game(thumb_and_pouch)
Sol.register_game(trigon)
Sol.register_game(triple_klondike)
Sol.register_game(triple_klondike_by_3s)
Sol.register_game(usk)
Sol.register_game(westcliff)
Sol.register_game(whitehead)
Sol.register_game(whitehorse)
