-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local klondike = {
    Info       = {
        Name      = "Klondike",
        Family    = "Klondike",
        DeckCount = 1,
        Redeals   = -1
    },
    Stock      = { Initial = Sol.Initial.face_down(24) },
    Waste      = {},
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = Sol.Initial.top_face_up(i + 1),
                Layout  = Sol.Pile.Layout.Column,
                Rule    = Sol.Rules.king_downac_inseq
            }
        end
    },
    redeal     = Sol.Ops.Redeal.waste_to_stock,
    deal       = Sol.Ops.Deal.stock_to_waste,
    on_init    = Sol.Layout.klondike
}


------

local klondike_by_3s        = Sol.copy(klondike)
klondike_by_3s.Info.Name    = "Klondike by Threes"
klondike_by_3s.Waste.Layout = Sol.Pile.Layout.Fan
klondike_by_3s.deal         = Sol.Ops.Deal.stock_to_waste_by_3


------

local double_klondike           = Sol.copy(klondike)
double_klondike.Info.Name       = "Double Klondike"
double_klondike.Info.DeckCount  = 2
double_klondike.Stock.Initial   = Sol.Initial.face_down(59)
double_klondike.Foundation.Size = 8
double_klondike.Tableau.Size    = 9
double_klondike.on_init         = Sol.Layout.big_harp


------

local double_klondike_by_3s     = Sol.copy(double_klondike)
double_klondike_by_3s.Info.Name = "Double Klondike by Threes"
double_klondike_by_3s.deal      = Sol.Ops.Deal.stock_to_waste_by_3


------

local triple_klondike           = Sol.copy(klondike)
triple_klondike.Info.Name       = "Triple Klondike"
triple_klondike.Info.DeckCount  = 3
triple_klondike.Stock.Initial   = Sol.Initial.face_down(65)
triple_klondike.Foundation.Size = 12
triple_klondike.Tableau.Size    = 13
triple_klondike.on_init         = Sol.Layout.big_harp


------

local triple_klondike_by_3s     = Sol.copy(triple_klondike)
triple_klondike_by_3s.Info.Name = "Triple Klondike by Threes"
triple_klondike_by_3s.deal      = Sol.Ops.Deal.stock_to_waste_by_3


------

local quadruple_klondike           = Sol.copy(klondike)
quadruple_klondike.Info.Name       = "Quadruple Klondike"
quadruple_klondike.Info.DeckCount  = 4
quadruple_klondike.Stock.Initial   = Sol.Initial.face_down(72)
quadruple_klondike.Foundation.Size = 16
quadruple_klondike.Tableau.Size    = 16
quadruple_klondike.on_init         = Sol.Layout.big_harp


------

local quadruple_klondike_by_3s     = Sol.copy(quadruple_klondike)
quadruple_klondike_by_3s.Info.Name = "Quadruple Klondike by Threes"
quadruple_klondike_by_3s.deal      = Sol.Ops.Deal.stock_to_waste_by_3


------

local half_klondike           = Sol.copy(klondike)
half_klondike.Info.Name       = "Half Klondike"
half_klondike.Info.DeckSuits  = { "Spades", "Hearts" }
half_klondike.Stock.Initial   = Sol.Initial.face_down(16)
half_klondike.Foundation.Size = 2
half_klondike.Tableau.Size    = 4


------

