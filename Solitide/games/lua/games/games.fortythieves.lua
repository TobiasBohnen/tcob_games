-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local forty_thieves = {
    Info = {
        Name      = "Forty Thieves",
        Family    = "FortyThieves",
        DeckCount = 2
    },
    Stock = { Initial = Sol.Initial.face_down(64) },
    Waste = {},
    Foundation = {
        Size = 8,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau = {
        Size = 10,
        Pile = {
            Initial = Sol.Initial.face_up(4),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.any_downsuit_top
        }
    },
    deal = Sol.Ops.Deal.stock_to_waste,
    on_init = Sol.Layout.klondike
}


------

local sixty_thieves                = Sol.copy(forty_thieves)
sixty_thieves.Info.Name            = "Sixty Thieves"
sixty_thieves.Info.DeckCount       = 3
sixty_thieves.Stock.Initial        = Sol.Initial.face_down(96)
sixty_thieves.Foundation.Size      = 12
sixty_thieves.Tableau.Size         = 12
sixty_thieves.Tableau.Pile.Initial = Sol.Initial.face_up(5)


------

local eighty_thieves                = Sol.copy(forty_thieves)
eighty_thieves.Info.Name            = "Eighty Thieves"
eighty_thieves.Info.DeckCount       = 4
eighty_thieves.Stock.Initial        = Sol.Initial.face_down(128)
eighty_thieves.Foundation.Size      = 16
eighty_thieves.Tableau.Pile.Initial = Sol.Initial.face_up(8)


------

local busy_aces         = Sol.copy(forty_thieves)
busy_aces.Info.Name     = "Busy Aces"
busy_aces.Stock.Initial = Sol.Initial.face_down(92)
busy_aces.Tableau       = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downsuit_top
    }
}


------

local carre_napoleon             = Sol.copy(forty_thieves)
carre_napoleon.Info.Name         = "Carre Napoleon"
carre_napoleon.Stock.Initial     = Sol.Initial.face_down(48)
carre_napoleon.Tableau           = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.face_up(4),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.king_downsuit_top
    }
}
carre_napoleon.on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation
carre_napoleon.on_shuffle        = Sol.Ops.Shuffle.play_to_foundation


------

local corona         = Sol.copy(forty_thieves)
corona.Info.Name     = "Corona"
corona.Stock.Initial = Sol.Initial.face_down(68)
corona.Tableau       = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.face_up(3),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downsuit_top
    }
}
corona.on_end_turn   = Sol.Ops.Deal.waste_or_stock_to_empty_tableau


------

local courtyard         = Sol.copy(forty_thieves)
courtyard.Info.Name     = "Courtyard"
courtyard.Stock.Initial = Sol.Initial.face_down(92)
courtyard.Tableau       = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downsuit_inseq
    }
}
courtyard.on_end_turn   = Sol.Ops.Deal.waste_or_stock_to_empty_tableau


------

local big_courtyard           = Sol.copy(courtyard)
big_courtyard.Info.Name       = "Big Courtyard"
big_courtyard.Info.DeckCount  = 3
big_courtyard.Stock.Initial   = Sol.Initial.face_down(144)
big_courtyard.Foundation.Size = 12


------

local delivery           = Sol.copy(forty_thieves)
delivery.Info.Name       = "Delivery"
delivery.Info.DeckCount  = 4
delivery.Stock.Initial   = Sol.Initial.face_down(172)
delivery.Foundation.Size = 16
delivery.Tableau         = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.face_up(3),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downsuit_top
    }
}


------

local dieppe            = Sol.copy(forty_thieves)
dieppe.Info.Name        = "Dieppe"
dieppe.Stock.Initial    = Sol.Initial.face_down(80)
dieppe.Tableau          = {
    Size = 8,
    Pile = {
        Initial = Sol.Initial.face_up(3),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downrank_top
    }
}
dieppe.on_shuffle       = Sol.Ops.Shuffle.play_to_foundation
dieppe.on_after_shuffle = function(game)
    -- refill Tableau from Stock back to three cards
    local stock = game.Stock[1]
    local stockCards = stock.Cards
    local idx = #stockCards
    for _, tab in ipairs(game.Tableau) do
        while tab.CardCount < 3 do
            stock:move_cards(tab, idx, 1, false)
            idx = idx - 1
        end
        tab:flip_up_cards()
    end

    return false
end


------

