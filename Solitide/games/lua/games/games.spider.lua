-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local spider                        = {
    Info       = {
        Name      = "Spider",
        Family    = "Spider",
        DeckCount = 2
    },
    Stock      = { Initial = Sol.Initial.face_down(50) },
    Foundation = {
        Size = 8,
        Pile = { Rule = Sol.Rules.spider_foundation }
    },
    Tableau    = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = Sol.Initial.top_face_up(i % 3 == 0 and 6 or 5),
                Layout  = "Column",
                Rule    = Sol.Rules.spider_tableau
            }
        end
    },
    deal       = function(game)
        for _, tableau in ipairs(game.Tableau) do
            if tableau.IsEmpty then return false end
        end

        return Sol.Ops.Deal.stock_to_tableau(game)
    end,
    on_init    = Sol.Layout.klondike
}

------

local spider_1_suit                 = Sol.copy(spider)
spider_1_suit.Info.Name             = "Spider (1 suit)"
spider_1_suit.Info.DeckCount        = 8
spider_1_suit.Info.DeckSuits        = { "Clubs" }

------

local spider_2_suits                = Sol.copy(spider)
spider_2_suits.Info.Name            = "Spider (2 suits)"
spider_2_suits.Info.DeckCount       = 4
spider_2_suits.Info.DeckSuits       = { "Clubs", "Hearts" }

------

local spider_4_decks                = Sol.copy(spider)
spider_4_decks.Info.Name            = "Spider (4 decks)"
spider_4_decks.Info.DeckCount       = 4
spider_4_decks.Stock.Initial        = Sol.Initial.face_down(130)
spider_4_decks.Foundation.Size      = 16
spider_4_decks.Tableau              = {
    Size = 13,
    Pile = {
        Initial = Sol.Initial.top_face_up(6),
        Layout  = "Column",
        Rule    = Sol.Rules.spider_tableau
    }
}

------

local big_spider                    = Sol.copy(spider)
big_spider.Info.Name                = "Big Spider"
big_spider.Info.DeckCount           = 3
big_spider.Stock.Initial            = Sol.Initial.face_down(78)
big_spider.Foundation.Size          = 12
big_spider.Tableau                  = {
    Size = 13,
    Pile = {
        Initial = Sol.Initial.top_face_up(6),
        Layout  = "Column",
        Rule    = Sol.Rules.spider_tableau
    }
}

------

local big_spider_1_suit             = Sol.copy(big_spider)
big_spider_1_suit.Info.Name         = "Big Spider (1 suit)"
big_spider_1_suit.Info.DeckCount    = 12
big_spider_1_suit.Info.DeckSuits    = { "Clubs" }

------

local big_spider_2_suits            = Sol.copy(big_spider)
big_spider_2_suits.Info.Name        = "Big Spider (2 suits)"
big_spider_2_suits.Info.DeckCount   = 6
big_spider_2_suits.Info.DeckSuits   = { "Clubs", "Hearts" }

------

local arachnida                     = Sol.copy(spider)
arachnida.Info.Name                 = "Arachnida"
arachnida.Foundation                = { Rule = Sol.Rules.none_none_none }
arachnida.Tableau.Pile              = function(i)
    return {
        Initial = Sol.Initial.top_face_up(i < 4 and 6 or 5),
        Layout  = "Column",
        Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.InRankOrDownByRank(), Move = Sol.Rules.Move.InSeqInSuitOrSameRank() }
    }
end

------

local black_widow                   = Sol.copy(spider)
black_widow.Info.Name               = "Black Widow"
black_widow.Tableau.Pile            = function(i)
    return {
        Initial = Sol.Initial.top_face_up(i % 3 == 0 and 6 or 5),
        Layout  = "Column",
        Rule    = Sol.Rules.any_downrank_inseq
    }
end

------

local chinese_spider                = Sol.copy(spider)
chinese_spider.Info.Name            = "Chinese Spider"
chinese_spider.Info.DeckCount       = 4
chinese_spider.Info.DeckSuits       = { "Clubs", "Spades", "Hearts" }
chinese_spider.Stock.Initial        = Sol.Initial.face_down(84)
chinese_spider.Foundation.Size      = 12
chinese_spider.Tableau              = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.top_face_up(6),
        Layout  = "Column",
        Rule    = Sol.Rules.spider_tableau
    }
}