local big_deal = {
    Info       = {
        Name      = "Big Deal",
        Family    = "Klondike",
        DeckCount = 4,
        Redeals   = 1
    },
    Stock      = {
        Position = { x = 0, y = 7 },
        Initial  = Sol.Initial.face_down(130)
    },
    Waste      = { Position = { x = 1, y = 7 } },
    Foundation = {
        Size = 16,
        Pile = function(i)
            return {
                Position = { x = i % 2 + 12, y = i // 2 },
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 12,
        Pile = function(i)
            return {
                Position = { x = i, y = 0 },
                Initial  = Sol.Initial.top_face_up(i + 1),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.king_downac_inseq
            }
        end
    },
    redeal     = Sol.Ops.Redeal.waste_to_stock,
    deal       = Sol.Ops.Deal.stock_to_waste,
}


------

local big_forty         = Sol.copy(klondike)
big_forty.Info.Name     = "Big Forty"
big_forty.Stock.Initial = Sol.Initial.face_down(12)
big_forty.Tableau       = {
    Size = 10,
    Pile = {
        Initial = Sol.Initial.top_face_up(4),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downsuit_inseq
    }
}


------

local big_harp           = Sol.copy(klondike)
big_harp.Info.Name       = "Big Harp"
big_harp.Info.DeckCount  = 2
big_harp.Info.Redeals    = 0
big_harp.Stock.Initial   = Sol.Initial.face_down(49)
big_harp.Foundation.Size = 8
big_harp.Tableau         = {
    Size = 10,
    Pile = function(i)
        return {
            Initial = Sol.Initial.top_face_up(10 - i),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.any_downac_inseq
        }
    end
}
big_harp.on_init         = Sol.Layout.big_harp


------

local inquisitor        = Sol.copy(big_harp)
inquisitor.Info.Name    = "Inquisitor"
inquisitor.Info.Redeals = 2
inquisitor.Tableau.Pile = function(i)
    return {
        Initial = Sol.Initial.face_up(i + 1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downsuit_inseq
    }
end
inquisitor.deal         = Sol.Ops.Deal.stock_to_waste_by_3


------

local lady_jane        = Sol.copy(big_harp)
lady_jane.Info.Name    = "Lady Jane"
lady_jane.Info.Redeals = 1
lady_jane.Tableau.Pile = function(i)
    return {
        Initial = Sol.Initial.face_up(i + 1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.spider_tableau
    }
end
lady_jane.deal         = Sol.Ops.Deal.stock_to_waste_by_3


------

local ali_baba           = Sol.copy(klondike)
ali_baba.Info.Name       = "Ali Baba"
ali_baba.Stock.Initial   = Sol.Initial.face_down(8)
ali_baba.Tableau         = {
    Size = 10,
    Pile = {
        Initial = Sol.Initial.top_face_up(4),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downsuit_inseq
    }
}
ali_baba.on_before_setup = Sol.Ops.Shuffle.ace_to_foundation


------

local australian_patience         = Sol.copy(klondike)
australian_patience.Info.Name     = "Australian Patience"
--australian_patience.Info.Family = "Klondike/Yukon"
australian_patience.Info.Redeals  = 0
australian_patience.Stock.Initial = Sol.Initial.face_down(24)
australian_patience.Tableau       = {
    Size = 7,
    Pile = {
        Initial = Sol.Initial.face_up(4),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.king_downsuit_faceup
    }
}


------

local canberra        = Sol.copy(australian_patience)
canberra.Info.Name    = "Canberra"
--canberra.Info.Family = "Klondike/Yukon"
canberra.Info.Redeals = 1


------

local raw_prawn             = Sol.copy(australian_patience)
raw_prawn.Info.Name         = "Raw Prawn"
--raw_prawn.Info.Family = "Klondike/Yukon"
raw_prawn.Tableau.Pile.Rule = Sol.Rules.any_downsuit_faceup


------

local tasmanian_patience        = Sol.copy(australian_patience)
tasmanian_patience.Info.Name    = "Tasmanian Patience"
--tasmanian_patience.Info.Family = "Klondike/Yukon"
tasmanian_patience.Info.Redeals = -1
tasmanian_patience.deal         = Sol.Ops.Deal.stock_to_waste_by_3


------

local arabella           = Sol.copy(klondike)
arabella.Info.Name       = "Arabella"
arabella.Info.DeckCount  = 3
arabella.Stock.Initial   = Sol.Initial.face_down(65)
arabella.Foundation.Size = 12
arabella.Tableau         = {
    Size = 13,
    Pile = function(i)
        return {
            Initial = Sol.Initial.top_face_up(i + 1),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.InSeqInSuit() }
        }
    end
}
arabella.on_init         = Sol.Layout.big_harp


------

local athena        = Sol.copy(klondike)
athena.Info.Name    = "Athena"
athena.Tableau.Pile = {
    Initial = Sol.Initial.alternate(4, false),
    Layout  = Sol.Pile.Layout.Column,
    Rule    = Sol.Rules.any_downac_inseq
}


------

local aunt_mary         = Sol.copy(klondike)
aunt_mary.Info.Name     = "Aunt Mary"
aunt_mary.Info.Redeals  = 0
aunt_mary.Stock.Initial = Sol.Initial.face_down(16)
aunt_mary.Tableau       = {
    Size = 6,
    Pile = function(i)
        return {
            Initial = { i < 1, i < 2, i < 3, i < 4, i < 5, i < 6 },
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.any_downac_inseq
        }
    end
}


------

local bim_bom           = Sol.copy(klondike)
bim_bom.Info.Name       = "Bim Bom"
bim_bom.Info.DeckCount  = 2
bim_bom.Info.Redeals    = 0
bim_bom.Stock.Initial   = Sol.Initial.face_down(64)
bim_bom.Foundation.Size = 8
bim_bom.Tableau         = {
    Size = 8,
    Pile = {
        Initial = Sol.Initial.face_up(5),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downsuit_faceup
    }
}


------

local blind_alleys           = Sol.copy(klondike)
blind_alleys.Info.Name       = "Blind Alleys"
blind_alleys.Info.Redeals    = 1
blind_alleys.Stock.Initial   = Sol.Initial.face_down(30)
blind_alleys.Tableau         = {
    Size = 6,
    Pile = {
        Initial = Sol.Initial.top_face_up(3),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downac_inseq
    }
}
blind_alleys.on_before_setup = Sol.Ops.Shuffle.ace_to_foundation


------

local pas_seul         = Sol.copy(klondike)
pas_seul.Info.Name     = "Pas Seul"
pas_seul.Info.Redeals  = 0
pas_seul.Stock.Initial = Sol.Initial.face_down(46)
pas_seul.Tableau       = {
    Size = 6,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downac_inseq
    }
}


------

local boost         = Sol.copy(klondike)
boost.Info.Name     = "Boost"
boost.Info.Redeals  = 2
boost.Stock.Initial = Sol.Initial.face_down(42)
boost.Tableau.Size  = 4


------

local cassim           = Sol.copy(klondike)
cassim.Info.Name       = "Cassim"
cassim.Stock.Initial   = Sol.Initial.face_down(20)
cassim.Tableau.Pile    = {
    Initial = Sol.Initial.top_face_up(4),
    Layout  = Sol.Pile.Layout.Column,
    Rule    = Sol.Rules.any_downsuit_inseq
}
cassim.on_before_setup = Sol.Ops.Shuffle.ace_to_foundation


------

local chinaman        = Sol.copy(klondike_by_3s)
chinaman.Info.Name    = "Chinaman"
chinaman.Info.Redeals = 2
chinaman.Tableau      = {
    Size = 7,
    Pile = function(i)
        return {
            Initial = Sol.Initial.top_face_up(i + 1),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownAnyButOwnSuit(), Move = Sol.Rules.Move.InSeq() }
        }
    end
}


------

local chinese_klondike           = Sol.copy(klondike)
chinese_klondike.Info.Name       = "Chinese Klondike"
chinese_klondike.Info.DeckCount  = 3
chinese_klondike.Info.DeckSuits  = { "Clubs", "Spades", "Hearts" }
chinese_klondike.Stock.Initial   = Sol.Initial.face_down(39)
chinese_klondike.Foundation.Size = 9
chinese_klondike.Tableau.Size    = 12
chinese_klondike.on_init         = Sol.Layout.big_harp


------

local eastcliff         = Sol.copy(klondike)
eastcliff.Info.Name     = "Eastcliff"
eastcliff.Info.Redeals  = 0
eastcliff.Stock.Initial = Sol.Initial.face_down(31)
eastcliff.Tableau       = {
    Size = 7,
    Pile = {
        Initial = Sol.Initial.top_face_up(3),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downac_inseq
    }
}


------

local eight_by_eight = {
    Info       = {
        Name = "8 x 8",
        Family = "Klondike",
        DeckCount = 2,
        Redeals = -1
    },
    Stock      = { Initial = Sol.Initial.face_down(40) },
    Waste      = {},
    Foundation = {
        Size = 8,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(8),
                Layout  = Sol.Pile.Layout.Column,
                Rule    = Sol.Rules.any_downac_inseq
            }
        end
    },
    redeal     = Sol.Ops.Redeal.waste_to_stock,
    deal       = Sol.Ops.Deal.stock_to_waste,
    on_init    = Sol.Layout.big_harp
}


