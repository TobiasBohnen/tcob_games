-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local function spider_check(game)
    for _, tableau in ipairs(game.Tableau) do
        if tableau.CardCount >= 13 then
            -- look for completed stack
            local targetSuit = tableau.Cards[tableau.CardCount].Suit
            local found = true
            for i = 0, 12 do
                local card = tableau.Cards[tableau.CardCount - i]
                if card.IsFaceDown or card.Suit ~= targetSuit or card.Rank ~= Sol.Ranks[i + 1] then
                    found = false
                    break
                end
            end

            -- move to foundation
            if found then
                for _, foundation in ipairs(game.Foundation) do
                    if foundation.IsEmpty then
                        tableau:move_cards(foundation, tableau.CardCount - 12, 13, false)
                        tableau:flip_up_top_card()
                        break
                    end
                end
            end
            --
        end
    end
end

local spider                      = {
    Info        = {
        Name          = "Spider",
        Family        = "Spider",
        DeckCount     = 2,
        CardDealCount = 10,
        Redeals       = 0
    },
    Stock       = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(50)
    },
    Foundation  = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Rule = Sol.Rules.none_none_none
            }
        end
    },
    Tableau     = {
        Size = 10,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial  = Sol.Initial.top_face_up(i % 3 == 0 and 6 or 5),
                Layout   = "Column",
                Rule     = Sol.Rules.spider
            }
        end
    },
    on_end_turn = spider_check,
    do_deal     = function(game)
        for _, tableau in ipairs(game.Tableau) do
            if tableau.IsEmpty then return false end
        end

        return Sol.Ops.Deal.to_group(game.Stock[1], game.Tableau, false)
    end
}

------

local spider_1_suit               = Sol.copy(spider)
spider_1_suit.Info.Name           = "Spider (1 suit)"
spider_1_suit.Info.DeckCount      = 8
spider_1_suit.Info.DeckSuits      = { "Clubs" }

------

local spider_2_suits              = Sol.copy(spider)
spider_2_suits.Info.Name          = "Spider (2 suits)"
spider_2_suits.Info.DeckCount     = 4
spider_2_suits.Info.DeckSuits     = { "Clubs", "Hearts" }

------

local spider_4_decks              = Sol.copy(spider)
spider_4_decks.Info.Name          = "Spider (4 decks)"
spider_4_decks.Info.DeckCount     = 4
spider_4_decks.Info.CardDealCount = 13
spider_4_decks.Stock.Initial      = Sol.Initial.face_down(130)
spider_4_decks.Foundation.Size    = 16
spider_4_decks.Tableau            = {
    Size = 13,
    Pile = function(i)
        return {
            Position = { x = i + 0.5, y = 1 },
            Initial  = Sol.Initial.top_face_up(6),
            Layout   = "Column",
            Rule     = Sol.Rules.spider
        }
    end
}

------

local big_spider                  = Sol.copy(spider)
big_spider.Info.Name              = "Big Spider"
big_spider.Info.DeckCount         = 3
big_spider.Info.CardDealCount     = 13
big_spider.Stock.Initial          = Sol.Initial.face_down(78)
big_spider.Foundation.Size        = 12
big_spider.Tableau                = {
    Size = 13,
    Pile = function(i)
        return {
            Position = { x = i + 0.5, y = 1 },
            Initial  = Sol.Initial.top_face_up(6),
            Layout   = "Column",
            Rule     = Sol.Rules.spider
        }
    end
}

------

local big_spider_1_suit           = Sol.copy(big_spider)
big_spider_1_suit.Info.Name       = "Big Spider (1 suit)"
big_spider_1_suit.Info.DeckCount  = 12
big_spider_1_suit.Info.DeckSuits  = { "Clubs" }

------

local big_spider_2_suits          = Sol.copy(big_spider)
big_spider_2_suits.Info.Name      = "Big Spider (2 suits)"
big_spider_2_suits.Info.DeckCount = 6
big_spider_2_suits.Info.DeckSuits = { "Clubs", "Hearts" }

------

local arachnida                   = Sol.copy(spider)
arachnida.Info.Name               = "Arachnida"
arachnida.Foundation              = {
    Position = { x = 11, y = 0 },
    Rule = Sol.Rules.none_none_none
}
arachnida.Tableau.Pile            = function(i)
    return {
        Position = { x = i + 1, y = 0 },
        Initial  = Sol.Initial.top_face_up(i < 4 and 6 or 5),
        Layout   = "Column",
        Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.InRankOrDownByRank(), Move = Sol.Rules.Move.InSeqInSuitOrSameRank() }
    }
end

------

local black_widow                 = Sol.copy(spider)
black_widow.Info.Name             = "Black Widow"
black_widow.Tableau.Pile          = function(i)
    return {
        Position = { x = i, y = 1 },
        Initial  = Sol.Initial.top_face_up(i % 3 == 0 and 6 or 5),
        Layout   = "Column",
        Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.InSeq() }
    }