------

local grounds_for_a_divorce         = Sol.copy(spider)
grounds_for_a_divorce.Info.Name     = "Grounds for a Divorce"
grounds_for_a_divorce.Stock.Initial = Sol.Initial.face_down(54)
grounds_for_a_divorce.Tableau.Pile  = {
    Initial = Sol.Initial.face_up(5),
    Layout  = "Column",
    Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(true), Move = Sol.Rules.Move.InSeqInSuit() }
}
grounds_for_a_divorce.deal          = function(game)
    local stock = game.Stock[1]
    if stock.IsEmpty then return false end

    for _, toPile in ipairs(game.Tableau) do
        if stock.IsEmpty then break end
        if not toPile.IsEmpty then
            stock:move_cards(toPile, #stock.Cards, 1, false)
            toPile:flip_up_top_card()
        end
    end
    return true
end

------

local big_divorce                   = Sol.copy(grounds_for_a_divorce)
big_divorce.Info.Name               = "Big Divorce"
big_divorce.Info.DeckCount          = 3
big_divorce.Stock.Initial           = Sol.Initial.face_down(78)
big_divorce.Foundation.Size         = 12
big_divorce.Tableau                 = {
    Size = 13,
    Pile = {
        Initial = Sol.Initial.face_up(6),
        Layout  = "Column",
        Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(true), Move = Sol.Rules.Move.InSeqInSuit() }
    }
}

------

local very_big_divorce              = Sol.copy(grounds_for_a_divorce)
very_big_divorce.Info.Name          = "Very Big Divorce"
very_big_divorce.Info.DeckCount     = 4
very_big_divorce.Stock.Initial      = Sol.Initial.face_down(136)
very_big_divorce.Foundation.Size    = 16
very_big_divorce.Tableau            = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.face_up(6),
        Layout  = "Column",
        Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(true), Move = Sol.Rules.Move.InSeqInSuit() }
    }
}


------

local open_spider                          = Sol.copy(spider)
open_spider.Info.Name                      = "Open Spider"
open_spider.Tableau.Pile                   = function(i)
    return {
        Initial = Sol.Initial.face_up(i % 3 == 0 and 6 or 5),
        Layout  = "Column",
        Rule    = Sol.Rules.spider_tableau
    }
end

------

local relaxed_spider                       = Sol.copy(spider)
relaxed_spider.Info.Name                   = "Relaxed Spider"
relaxed_spider.deal                        = Sol.Ops.Deal.stock_to_tableau

------

local spider_3x3                           = Sol.copy(spider)
spider_3x3.Info.Name                       = "Spider 3x3"
spider_3x3.Info.DeckCount                  = 3
spider_3x3.Info.DeckSuits                  = { "Clubs", "Spades", "Hearts" }
spider_3x3.Stock.Initial                   = Sol.Initial.face_down(52)
spider_3x3.Foundation.Size                 = 9
spider_3x3.Tableau                         = {
    Size = 13,
    Pile = {
        Initial = Sol.Initial.top_face_up(5),
        Layout  = "Column",
        Rule    = Sol.Rules.spider_tableau
    }
}

------

local kiev                                 = Sol.copy(spider)
kiev.Info.Name                             = "Kiev"
kiev.Info.DeckCount                        = 1
kiev.Stock.Initial                         = Sol.Initial.face_down(24)
kiev.Foundation.Size                       = 4
kiev.Tableau                               = {
    Size = 7,
    Pile = {
        Initial = Sol.Initial.top_face_up(4),
        Layout  = "Column",
        Rule    = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownInSuit(), Move = Sol.Rules.Move.FaceUp() }
    }
}

------

local dnieper                              = Sol.copy(kiev)
dnieper.Info.Name                          = "Dnieper"
dnieper.Tableau.Pile.Rule                  = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownInSuit(true), Move = Sol.Rules.Move.FaceUp() }

------