------

local eight_by_eight2           = Sol.copy(klondike_by_3s)
eight_by_eight2.Info.Name       = "Eight by Eight"
eight_by_eight2.Info.DeckCount  = 2
eight_by_eight2.Info.Redeals    = 2
eight_by_eight2.Stock.Initial   = Sol.Initial.face_down(40)
eight_by_eight2.Foundation.Size = 8
eight_by_eight2.Tableau         = {
    Size = 8,
    Pile = {
        Initial = Sol.Initial.face_up(8),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = { Base = Sol.Rules.Base.AnySingle(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.InSeq() }
    }
}
eight_by_eight2.deal            = Sol.Ops.Deal.stock_to_waste_by_redeals_left


------

local eight_by_eight3        = Sol.copy(eight_by_eight)
eight_by_eight3.Info.Name    = "Eight Times Eight"
eight_by_eight3.Info.Redeals = 2
eight_by_eight3.Tableau.Pile = {
    Initial = Sol.Initial.face_up(8),
    Layout  = Sol.Pile.Layout.Column,
    Rule    = Sol.Rules.any_downac_inseq
}


------

local eight_sages           = Sol.copy(klondike)
eight_sages.Info.Name       = "Eight Sages"
eight_sages.Info.DeckCount  = 2
eight_sages.Info.Redeals    = 1
eight_sages.Stock.Initial   = Sol.Initial.face_down(96)
eight_sages.Foundation.Size = 8
eight_sages.Tableau         = {
    Size = 8,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downac_top
    }
}
eight_sages.can_play        = function(game, targetPile, targetCardIndex, card, numCards)
    if targetPile.Type == Sol.Pile.Type.Tableau then
        local srcPile = game:find_pile(card)
        if srcPile.Type ~= Sol.Pile.Type.Waste then
            return false
        end
    end

    return game:can_play(targetPile, targetCardIndex, card, numCards)