local diplomat         = Sol.copy(forty_thieves)
diplomat.Info.Name     = "Diplomat"
diplomat.Stock.Initial = Sol.Initial.face_down(72)
diplomat.Tableau       = {
    Size = 8,
    Pile = {
        Initial = Sol.Initial.face_up(4),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downrank_top
    }
}


------

local famous_fifty                = Sol.copy(forty_thieves)
famous_fifty.Info.Name            = "Famous Fifty"
famous_fifty.Stock.Initial        = Sol.Initial.face_down(54)
famous_fifty.Tableau.Pile.Initial = Sol.Initial.face_up(5)


------
local function following_build(interval)
    return function(_, target, card)
        if target.Suit == "Hearts" and card.Suit ~= "Clubs" then return false end
        if target.Suit == "Clubs" and card.Suit ~= "Diamonds" then return false end
        if target.Suit == "Diamonds" and card.Suit ~= "Spades" then return false end
        if target.Suit == "Spades" and card.Suit ~= "Hearts" then return false end
        return Sol.get_rank(target.Rank, interval, false) == card.Rank
    end
end

local following          = Sol.copy(forty_thieves)
following.Info.Name      = "Following"
following.Info.DeckCount = 1
following.Info.Redeals   = 1
following.Stock.Initial  = Sol.Initial.face_down(46)
following.Foundation     = {
    Size = 4,
    Pile = {
        Rule = {
            Base = Sol.Rules.Base.Ace(),
            Build = {
                Hint = "Up by rank (H, C, D, S)",
                Func = following_build(1)
            },
            Move = Sol.Rules.Move.Top()
        }
    }
}
following.Tableau        = {
    Size = 6,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = {
            Base = Sol.Rules.Base.Any(),
            Build = {
                Hint = "Down by rank (H, C, D, S)",
                Func = following_build(-1)
            },
            Move = Sol.Rules.Move.InSeq()
        }
    }
}


------

local foothold     = Sol.copy(forty_thieves)
foothold.Info.Name = "Foothold"
foothold.Tableau   = {
    Size = 8,
    Pile = {
        Initial = Sol.Initial.face_up(5),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_updownac_top
    }
}


------

local forty_and_eight         = Sol.copy(forty_thieves)
forty_and_eight.Info.Name     = "Forty and Eight"
forty_and_eight.Info.Redeal   = 1
forty_and_eight.Stock.Initial = Sol.Initial.face_down(72)
forty_and_eight.Tableau.Size  = 8
forty_and_eight.redeal        = Sol.Ops.Redeal.waste_to_stock


------

local indian                = Sol.copy(forty_thieves)
indian.Info.Name            = "Indian"
indian.Stock.Initial        = Sol.Initial.face_down(74)
indian.Tableau.Pile.Initial = { false, true, true }
indian.Tableau.Pile.Rule    = Sol.Rules.any_downabos_top


------

local indian_patience       = Sol.copy(indian)
indian_patience.Info.Name   = "Indian Patience"
indian_patience.can_play    = function(game, targetPile, targetCardIndex, card, numCards)
    if numCards > 1 then return false end

    if not game.Stock.IsEmpty then
        -- no building on single-card tableau
        if targetPile.Type == Sol.Pile.Type.Tableau and targetPile.CardCount == 1 then
            return false
        end

        -- single-card tableau only movable to foundation
        local srcPile = game:find_pile(card)
        if srcPile.Type == Sol.Pile.Type.Tableau and srcPile.CardCount == 1 then
            if targetPile.Type ~= Sol.Pile.Type.Foundation then return false end
        end
    end

    return game:can_play(targetPile, targetCardIndex, card, numCards)
end
indian_patience.on_end_turn = function(game)
    local tableau = game.Tableau
    for _, tab in ipairs(tableau) do -- deal 3 cards to empty tableau
        if tab.IsEmpty and Sol.Ops.Deal.to_pile(game.Stock[1], tab, 3) then
            tab:flip_cards(indian_initial)
        end
    end
end


------

local josephine             = Sol.copy(forty_thieves)
josephine.Info.Name         = "Josephine"
josephine.Tableau.Pile.Rule = Sol.Rules.any_downsuit_inseq


------

local lady_palk             = Sol.copy(forty_thieves)
lady_palk.Info.Name         = "Lady Palk"
lady_palk.Stock.Initial     = Sol.Initial.face_down(72)
lady_palk.Tableau.Size      = 8
lady_palk.Tableau.Pile.Rule = Sol.Rules.any_downrank_inseq