local spiderette                           = Sol.copy(spider)
spiderette.Info.Name                       = "Spiderette"
spiderette.Info.DeckCount                  = 1
spiderette.Stock.Initial                   = Sol.Initial.face_down(24)
spiderette.Foundation.Size                 = 4
spiderette.Tableau                         = {
    Size = 7,
    Pile = function(i)
        return {
            Initial = Sol.Initial.top_face_up(i + 1),
            Layout  = "Column",
            Rule    = Sol.Rules.spider_tableau
        }
    end
}

------

local baby_spiderette                      = Sol.copy(spiderette)
baby_spiderette.Info.Name                  = "Baby Spiderette"
baby_spiderette.Tableau.Pile               = function(i)
    return {
        Initial = Sol.Initial.top_face_up(i + 1),
        Layout  = "Column",
        Rule    = Sol.Rules.any_downrank_inseq
    }
end

------

local will_o_the_wisp                      = Sol.copy(spiderette)
will_o_the_wisp.Info.Name                  = "Will o' the Wisp"
will_o_the_wisp.Stock.Initial              = Sol.Initial.face_down(31)
will_o_the_wisp.Tableau.Pile               = function(i)
    return {
        Initial = Sol.Initial.top_face_up(3),
        Layout  = "Column",
        Rule    = Sol.Rules.spider_tableau
    }
end

------

local fair_maids                           = Sol.copy(will_o_the_wisp)
fair_maids.Info.Name                       = "Fair Maids"
fair_maids.Stock.Initial                   = Sol.Initial.face_down(24)
fair_maids.Tableau.Pile                    = {
    Initial = Sol.Initial.top_face_up(4),
    Layout  = "Column",
    Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.InSeqAlternateColors() }
}

------

local ten_across_initial <const>           = {
    { true,  true,  true,  true,  true },
    { false, true,  true,  true,  true },
    { false, false, true,  true,  true },
    { false, false, false, true,  true },
    { false, false, false, false, true },
    { false, false, false, false, true },
    { false, false, false, true,  true },
    { false, false, true,  true,  true },
    { false, true,  true,  true,  true },
    { true,  true,  true,  true,  true }
}

local ten_across                           = {
    Info       = {
        Name      = "Ten Across",
        Family    = "Spider",
        --Family = "Yukon/Spider"
        DeckCount = 1
    },
    FreeCell   = {
        Size = 2,
        Pile = {
            Initial = Sol.Initial.face_up(1),
            Rule = Sol.Rules.any_none_top
        }
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.spider_foundation }
    },
    Tableau    = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = ten_across_initial[i + 1],
                Layout = "Column",
                Rule = Sol.Rules.king_downsuit_faceup
            }
        end
    },
    on_init    = Sol.Layout.free_cell
}

------

local panopticon_initial <const>           = {
    { false, true,  true,  true,  true, true },
    { false, false, true,  true,  true, true },
    { false, false, false, true,  true, true },
    { false, false, false, false, true, true },
    { false, false, false, false, true, true },
    { false, false, false, true,  true, true },
    { false, false, true,  true,  true, true },
    { false, true,  true,  true,  true, true }
}

local panopticon                           = Sol.copy(ten_across)
panopticon.Info.Name                       = "Panopticon"
panopticon.FreeCell.Size                   = 4
panopticon.Tableau                         = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = panopticon_initial[i + 1],
            Layout = "Column",
            Rule = Sol.Rules.king_downsuit_faceup
        }
    end
}

------

local astrocyte                            = {
    Info = {
        Name      = "Astrocyte",
        Family    = "Spider",
        DeckCount = 2
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(40)
    },
    FreeCell = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 1.5, y = 0 },
                Rule = Sol.Rules.any_none_top
            }
        end
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 6, y = 0 },
                Rule = Sol.Rules.spider_foundation
            }
        end
    },
    Tableau = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 1 },
                Initial = Sol.Initial.top_face_up(8),
                Layout = "Column",
                Rule = Sol.Rules.spider_tableau
            }
        end
    },
    deal = Sol.Ops.Deal.stock_to_tableau
}

------