end


------

local gargantua        = Sol.copy(double_klondike)
gargantua.Info.Name    = "Gargantua"
gargantua.Info.Redeals = 1


------

local open_gargantua        = Sol.copy(gargantua)
open_gargantua.Info.Name    = "Open Gargantua"
open_gargantua.Tableau.Pile = function(i)
    return {
        Initial = Sol.Initial.face_up(i + 1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.king_downac_inseq
    }
end


------

local guardian = {
    Info        = {
        Name      = "Guardian",
        Family    = "Klondike",
        DeckCount = 1,
        Redeals   = -1
    },
    Stock       = {
        Position = { x = 0, y = 0 },
        Initial  = Sol.Initial.face_down(40)
    },
    Waste       = {
        Position = { x = 1, y = 0 },
        Layout   = Sol.Pile.Layout.Fan
    },
    Foundation  = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau     = {
        Size = 12,
        Pile = function(i)
            local pile = Sol.Ops.Pyramid.pile(12, 3, { x = 1, y = 0.5 }, i)
            pile.Rule  = Sol.Rules.any_downac_inseq
            return pile
        end
    },
    redeal      = Sol.Ops.Redeal.waste_to_stock,
    deal        = Sol.Ops.Deal.stock_to_waste_by_3,
    on_end_turn = function(game) Sol.Ops.Pyramid.flip(12, 3, game.Tableau) end
}


------

local gold_rush        = Sol.copy(klondike_by_3s)
gold_rush.Info.Name    = "Gold Rush"
gold_rush.Info.Redeals = 2
gold_rush.deal         = Sol.Ops.Deal.stock_to_waste_by_redeals_left


------

local kingsley           = Sol.copy(klondike)
kingsley.Info.Name       = "Kingsley"
kingsley.Info.Redeals    = 0
kingsley.Foundation.Pile = { Rule = Sol.Rules.king_downsuit_top }
kingsley.Tableau.Pile    = function(i)
    return {
        Initial = Sol.Initial.top_face_up(i + 1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = { Base = Sol.Rules.Base.Ace(), Build = Sol.Rules.Build.UpAlternateColors(), Move = Sol.Rules.Move.InSeq() }
    }
end


------

local double_kingsley           = Sol.copy(kingsley)
double_kingsley.Info.Name       = "Double Kingsley"
double_kingsley.Info.DeckCount  = 2
double_kingsley.Stock.Initial   = Sol.Initial.face_down(59)
double_kingsley.Foundation.Size = 8
double_kingsley.Tableau.Size    = 9
double_kingsley.on_init         = Sol.Layout.big_harp


------

local lanes           = Sol.copy(klondike)
lanes.Info.Name       = "Lanes"
lanes.Info.Redeals    = 1
lanes.Stock.Initial   = Sol.Initial.face_down(30)
lanes.Foundation.Pile = { Rule = Sol.Rules.ace_upsuit_none }
lanes.Tableau         = {
    Size = 6,
    Pile = {
        Initial = Sol.Initial.face_up(3),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downac_top
    }
}
lanes.on_before_setup = Sol.Ops.Shuffle.ace_to_foundation


------

local legion         = Sol.copy(klondike)
legion.Info.Name     = "Legion"
legion.Info.Redeals  = 0
legion.Stock.Initial = Sol.Initial.face_down(20)
legion.Tableau       = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = Sol.Initial.alternate(i < 4 and i * 2 + 1 or (7 - i) * 2 + 1, true),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.king_downac_top
        }
    end
}