------

local little_forty             = Sol.copy(forty_thieves)
little_forty.Info.Name         = "Little Forty"
little_forty.Info.Redeals      = 3
little_forty.Tableau.Pile.Rule = Sol.Rules.spider_tableau
little_forty.on_end_turn       = Sol.Ops.Deal.waste_or_stock_to_empty_tableau
little_forty.deal              = Sol.Ops.Deal.stock_to_waste_by_3


------

local little_napoleon         = Sol.copy(forty_thieves)
little_napoleon.Info.Name     = "Little Napoleon"
little_napoleon.Stock.Initial = Sol.Initial.face_down(72)
little_napoleon.Tableau       = {
    Size = 8,
    Pile = {
        Initial = Sol.Initial.top_face_up(4),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.spider_tableau
    }
}


------

local malmaison           = Sol.copy(forty_thieves)
malmaison.Info.Name       = "Malmaison"
malmaison.Info.DeckCount  = 4
malmaison.Stock.Initial   = Sol.Initial.face_down(128)
malmaison.Foundation.Size = 16
malmaison.Tableau.Pile    = {
    Initial = Sol.Initial.face_up(8),
    Layout  = Sol.Pile.Layout.Column,
    Rule    = Sol.Rules.any_downsuit_inseq
}


------

local maria         = Sol.copy(forty_thieves)
maria.Info.Name     = "Maria"
maria.Stock.Initial = Sol.Initial.face_down(68)
maria.Tableau       = {
    Size = 9,
    Pile = {
        Initial = Sol.Initial.face_up(4),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downac_top
    }
}


------

local marie_rose           = Sol.copy(forty_thieves)
marie_rose.Info.Name       = "Marie Rose"
marie_rose.Info.DeckCount  = 3
marie_rose.Stock.Initial   = Sol.Initial.face_down(96)
marie_rose.Foundation.Size = 12
marie_rose.Tableau         = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.face_up(5),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downsuit_inseq
    }
}


------

local mc_clellan         = Sol.copy(forty_thieves)
mc_clellan.Info.Name     = "McClellan"
mc_clellan.Stock.Initial = Sol.Initial.face_down(72)
mc_clellan.Tableau       = {
    Size = 8,
    Pile = {
        Initial = Sol.Initial.top_face_up(4),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downsuit_inseq
    }
}


------

local midshipman         = Sol.copy(forty_thieves)
midshipman.Info.Name     = "Midshipman"
midshipman.Stock.Initial = Sol.Initial.face_down(68)
midshipman.Tableau       = {
    Size = 9,
    Pile = {
        Initial = { false, false, true, true },
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downabos_top
    }
}


------

local mumbai           = Sol.copy(forty_thieves)
mumbai.Info.Name       = "Mumbai"
mumbai.Info.DeckCount  = 3
mumbai.Stock.Initial   = Sol.Initial.face_down(117)
mumbai.Foundation.Size = 12
mumbai.Tableau         = {
    Size = 13,
    Pile = {
        Initial = indian_initial,
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downabos_top
    }
}


------

local napoleons_exile             = Sol.copy(forty_thieves)
napoleons_exile.Info.Name         = "Napoleon's Exile"
napoleons_exile.Tableau.Pile.Rule = Sol.Rules.any_downrank_top


------

local napoleons_shoulder         = Sol.copy(forty_thieves)
napoleons_shoulder.Info.Name     = "Napoleon's Shoulder"
napoleons_shoulder.Stock.Initial = Sol.Initial.face_down(56)
napoleons_shoulder.Tableau       = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.face_up(4),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downrank_top
    }
}
napoleons_shoulder.can_play      = function(game, targetPile, targetCardIndex, card, numCards)
    -- empty tableau can only be filled from waste
    if targetPile.IsEmpty and targetPile.Type == Sol.Pile.Type.Tableau then
        return game:find_pile(card).Type == Sol.Pile.Type.Waste
    end

    return game:can_play(targetPile, targetCardIndex, card, numCards)
end


------

local napoleons_square         = Sol.copy(forty_thieves)
napoleons_square.Info.Name     = "Napoleon's Square"
napoleons_square.Stock.Initial = Sol.Initial.face_down(56)
napoleons_square.Tableau       = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.face_up(4),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downsuit_inseq
    }
}


------