end

------

local chinese_spider              = Sol.copy(spider)
chinese_spider.Info.Name          = "Chinese Spider"
chinese_spider.Info.DeckCount     = 4
chinese_spider.Info.DeckSuits     = { "Clubs", "Spades", "Hearts" }
chinese_spider.Info.CardDealCount = 13
chinese_spider.Stock.Initial      = Sol.Initial.face_down(84)
chinese_spider.Foundation.Size    = 12
chinese_spider.Tableau            = {
    Size = 12,
    Pile = function(i)
        return {
            Position = { x = i + 0.5, y = 1 },
            Initial  = Sol.Initial.top_face_up(6),
            Layout   = "Column",
            Rule     = Sol.Rules.spider
        }
    end
}

------

local open_spider                 = Sol.copy(spider)
open_spider.Info.Name             = "Open Spider"
open_spider.Tableau.Pile          = function(i)
    return {
        Position = { x = i, y = 1 },
        Initial  = Sol.Initial.face_up(i % 3 == 0 and 6 or 5),
        Layout   = "Column",
        Rule     = Sol.Rules.spider
    }
end

------

local relaxed_spider              = Sol.copy(spider)
relaxed_spider.Info.Name          = "Relaxed Spider"
relaxed_spider.do_deal            = function(game)
    return Sol.Ops.Deal.to_group(game.Stock[1], game.Tableau, false)
end

------

local spider_3x3                  = Sol.copy(spider)
spider_3x3.Info.Name              = "Spider 3x3"
spider_3x3.Info.DeckCount         = 3
spider_3x3.Info.DeckSuits         = { "Clubs", "Spades", "Hearts" }
spider_3x3.Info.CardDealCount     = 9
spider_3x3.Stock.Initial          = Sol.Initial.face_down(52)
spider_3x3.Foundation.Size        = 9
spider_3x3.Tableau                = {
    Size = 13,
    Pile = function(i)
        return {
            Position = { x = i + 0.5, y = 1 },
            Initial  = Sol.Initial.top_face_up(5),
            Layout   = "Column",
            Rule     = Sol.Rules.spider
        }
    end
}

------

local spiderette                  = Sol.copy(spider)
spiderette.Info.Name              = "Spiderette"
spiderette.Info.DeckCount         = 1
spiderette.Info.CardDealCount     = 7
spiderette.Stock.Initial          = Sol.Initial.face_down(24)
spiderette.Foundation.Size        = 4
spiderette.Tableau                = {
    Size = 7,
    Pile = function(i)
        return {
            Position = { x = i, y = 1 },
            Initial  = Sol.Initial.top_face_up(i + 1),
            Layout   = "Column",
            Rule     = Sol.Rules.spider
        }
    end
}

------

local baby_spiderette             = Sol.copy(spiderette)
baby_spiderette.Info.Name         = "Baby Spiderette"
baby_spiderette.Tableau.Pile      = function(i)
    return {
        Position = { x = i, y = 1 },
        Initial  = Sol.Initial.top_face_up(i + 1),
        Layout   = "Column",
        Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.InSeq() }
    }
end

------

local will_o_the_wisp             = Sol.copy(spiderette)
will_o_the_wisp.Info.Name         = "Will o' the Wisp"
will_o_the_wisp.Stock.Initial     = Sol.Initial.face_down(31)
will_o_the_wisp.Tableau.Pile      = function(i)
    return {
        Position = { x = i, y = 1 },
        Initial  = Sol.Initial.top_face_up(3),
        Layout   = "Column",
        Rule     = Sol.Rules.spider
    }
end

------

local fair_maids                  = Sol.copy(will_o_the_wisp)
fair_maids.Info.Name              = "Fair Maids"
fair_maids.Stock.Initial          = Sol.Initial.face_down(24)
fair_maids.Tableau.Pile           = function(i)
    return {
        Position = { x = i, y = 1 },
        Initial  = Sol.Initial.top_face_up(4),
        Layout   = "Column",
        Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.InSeqAlternateColors() }
    }
end

------

------------------------

Sol.register_game(spider)
Sol.register_game(arachnida)
Sol.register_game(baby_spiderette)
Sol.register_game(big_spider)
Sol.register_game(big_spider_1_suit)
Sol.register_game(big_spider_2_suits)
Sol.register_game(black_widow)
Sol.register_game(chinese_spider)
Sol.register_game(fair_maids)
Sol.register_game(open_spider)
Sol.register_game(relaxed_spider)
Sol.register_game(spider_1_suit)
Sol.register_game(spider_2_suits)
Sol.register_game(spider_4_decks)
Sol.register_game(spider_3x3)
Sol.register_game(spiderette)
Sol.register_game(will_o_the_wisp)