------

local moving_left         = Sol.copy(double_klondike)
moving_left.Info.Name     = "Moving Left"
moving_left.Info.Redeals  = 0
moving_left.Stock.Initial = Sol.Initial.face_down(49)
moving_left.Tableau.Size  = 10
moving_left.on_end_turn   = function(game)
    local function move_cards(from, to)
        local cards = from.Cards
        for j = 1, #cards do
            if cards[j].IsFaceUp then
                from:move_cards(to, j, #cards - j + 1, false)
                from:flip_up_top_card()
                break
            end
        end
    end

    local tableau = game.Tableau
    for i = 1, #tableau - 1 do
        local to = tableau[i]
        if to.IsEmpty then move_cards(tableau[i + 1], to) end
    end
end
moving_left.on_init       = Sol.Layout.klondike


------

local souter        = Sol.copy(moving_left)
souter.Info.Name    = "Souter"
souter.Info.Redeals = 1


------

local nine_across         = Sol.copy(klondike)
nine_across.Info.Name     = "Nine Across"
nine_across.Stock.Initial = Sol.Initial.face_down(7)
nine_across.Foundation    = {
    Size = 4,
    Pile = {
        Rule = Sol.Rules.ff_upsuit_none
    }
}
nine_across.Tableau       = {
    Size = 9,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i + 1),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = { Base = Sol.Rules.Base.FirstFoundation(-1), Build = Sol.Rules.Build.DownAlternateColors(true), Move = Sol.Rules.Move.InSeq() }
        }
    end
}
nine_across.can_play      = function(game, targetPile, targetCardIndex, card, numCards)
    if targetPile.Type == Sol.Pile.Type.Foundation and targetPile.Index == 1 and targetPile.IsEmpty then -- allow any card on first foundation
        return true
    end

    return game:can_play(targetPile, targetCardIndex, card, numCards)
end


------