local pluto                = Sol.copy(forty_thieves)
pluto.Info.Name            = "Pluto"
pluto.Stock.Initial        = Sol.Initial.face_down(92)
pluto.Foundation.Pile.Rule = { Base = Sol.Rules.Base.Ranks({ "Two" }), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.Top() }
pluto.Tableau              = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownInSuit(true), Move = Sol.Rules.Move.Top() }
    }
}


------

local deuces             = Sol.copy(pluto)
deuces.Info.Name         = "Deuces"
deuces.Stock.Initial     = Sol.Initial.face_down(86)
deuces.Tableau.Size      = 10
deuces.on_before_shuffle = function(game, card)
    if card.Rank == "Two" then
        return game.PlaceTop(card, game.Foundation, true)
    end
    return false
end


------

local red_and_black                = Sol.copy(forty_thieves)
red_and_black.Info.Name            = "Red and Black"
red_and_black.Stock.Initial        = Sol.Initial.face_down(88)
red_and_black.Foundation.Pile.Rule = Sol.Rules.ace_upac_top
red_and_black.Tableau              = {
    Size = 8,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downac_inseq
    }
}
red_and_black.on_before_shuffle    = Sol.Ops.Shuffle.ace_to_foundation


------

local rows_of_four         = Sol.copy(forty_thieves)
rows_of_four.Info.Name     = "Rows of Four"
rows_of_four.Info.Redeals  = 2
rows_of_four.Stock.Initial = Sol.Initial.face_down(72)
rows_of_four.Tableau.Size  = 8
rows_of_four.Tableau.Rule  = Sol.Rules.any_downrank_top


------

local san_juan_hill             = Sol.copy(forty_thieves)
san_juan_hill.Info.Name         = "San Juan Hill"
san_juan_hill.Stock.Initial     = Sol.Initial.face_down(56)
san_juan_hill.on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation


------

local sanibel                = Sol.copy(forty_thieves)
sanibel.Info.Name            = "Sanibel"
sanibel.Stock.Initial        = Sol.Initial.face_down(4)
sanibel.Foundation.Pile.Rule = Sol.Rules.ace_upsuit_none
sanibel.Tableau.Pile         = {
    Initial = { false, false, false, true, true, true, true, true, true, true },
    Layout  = Sol.Pile.Layout.Column,
    Rule    = Sol.Rules.any_downac_faceup
}


------

local thirty_nine_steps                = Sol.copy(forty_thieves)
thirty_nine_steps.Info.Name            = "Thirty-Nine Steps"
thirty_nine_steps.Stock.Initial        = Sol.Initial.face_down(65)
thirty_nine_steps.Tableau.Size         = 13
thirty_nine_steps.Tableau.Pile.Initial = Sol.Initial.face_up(3)


------

local triple_line         = Sol.copy(forty_thieves)
triple_line.Info.Name     = "Triple Line"
triple_line.Info.Redeals  = 1
triple_line.Stock.Initial = Sol.Initial.face_down(68)
triple_line.Tableau       = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.face_up(3),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downac_inseq
    }
}
triple_line.on_end_turn   = Sol.Ops.Deal.waste_or_stock_to_empty_tableau


------

local waterloo             = Sol.copy(forty_thieves)
waterloo.Info.Name         = "Waterloo"
waterloo.Stock.Initial     = Sol.Initial.face_down(90)
waterloo.Tableau           = {
    Size = 6,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.spider_tableau
    }
}
waterloo.on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation


------

local zebra             = Sol.copy(forty_thieves)
zebra.Info.Name         = "Zebra"
zebra.Info.Redeal       = 1
zebra.Stock.Initial     = Sol.Initial.face_down(88)
zebra.Tableau           = {
    Size = 8,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downac_top
    }
}
zebra.on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation
zebra.on_end_turn       = Sol.Ops.Deal.waste_or_stock_to_empty_tableau


------
------

local limited                = Sol.copy(forty_thieves)
limited.Info.Name            = "Limited"
limited.Stock.Initial        = Sol.Initial.face_down(68)
limited.Tableau.Size         = 12
limited.Tableau.Pile.Initial = Sol.Initial.face_up(3)


------

local carnation           = Sol.copy(limited)
carnation.Info.Name       = "Carnation"
carnation.Info.DeckCount  = 4
carnation.Stock.Initial   = Sol.Initial.face_down(160)
carnation.Foundation.Size = 16
carnation.Tableau.Size    = 16


------

