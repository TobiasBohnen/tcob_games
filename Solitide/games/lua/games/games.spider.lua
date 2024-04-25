-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local function spider_check_and_move(game)
    local function check(tableau)
        local cards = tableau.Cards

        -- look for completed stack
        local targetSuit = cards[#cards].Suit
        local startRank = cards[#cards].Rank

        for i = 0, 12 do
            local card = cards[#cards - i]
            local targetRank = Sol.get_rank(startRank, i, true)
            if card.IsFaceDown or card.Suit ~= targetSuit or card.Rank ~= targetRank then return end
        end

        -- move to foundation
        for _, foundation in ipairs(game.Foundation) do
            if foundation.IsEmpty then
                tableau:move_cards(foundation, tableau.CardCount - 12, 13, false)
                tableau:flip_up_top_card()
                return
            end
        end
    end

    for _, tableau in ipairs(game.Tableau) do
        if tableau.CardCount >= 13 then check(tableau) end
    end
end

local spider                        = {
    Info        = {
        Name      = "Spider",
        Family    = "Spider",
        DeckCount = 2
    },
    Stock       = {
        Initial = Sol.Initial.face_down(50)
    },
    Foundation  = {
        Size = 8,
        Pile = { Rule = Sol.Rules.none_none_none }
    },
    Tableau     = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = Sol.Initial.top_face_up(i % 3 == 0 and 6 or 5),
                Layout  = "Column",
                Rule    = Sol.Rules.spider
            }
        end
    },
    on_end_turn = spider_check_and_move,
    do_deal     = function(game)
        for _, tableau in ipairs(game.Tableau) do
            if tableau.IsEmpty then return false end
        end

        return Sol.Ops.Deal.stock_to_tableau(game)
    end,
    on_init     = Sol.Layout.klondike
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
        Rule    = Sol.Rules.spider
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
        Rule    = Sol.Rules.spider
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
        Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.InSeq() }
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
        Rule    = Sol.Rules.spider
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
grounds_for_a_divorce.do_deal       = function(game)
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

local open_spider               = Sol.copy(spider)
open_spider.Info.Name           = "Open Spider"
open_spider.Tableau.Pile        = function(i)
    return {
        Initial = Sol.Initial.face_up(i % 3 == 0 and 6 or 5),
        Layout  = "Column",
        Rule    = Sol.Rules.spider
    }
end

------

local relaxed_spider            = Sol.copy(spider)
relaxed_spider.Info.Name        = "Relaxed Spider"
relaxed_spider.do_deal          = Sol.Ops.Deal.stock_to_tableau

------

local scorpion                  = Sol.copy(spider)
scorpion.Info.Name              = "Scorpion"
scorpion.Info.DeckCount         = 1
scorpion.Stock.Initial          = Sol.Initial.face_down(3)
scorpion.Foundation.Size        = 4
scorpion.Tableau                = {
    Size = 7,
    Pile = function(i)
        return {
            Initial = i < 4 and { false, false, false, true, true, true, true } or Sol.Initial.face_up(7),
            Layout  = "Column",
            Rule    = Sol.Rules.king_downsuit_faceup
        }
    end
}

------

local double_scorpion           = Sol.copy(scorpion)
double_scorpion.Info.Name       = "Double Scorpion"
double_scorpion.Info.DeckCount  = 2
double_scorpion.Stock           = nil
double_scorpion.Foundation.Size = 8
double_scorpion.Tableau         = {
    Size = 10,
    Pile = function(i)
        local initial = Sol.Initial.face_up(i < 4 and 11 or 10)
        if i < 5 then
            initial[1], initial[2], initial[3], initial[4] = false, false, false, false
        end
        return {
            Initial = initial,
            Layout  = "Column",
            Rule    = Sol.Rules.king_downsuit_faceup
        }
    end
}

------

local scorpion_2                = Sol.copy(scorpion)
scorpion_2.Info.Name            = "Scorpion II"
scorpion_2.Tableau.Pile         = function(i)
    return {
        Initial = i < 3 and { false, false, false, true, true, true, true } or Sol.Initial.face_up(7),
        Layout  = "Column",
        Rule    = Sol.Rules.king_downsuit_faceup
    }
end

------

local scorpion_tail             = Sol.copy(scorpion)
scorpion_tail.Info.Name         = "Scorpion Tail"
scorpion_tail.Tableau.Pile      = function(i)
    return {
        Initial = i < 3 and { false, false, false, true, true, true, true } or Sol.Initial.face_up(7),
        Layout  = "Column",
        Rule    = Sol.Rules.king_downac_faceup
    }
end

------

local spider_3x3                = Sol.copy(spider)
spider_3x3.Info.Name            = "Spider 3x3"
spider_3x3.Info.DeckCount       = 3
spider_3x3.Info.DeckSuits       = { "Clubs", "Spades", "Hearts" }
spider_3x3.Stock.Initial        = Sol.Initial.face_down(52)
spider_3x3.Foundation.Size      = 9
spider_3x3.Tableau              = {
    Size = 13,
    Pile = {
        Initial = Sol.Initial.top_face_up(5),
        Layout  = "Column",
        Rule    = Sol.Rules.spider
    }
}

------

local spiderette                = Sol.copy(spider)
spiderette.Info.Name            = "Spiderette"
spiderette.Info.DeckCount       = 1
spiderette.Stock.Initial        = Sol.Initial.face_down(24)
spiderette.Foundation.Size      = 4
spiderette.Tableau              = {
    Size = 7,
    Pile = function(i)
        return {
            Initial = Sol.Initial.top_face_up(i + 1),
            Layout  = "Column",
            Rule    = Sol.Rules.spider
        }
    end
}

------

local baby_spiderette           = Sol.copy(spiderette)
baby_spiderette.Info.Name       = "Baby Spiderette"
baby_spiderette.Tableau.Pile    = function(i)
    return {
        Initial = Sol.Initial.top_face_up(i + 1),
        Layout  = "Column",
        Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.InSeq() }
    }