local pantagruel        = Sol.copy(double_klondike)
pantagruel.Info.Name    = "Pantagruel"
pantagruel.Info.Redeals = 0
pantagruel.Tableau.Pile = function(i)
    return {
        Initial = Sol.Initial.top_face_up(i + 1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downac_inseq
    }
end


------

local pas_seul         = Sol.copy(klondike)
pas_seul.Info.Name     = "Pas Seul"
pas_seul.Info.Redeals  = 0
pas_seul.Stock.Initial = Sol.Initial.face_down(46)
pas_seul.Tableau       = {
    Size = 6,
    Pile = {
        Initial = Sol.Initial.top_face_up(1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downac_inseq
    }
}


------

local qc          = Sol.copy(klondike)
qc.Info.Name      = "Q.C."
qc.Info.DeckCount = 2
qc.Info.Redeals   = 1
qc.Stock.Initial  = Sol.Initial.face_down(80)
qc.Foundation     = {
    Size = 8,
    Pile = { Rule = Sol.Rules.ace_upsuit_none }
}
qc.Tableau        = {
    Size = 6,
    Pile = {
        Initial = Sol.Initial.top_face_up(4),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downsuit_top
    }
}
qc.on_end_turn    = Sol.Ops.Deal.waste_or_stock_to_empty_tableau


------

local saratoga        = Sol.copy(klondike_by_3s)
saratoga.Info.Name    = "Saratoga"
saratoga.Tableau.Pile = function(i)
    return {
        Initial = Sol.Initial.face_up(i + 1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.king_downac_inseq
    }
end


------

local smokey        = Sol.copy(klondike)
smokey.Info.Name    = "Smokey"
smokey.Info.Redeals = 2
smokey.Tableau.Pile = function(i)
    return {
        Initial = Sol.Initial.face_up(i + 1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downcolor_inseqsuit
    }
end


------

local somerset = {
    Info       = {
        Name      = "Somerset",
        Family    = "Klondike",
        DeckCount = 1
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 7 and i + 1 or 8),
                Layout  = Sol.Pile.Layout.Column,
                Rule    = Sol.Rules.any_downac_top
            }
        end
    },
    on_init    = Sol.Layout.canister
}


------

local steps           = Sol.copy(klondike)
steps.Info.Name       = "Steps"
steps.Info.DeckCount  = 2
steps.Info.Redeals    = 1
steps.Stock.Initial   = Sol.Initial.face_down(76)
steps.Foundation.Size = 8
steps.Tableau         = {
    Size = 7,
    Pile = function(i)
        return {
            Initial = Sol.Initial.top_face_up(i + 1),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.any_downac_inseq
        }
    end
}
steps.on_init         = Sol.Layout.klondike


------

local spike = {
    Info       = {
        Name      = "Spike",
        Family    = "Klondike",
        DeckCount = 1
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(24)
    },
    Waste      = {
        Size = 3,
        Pile = function(i)
            return {
                Position = { x = 0, y = i + 1 },
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = Sol.Rules.none_none_top
            }
        end
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = 3.5 + i, y = 0 },
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 7,
        Pile = function(i)
            return {
                Position = { x = 1.5 + i, y = 1 },
                Initial  = Sol.Initial.top_face_up(i + 1),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.king_downac_inseq
            }
        end
    },
    deal       = function(game) return Sol.Ops.Deal.to_group(game.Stock[1], game.Waste) end
}


------

local thirty_six         = Sol.copy(klondike)
thirty_six.Info.Name     = "Thirty-Six"
thirty_six.Info.Redeals  = 0
thirty_six.Stock.Initial = Sol.Initial.face_down(16)
thirty_six.Tableau       = {
    Size = 6,
    Pile = {
        Initial = Sol.Initial.face_up(6),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downrank_inseq
    }
}
thirty_six.on_setup      = function(game, card, pile)
    if pile.Type == Sol.Pile.Type.Tableau and card.Rank == "Ace" then
        return game.PlaceTop(card, game.Foundation, true)
    end

    return false
end


------

local taking_silk           = Sol.copy(thirty_six)
taking_silk.Info.Name       = "Taking Silk"
taking_silk.Info.DeckCount  = 2
taking_silk.Stock.Initial   = Sol.Initial.face_down(68)
taking_silk.Foundation.Size = 8


------

local thieves_of_egypt           = Sol.copy(klondike)
thieves_of_egypt.Info.Name       = "Thieves of Egypt"
thieves_of_egypt.Info.DeckCount  = 2
thieves_of_egypt.Info.Redeals    = 1
thieves_of_egypt.Stock.Initial   = Sol.Initial.face_down(49)
thieves_of_egypt.Foundation.Size = 8
thieves_of_egypt.Tableau         = {
    Size = 10,
    Pile = function(i)
        local cardCount <const> = { 1, 3, 5, 7, 9, 10, 8, 6, 4, 2 }
        return {
            Initial = Sol.Initial.face_up(cardCount[i + 1]),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.king_downac_inseq
        }
    end
}


------

local thumb_and_pouch        = Sol.copy(klondike)
thumb_and_pouch.Info.Name    = "Thumb and Pouch"
thumb_and_pouch.Info.Redeals = 0
thumb_and_pouch.Tableau.Pile = function(i)
    return {
        Initial = Sol.Initial.top_face_up(i + 1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownAnyButOwnSuit(), Move = Sol.Rules.Move.InSeq() }
    }
end


------

local trigon        = Sol.copy(klondike)
trigon.Info.Name    = "Trigon"
trigon.Tableau.Pile = function(i)
    return {
        Initial = Sol.Initial.top_face_up(i + 1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.king_downsuit_inseq
    }
end


------

local double_trigon           = Sol.copy(trigon)
double_trigon.Info.Name       = "Double Trigon"
double_trigon.Info.DeckCount  = 2
double_trigon.Stock.Initial   = Sol.Initial.face_down(59)
double_trigon.Foundation.Size = 8
double_trigon.Tableau.Size    = 9
double_trigon.on_init         = Sol.Layout.big_harp


------
local function usk_redeal(game)
    local tableau = game.Tableau
    local cards = Sol.shuffle_piles(game, { tableau })
    if #cards == 0 then return false end

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
        Name      = "Usk",
        Family    = "Klondike",
        DeckCount = 1,
        Redeals   = 1
    },
    Stock      = {},
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 7 and i + 1 or 8),
                Layout  = Sol.Pile.Layout.Column,
                Rule    = Sol.Rules.king_downac_inseq
            }
        end
    },
    redeal     = usk_redeal,
    on_init    = Sol.Layout.canister
}