local express           = Sol.copy(limited)
express.Info.Name       = "Express"
express.Info.DeckCount  = 3
express.Stock.Initial   = Sol.Initial.face_down(114)
express.Foundation.Size = 12
express.Tableau.Size    = 14


------
------

local double_rail         = Sol.copy(forty_thieves)
double_rail.Info.Name     = "Double Rail"
double_rail.Stock.Initial = Sol.Initial.face_down(99)
double_rail.Tableau       = {
    Size = 5,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downrank_inseq
    }
}


------

local single_rail           = Sol.copy(double_rail)
single_rail.Info.Name       = "Single Rail"
single_rail.Info.DeckCount  = 1
single_rail.Stock.Initial   = Sol.Initial.face_down(48)
single_rail.Foundation.Size = 4
single_rail.Tableau.Size    = 4


------

local triple_rail           = Sol.copy(double_rail)
triple_rail.Info.Name       = "Triple Rail"
triple_rail.Info.DeckCount  = 3
triple_rail.Stock.Initial   = Sol.Initial.face_down(147)
triple_rail.Foundation.Size = 12
triple_rail.Tableau.Size    = 9


------

local final_battle         = Sol.copy(double_rail)
final_battle.Info.Name     = "Final Battle"
final_battle.Stock.Initial = Sol.Initial.face_down(98)
final_battle.Tableau.Size  = 6


------
------

local waning_moon         = Sol.copy(forty_thieves)
waning_moon.Info.Name     = "Waning Moon"
waning_moon.Stock.Initial = Sol.Initial.face_down(52)
waning_moon.Tableau.Size  = 13


------

local lucas             = Sol.copy(waning_moon)
lucas.Info.Name         = "Lucas"
lucas.Tableau.Pile.Rule = Sol.Rules.any_downsuit_inseq


------
------

local number_ten        = Sol.copy(forty_thieves)
number_ten.Info.Name    = "Number Ten"
number_ten.Tableau.Pile = {
    Initial = { false, false, true, true },
    Layout  = Sol.Pile.Layout.Column,
    Rule    = Sol.Rules.any_downac_inseq
}


------

local rank_and_file                = Sol.copy(number_ten)
rank_and_file.Info.Name            = "Rank and File"
rank_and_file.Tableau.Pile.Initial = Sol.Initial.top_face_up(4)


------

local number_twelve           = Sol.copy(number_ten)
number_twelve.Info.Name       = "Number Twelve"
number_twelve.Info.DeckCount  = 3
number_twelve.Stock.Initial   = Sol.Initial.face_down(108)
number_twelve.Foundation.Size = 12
number_twelve.Tableau.Size    = 12


------
------

local streets             = Sol.copy(forty_thieves)
streets.Info.Name         = "Streets"
streets.Tableau.Pile.Rule = Sol.Rules.any_downac_top


------

local big_streets           = Sol.copy(streets)
big_streets.Info.Name       = "Big Streets"
big_streets.Info.DeckCount  = 3
big_streets.Stock.Initial   = Sol.Initial.face_down(108)
big_streets.Foundation.Size = 12
big_streets.Tableau.Size    = 12


------

local emperor                = Sol.copy(streets)
emperor.Info.Name            = "Emperor"
emperor.Tableau.Pile.Initial = Sol.Initial.top_face_up(4)


------

local roosevelt         = Sol.copy(streets)
roosevelt.Info.Name     = "Roosevelt"
roosevelt.Stock.Initial = Sol.Initial.face_down(76)
roosevelt.Tableau.Size  = 7


------
------

local alternation         = Sol.copy(forty_thieves)
alternation.Info.Name     = "Alternation"
alternation.Stock.Initial = Sol.Initial.face_down(55)
alternation.Tableau       = {
    Size = 7,
    Pile = {
        Initial = Sol.Initial.alternate(7, true),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.king_downac_inseq
    }
}


------

local breakwater             = Sol.copy(alternation)
breakwater.Info.Name         = "Breakwater"
breakwater.Tableau.Pile.Rule = Sol.Rules.king_downrank_inseq


------

local interchange             = Sol.copy(alternation)
interchange.Info.Name         = "Interchange"
interchange.Tableau.Pile.Rule = Sol.Rules.king_downsuit_inseq


------