local brush                                = {
    Info = {
        Name      = "Brush",
        Family    = "Spider",
        DeckCount = 2
    },
    Stock = { Initial = Sol.Initial.face_down(64) },
    Waste = {},
    Foundation = {
        Size = 8,
        Pile = { Rule = Sol.Rules.spider_foundation }
    },
    Tableau = {
        Size = 10,
        Pile = {
            Initial = Sol.Initial.face_up(4),
            Layout = "Column",
            Rule = Sol.Rules.spider_tableau
        }
    },
    deal = Sol.Ops.Deal.stock_to_waste,
    on_init = Sol.Layout.klondike
}

------

local scorpion_head                        = {
    Info       = {
        Name      = "Scorpion Head",
        Family    = "Spider",
        DeckCount = 1
    },
    FreeCell   = {
        Size = 4,
        Pile = { Rule = Sol.Rules.any_none_top }
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.spider_foundation }
    },
    Tableau    = {
        Size = 7,
        Pile = function(i)
            local initial = {}
            if i > 2 then
                initial = { false, false, false, true, true, true, true }
            else
                initial = Sol.Initial.face_up(8)
            end
            return {
                Initial = initial,
                Layout  = "Column",
                Rule    = Sol.Rules.king_downsuit_faceup
            }
        end
    },
    on_init    = Sol.Layout.free_cell
}

------

local spidike                              = {
    Info       = {
        Name      = "Spidike",
        Family    = "Spider",
        DeckCount = 1
    },
    Stock      = {
        Initial = Sol.Initial.face_down(24)
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i + 1),
                Layout  = "Column",
                Rule    = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.InSeqInSuit() }
            }
        end
    },
    deal       = Sol.Ops.Deal.stock_to_tableau,
    on_init    = Sol.Layout.klondike
}

------

local ukrainian_solitaire                  = {
    Info       = {
        Name      = "Ukrainian Solitaire",
        Family    = "Spider",
        DeckCount = 1
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.spider_foundation }
    },
    Tableau    = {
        Size = 7,
        Pile = function(i)
            local initial = {}
            if i == 0 then
                initial[1] = true
            else
                for j = 1, i + 5 do
                    initial[j] = j > i
                end
            end

            return {
                Initial = initial,
                Layout  = "Column",
                Rule    = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownInSuit(), Move = Sol.Rules.Move.FaceUp() }
            }
        end
    },
    on_init    = Sol.Layout.klondike
}

------

local double_ukrainian_solitaire           = Sol.copy(ukrainian_solitaire)
double_ukrainian_solitaire.Info.Name       = "Double Ukrainian Solitaire"
double_ukrainian_solitaire.Info.DeckCount  = 2
double_ukrainian_solitaire.Foundation.Size = 8
double_ukrainian_solitaire.Tableau         = {
    Size = 10,
    Pile = function(i)
        local initial = Sol.Initial.face_down(math.min(i + 6, 14))
        for j = #initial - 5, #initial do
            initial[j] = true
        end

        return {
            Initial = initial,
            Layout  = "Column",
            Rule    = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownInSuit(), Move = Sol.Rules.Move.FaceUp() }
        }
    end
}


------

------------------------

Sol.register_game(spider)
Sol.register_game(arachnida)
Sol.register_game(astrocyte)
Sol.register_game(baby_spiderette)
Sol.register_game(big_divorce)
Sol.register_game(big_spider)
Sol.register_game(big_spider_1_suit)
Sol.register_game(big_spider_2_suits)
Sol.register_game(black_widow)
Sol.register_game(brush)
Sol.register_game(chinese_spider)
Sol.register_game(dnieper)
Sol.register_game(double_ukrainian_solitaire)
Sol.register_game(fair_maids)
Sol.register_game(grounds_for_a_divorce)
Sol.register_game(kiev)
Sol.register_game(open_spider)
Sol.register_game(panopticon)
Sol.register_game(relaxed_spider)
Sol.register_game(scorpion_head)
Sol.register_game(spider_1_suit)
Sol.register_game(spider_2_suits)
Sol.register_game(spider_4_decks)
Sol.register_game(spider_3x3)
Sol.register_game(spiderette)
Sol.register_game(spidike)
Sol.register_game(ten_across)
Sol.register_game(ukrainian_solitaire)
Sol.register_game(very_big_divorce)
Sol.register_game(will_o_the_wisp)