------

local westcliff           = Sol.copy(klondike)
westcliff.Info.Name       = "Westcliff"
westcliff.Info.Redeals    = 0
westcliff.Stock.Initial   = Sol.Initial.face_down(22)
westcliff.Foundation.Pile = { Rule = Sol.Rules.ace_upsuit_none }
westcliff.Tableau         = {
    Size = 10,
    Pile = {
        Initial = Sol.Initial.top_face_up(3),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downac_inseq
    }
}


------

local whitehead        = Sol.copy(klondike)
whitehead.Info.Name    = "Whitehead"
whitehead.Info.Redeals = 0
whitehead.Tableau.Pile = function(i)
    return {
        Initial = Sol.Initial.face_up(i + 1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downcolor_inseqsuit
    }
end


------

local whitehorse         = Sol.copy(klondike_by_3s)
whitehorse.Info.Name     = "Whitehorse"
whitehorse.Info.Redeals  = 0
whitehorse.Stock.Initial = Sol.Initial.face_down(45)
whitehorse.Tableau       = {
    Size = 7,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.king_downac_inseq
    }
}
whitehorse.on_end_turn   = Sol.Ops.Deal.waste_or_stock_to_empty_tableau


--with freecells:
------

local batsford         = Sol.copy(double_klondike)
batsford.Info.Name     = "Batsford"
batsford.Info.Redeals  = 0
batsford.Stock.Initial = Sol.Initial.face_down(49)
batsford.FreeCell      = {
    Position = { x = 0, y = 3 },
    Layout   = Sol.Pile.Layout.Squared,
    Rule     = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.Top(), Limit = 3 }
}
batsford.Tableau.Size  = 10


------

local batsford_again        = Sol.copy(batsford)
batsford_again.Info.Name    = "Batsford Again"
batsford_again.Info.Redeals = 1


------