local triple_interchange                = Sol.copy(alternation)
triple_interchange.Info.Name            = "Triple Interchange"
triple_interchange.Info.DeckCount       = 3
triple_interchange.Stock.Initial        = Sol.Initial.face_down(75)
triple_interchange.Foundation.Size      = 12
triple_interchange.Tableau.Size         = 9
triple_interchange.Tableau.Pile.Initial = Sol.Initial.alternate(9, true)


------

local unlimited        = Sol.copy(interchange)
unlimited.Info.Name    = "Unlimited"
unlimited.Info.Redeals = -1


------

local forty_nine                = Sol.copy(interchange)
forty_nine.Info.Name            = "Forty Nine"
forty_nine.Tableau.Pile.Initial = Sol.Initial.face_up(7)


------
------

local floradora = {
    Info       = {
        Name      = "Floradora",
        Family    = "FortyThieves",
        DeckCount = 2
    },
    Stock      = { Initial = Sol.Initial.face_down(68) },
    Waste      = {},
    Foundation = {
        Size = 9,
        Pile = function(i)
            if i > 0 then
                return {
                    Rule = { Base = Sol.Rules.Base.Ace(), Build = Sol.Rules.Build.UpInSuit(), Move = Sol.Rules.Move.Top(), Limit = 12 }
                }
            else
                return {
                    Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.Top(), Limit = 8 }
                }
            end
        end
    },
    Tableau    = {
        Size = 6,
        Pile = {
            Initial = Sol.Initial.face_up(6),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.any_downrank_top
        }
    },
    deal       = Sol.Ops.Deal.stock_to_waste,
    on_init    = Sol.Layout.klondike
}


------

local octagon_foundation_pos <const> = {
    --[[     --]] --[[     --]] { 2.5, 0.0 }, { 3.5, 0.0 }, --[[     --]] --[[     --]]
    { 0.0, 1.5 }, { 1.0, 1.5 }, --[[     --]] --[[     --]] { 5.0, 1.5 }, { 6.0, 1.5 },
    --[[     --]] --[[     --]] { 2.5, 3.0 }, { 3.5, 3.0 }, --[[     --]] --[[     --]]
}

local octagon_tableau_pos <const>    = {
    { 0.25, 0.25 }, --[[ --]] --[[ --]] --[[ --]] --[[ --]] { 4.75, 0.25 },
    --[[ --]] --[[ --]] --[[ --]] --[[ --]] --[[ --]] --[[ --]]
    { 0.25, 2.75 }, --[[ --]] --[[ --]] --[[ --]] --[[ --]] { 4.75, 2.75 },
}

local octagon                        = {
    Info = {
        Name      = "Octagon",
        Family    = "FortyThieves",
        DeckCount = 2,
        Redeals   = 3
    },
    Stock = {
        Position = { x = 2.5, y = 1.5 },
        Initial  = Sol.Initial.face_down(76)
    },
    Waste = {
        Position = { x = 3.5, y = 1.5 }
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = octagon_foundation_pos[i + 1][1], y = octagon_foundation_pos[i + 1][2] },
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = octagon_tableau_pos[i + 1][1], y = octagon_tableau_pos[i + 1][2] },
                Initial  = Sol.Initial.face_up(5),
                Layout   = Sol.Pile.Layout.Row,
                Rule     = Sol.Rules.any_downsuit_top
            }
        end
    },
    on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation,
    deal = Sol.Ops.Deal.stock_to_waste,
    redeal = Sol.Ops.Redeal.waste_to_stock
}


------

local octave = {
    Info = {
        Name      = "Octave",
        Family    = "FortyThieves",
        DeckCount = 2,
        Redeals   = 1
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(72)
    },
    Waste = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 1, y = 0 }
            }
        end
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 1, y = 1 },
                Rule = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 1, y = 2 },
                Initial  = Sol.Initial.face_up(3),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.any_downac_top
            }
        end
    },
    on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation,
    deal = function(game)
        -- first round: deal to first waste
        return game.RedealsLeft == 1 and Sol.Ops.Deal.stock_to_waste(game)
    end,
    redeal = function(game)
        -- second round: deal to all waste piles and auto refill in on_end_turn
        return Sol.Ops.Redeal.waste_to_stock(game) and Sol.Ops.Deal.to_group(game.Stock[1], game.Waste, Sol.DealMode.IfEmpty)
    end,
    on_end_turn = function(game)
        if game.RedealsLeft == 0 then
            Sol.Ops.Deal.to_group(game.Stock[1], game.Waste, Sol.DealMode.IfEmpty)
        end
    end
}


------