end

------

local will_o_the_wisp           = Sol.copy(spiderette)
will_o_the_wisp.Info.Name       = "Will o' the Wisp"
will_o_the_wisp.Stock.Initial   = Sol.Initial.face_down(31)
will_o_the_wisp.Tableau.Pile    = function(i)
    return {
        Initial = Sol.Initial.top_face_up(3),
        Layout  = "Column",
        Rule    = Sol.Rules.spider
    }
end

------

local fair_maids                = Sol.copy(will_o_the_wisp)
fair_maids.Info.Name            = "Fair Maids"
fair_maids.Stock.Initial        = Sol.Initial.face_down(24)
fair_maids.Tableau.Pile         = {
    Initial = Sol.Initial.top_face_up(4),
    Layout  = "Column",
    Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.InSeqAlternateColors() }
}


------

local simple_simon               = {
    Info        = {
        Name      = "Simple Simon",
        Family    = "Spider",
        DeckCount = 1
    },
    Foundation  = {
        Size = 4,
        Pile = { Rule = Sol.Rules.none_none_none }
    },
    Tableau     = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 3 and 8 or 10 - i),
                Layout  = "Column",
                Rule    = Sol.Rules.spider
            }
        end
    },
    on_end_turn = spider_check_and_move,
    on_init     = Sol.Layout.klondike
}

------

local simon_jester               = Sol.copy(simple_simon)
simon_jester.Info.Name           = "Simon Jester"
simon_jester.Info.DeckCount      = 2
simon_jester.Foundation.Size     = 8
simon_jester.Tableau             = {
    Size = 14,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 2 and 13 or 14 - i),
            Layout  = "Column",
            Rule    = Sol.Rules.spider
        }
    end
}

------