local doorway = {
    Info       = {
        Name      = "Doorway",
        Family    = "Klondike",
        DeckCount = 1
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(47)
    },
    Waste      = { Position = { x = 1, y = 0 } },
    FreeCell   = {
        Size = 2,
        Pile = function(i)
            if i == 0 then
                return {
                    Position = { x = 0, y = 1 },
                    Rule     = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Base.None(), Move = Sol.Rules.Move.Top() }
                }
            else
                return {
                    Position = { x = 7, y = 1 },
                    Rule     = {
                        Base = Sol.Rules.Base.Ranks({ "Queen" }), Build = Sol.Rules.Base.None(), Move = Sol.Rules.Move.Top()
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
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 5,
        Pile = function(i)
            return {
                Position = { x = i + 1.5, y = 1 },
                Initial  = Sol.Initial.top_face_up(1),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.any_downrank_top
            }
        end
    },
    redeal     = Sol.Ops.Redeal.waste_to_stock,
    deal       = Sol.Ops.Deal.stock_to_waste
}


------

local sawayama = { --from Last Call BBS
    Info       = {
        Name      = "Sawayama",
        Family    = "Klondike",
        DeckCount = 1
    },
    Stock      = { --Stock turns into FreeCell when empty
        Position = { x = 1, y = 0 },
        Initial  = Sol.Initial.face_down(24)
    },
    FreeCell   = {
        Position = { x = 1, y = 0 },
        Rule     = {
            Base = {
                Hint = { "Any" },
                Func = function(game, _, _)
                    return game.Stock[1].IsEmpty
                end
            },
            Build = Sol.Rules.Base.None(),
            Move = Sol.Rules.Move.Top()
        }
    },
    Waste      = {
        Position = { x = 2, y = 0 },
        Layout   = Sol.Pile.Layout.Row
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = 0, y = i },
                Rule     = Sol.Rules.ace_upsuit_none
            }
        end
    },
    Tableau    = {
        Size = 7,
        Pile = function(i)
            return {
                Position = { x = i + 1, y = 1 },
                Initial  = Sol.Initial.face_up(i + 1),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.any_downac_inseq
            }
        end
    },
    deal       = Sol.Ops.Deal.stock_to_waste_by_3
}


------

local russian_patience_ranks <const> = { "Ace", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King" }
local russian_patience_rankvalues <const> = { Ace = 1, Seven = 2, Eight = 3, Nine = 4, Ten = 5, Jack = 6, Queen = 7, King = 8 }

local russian_patience = {
    Info            = {
        Name      = "Russian Patience",
        Family    = "Klondike",
        DeckCount = 2,
        DeckRanks = russian_patience_ranks
    },
    Foundation      = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i % 2 + 7, y = i // 2 },
                Rule     = {
                    Base = Sol.Rules.Base.Ace(),
                    Build = {
                        Hint = { "UpInSuit" },
                        Func = function(_, base, drop)
                            if base.Suit ~= drop.Suit then return false end
                            local target = russian_patience_rankvalues[base.Rank] + 1
                            return russian_patience_ranks[target] == drop.Rank
                        end
                    },
                    Move = Sol.Rules.Move.None()
                }
            }
        end
    },
    Tableau         = {
        Size = 7,
        Pile = function(i)
            return {
                Position = { x = i, y = 0 },
                Initial  = Sol.Initial.face_up(9),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = {
                    Base = Sol.Rules.Base.AnySingle(),
                    Build = {
                        Hint = { "DownAlternateColors" },
                        Func = function(_, base, drop)
                            if base.Color == drop.Color then return false end
                            local target = russian_patience_rankvalues[base.Rank] - 1
                            return russian_patience_ranks[target] == drop.Rank
                        end
                    },
                    Move = Sol.Rules.Move.InSeq()
                }
            }
        end
    },
    on_before_setup = function(game, card)
        if card.Rank == "Ace" then
            return game.PlaceTop(card, game.Foundation[1], true)
        end

        return false
    end
}


------

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
Sol.register_game(canberra)
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
Sol.register_game(nine_across)
Sol.register_game(moving_left)
Sol.register_game(open_gargantua)
Sol.register_game(pantagruel)
Sol.register_game(pas_seul)
Sol.register_game(qc)
Sol.register_game(quadruple_klondike)
Sol.register_game(quadruple_klondike_by_3s)
Sol.register_game(raw_prawn)
Sol.register_game(russian_patience)
Sol.register_game(saratoga)
Sol.register_game(sawayama)
Sol.register_game(somerset)
Sol.register_game(smokey)
Sol.register_game(souter)
Sol.register_game(spike)
Sol.register_game(steps)
Sol.register_game(taking_silk)
Sol.register_game(tasmanian_patience)
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