local quadrangle = {
    Info        = {
        Name      = "Quadrangle",
        Family    = "FortyThieves",
        DeckCount = 2
    },
    Stock       = { Initial = Sol.Initial.face_down(67) },
    Waste       = {},
    Foundation  = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i == 0 and 1 or 0),
                Rule    = Sol.Rules.ff_upsuit_none
            }
        end
    },
    Tableau     = {
        Size = 12,
        Pile = {
            Initial = Sol.Initial.face_up(3),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownInSuit(true), Move = Sol.Rules.Move.Top() }
        }
    },
    deal        = Sol.Ops.Deal.stock_to_waste,
    on_end_turn = function(game)
        if not Sol.Ops.Deal.to_group(game.Waste[1], game.Tableau, Sol.DealMode.IfEmpty) then
            Sol.Ops.Deal.to_group(game.Stock[1], game.Tableau, Sol.DealMode.IfEmpty)
        end
    end,
    on_init     = Sol.Layout.klondike
}


------

local twin_queens = {
    Info = {
        Name      = "Twin Queens",
        Family    = "FortyThieves",
        DeckCount = 2,
        Redeals   = 1
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial  = Sol.Initial.face_down(96)
    },
    Waste = {
        Position = { x = 1, y = 0 }
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = (i % 2) + 4, y = i // 2 },
                Rule     = { Base = Sol.Rules.Base.Ranks({ "King" }), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    Tableau = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = (i % 2) * 3 + 3, y = i // 2 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = Sol.Rules.any_downsuit_top
            }
        end
    },
    redeal = Sol.Ops.Redeal.waste_to_stock,
    deal = Sol.Ops.Deal.stock_to_waste,
    on_end_turn = function(game)
        if not Sol.Ops.Deal.to_group(game.Waste[1], game.Tableau, Sol.DealMode.IfEmpty) then
            Sol.Ops.Deal.to_group(game.Stock[1], game.Tableau, Sol.DealMode.IfEmpty)
        end
    end
}


------

------------------------

Sol.register_game(forty_thieves)
Sol.register_game(alternation)
Sol.register_game(big_courtyard)
Sol.register_game(big_streets)
Sol.register_game(breakwater)
Sol.register_game(busy_aces)
Sol.register_game(carnation)
Sol.register_game(carre_napoleon)
Sol.register_game(corona)
Sol.register_game(courtyard)
Sol.register_game(dieppe)
Sol.register_game(delivery)
Sol.register_game(deuces)
Sol.register_game(diplomat)
Sol.register_game(double_rail)
Sol.register_game(eighty_thieves)
Sol.register_game(emperor)
Sol.register_game(express)
Sol.register_game(famous_fifty)
Sol.register_game(final_battle)
Sol.register_game(floradora)
Sol.register_game(following)
Sol.register_game(foothold)
Sol.register_game(forty_and_eight)
Sol.register_game(forty_nine)
Sol.register_game(indian)
Sol.register_game(indian_patience)
Sol.register_game(interchange)
Sol.register_game(josephine)
Sol.register_game(lady_palk)
Sol.register_game(limited)
Sol.register_game(little_forty)
Sol.register_game(little_napoleon)
Sol.register_game(lucas)
Sol.register_game(malmaison)
Sol.register_game(maria)
Sol.register_game(marie_rose)
Sol.register_game(mc_clellan)
Sol.register_game(midshipman)
Sol.register_game(mumbai)
Sol.register_game(napoleons_exile)
Sol.register_game(napoleons_shoulder)
Sol.register_game(napoleons_square)
Sol.register_game(number_ten)
Sol.register_game(number_twelve)
Sol.register_game(octagon)
Sol.register_game(octave)
Sol.register_game(pluto)
Sol.register_game(quadrangle)
Sol.register_game(rank_and_file)
Sol.register_game(red_and_black)
Sol.register_game(roosevelt)
Sol.register_game(rows_of_four)
Sol.register_game(san_juan_hill)
Sol.register_game(sanibel)
Sol.register_game(single_rail)
Sol.register_game(sixty_thieves)
Sol.register_game(streets)
Sol.register_game(triple_interchange)
Sol.register_game(triple_line)
Sol.register_game(triple_rail)
Sol.register_game(thirty_nine_steps)
Sol.register_game(twin_queens)
Sol.register_game(unlimited)
Sol.register_game(waning_moon)
Sol.register_game(waterloo)
Sol.register_game(zebra)