local york                       = Sol.copy(simple_simon)
york.Info.Name                   = "York"
york.Info.DeckCount              = 2
york.Foundation.Size             = 8
york.Tableau                     = {
    Size = 12,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up((i < 2 or i > 9) and 8 or 9),
            Layout  = "Column",
            Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(true), Move = Sol.Rules.Move.InSeqInSuit() }
        }
    end
}

------

local big_york                   = Sol.copy(york)
big_york.Info.Name               = "Big York"
big_york.Info.DeckCount          = 3
big_york.Foundation.Size         = 12
big_york.Tableau                 = {
    Size = 14,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up((i < 1 or i > 12) and 12 or 11),
            Layout  = "Column",
            Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(true), Move = Sol.Rules.Move.InSeqInSuit() }
        }
    end
}

------

local ten_across_initial <const> = {
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

local ten_across                 = {
    Info        = {
        Name      = "Ten Across",
        Family    = "Spider",
        --Family = "Yukon/Spider"
        DeckCount = 1
    },
    FreeCell    = {
        Size = 2,
        Pile = {
            Initial = Sol.Initial.face_up(1),
            Rule = Sol.Rules.any_none_top
        }
    },
    Foundation  = {
        Size = 4,
        Pile = { Rule = Sol.Rules.none_none_none }
    },
    Tableau     = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = ten_across_initial[i + 1],
                Layout = "Column",
                Rule = Sol.Rules.king_downsuit_faceup
            }
        end
    },
    on_init     = Sol.Layout.free_cell,
    on_end_turn = spider_check_and_move
}

------

local panopticon_initial <const> = {
    { false, true,  true,  true,  true, true },
    { false, false, true,  true,  true, true },
    { false, false, false, true,  true, true },
    { false, false, false, false, true, true },
    { false, false, false, false, true, true },
    { false, false, false, true,  true, true },
    { false, false, true,  true,  true, true },
    { false, true,  true,  true,  true, true }
}

local panopticon                 = Sol.copy(ten_across)
panopticon.Info.Name             = "Panopticon"
panopticon.FreeCell.Size         = 4
panopticon.Tableau               = {
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

local scorpion_head              = {
    Info        = {
        Name      = "Scorpion Head",
        Family    = "Spider",
        DeckCount = 1
    },
    FreeCell    = {
        Size = 4,
        Pile = { Rule = Sol.Rules.any_none_top }
    },
    Foundation  = {
        Size = 4,
        Pile = { Rule = Sol.Rules.none_none_none }
    },
    Tableau     = {
        Size = 7,
        Pile = function(i)
            local initial = {}
            if i > 2 then
                initial = Sol.Initial.face_up(7)
                initial[1], initial[2], initial[3] = false, false, false
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
    on_end_turn = spider_check_and_move,
    on_init     = Sol.Layout.free_cell
}

------

------------------------

Sol.register_game(spider)
Sol.register_game(arachnida)
Sol.register_game(baby_spiderette)
Sol.register_game(big_divorce)
Sol.register_game(big_spider)
Sol.register_game(big_spider_1_suit)
Sol.register_game(big_spider_2_suits)
Sol.register_game(big_york)
Sol.register_game(black_widow)
Sol.register_game(chinese_spider)
Sol.register_game(double_scorpion)
Sol.register_game(fair_maids)
Sol.register_game(grounds_for_a_divorce)
Sol.register_game(open_spider)
Sol.register_game(panopticon)
Sol.register_game(relaxed_spider)
Sol.register_game(scorpion)
Sol.register_game(scorpion_2)
Sol.register_game(scorpion_head)
Sol.register_game(scorpion_tail)
Sol.register_game(spider_1_suit)
Sol.register_game(spider_2_suits)
Sol.register_game(spider_4_decks)
Sol.register_game(spider_3x3)
Sol.register_game(spiderette)
Sol.register_game(simon_jester)
Sol.register_game(simple_simon)
Sol.register_game(ten_across)
Sol.register_game(very_big_divorce)
Sol.register_game(will_o_the_wisp)
Sol.register_game(york)
