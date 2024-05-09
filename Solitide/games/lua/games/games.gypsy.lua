-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local gypsy = {
    Info       = {
        Name      = "Gypsy",
        Family    = "Gypsy",
        DeckCount = 2
    },
    Stock      = {
        Initial = Sol.Initial.face_down(80)
    },
    Foundation = {
        Size = 8,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 8,
        Pile = {
            Initial = Sol.Initial.top_face_up(3),
            Layout = "Column",
            Rule = Sol.Rules.any_downac_inseq
        }
    },
    on_init    = Sol.Layout.gypsy,
    deal       = Sol.Ops.Deal.stock_to_tableau
}


------

local agnes_sorel          = Sol.copy(gypsy)
agnes_sorel.Info.Name      = "Agnes Sorel"
agnes_sorel.Info.DeckCount = 1
agnes_sorel.Stock.Initial  = Sol.Initial.face_down(23)
agnes_sorel.Foundation     = {
    Size = 4,
    Pile = function(i)
        return {
            Initial = i == 0 and Sol.Initial.face_up(1) or {},
            Rule = Sol.Rules.ff_upsuit_none_l13
        }
    end
}
agnes_sorel.Tableau        = {
    Size = 7,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i + 1),
            Layout = "Column",
            Rule = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.DownInColor(), Move = Sol.Rules.Move.InSeq() }
        }
    end
}
agnes_sorel.on_init        = Sol.Layout.klondike


------

local blockade         = Sol.copy(gypsy)
blockade.Info.Name     = "Blockade"
blockade.Stock.Initial = Sol.Initial.face_down(92)
blockade.Tableau       = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout = "Column",
        Rule = Sol.Rules.any_downsuit_inseq
    }
}
blockade.on_init       = Sol.Layout.klondike
blockade.on_end_turn   = function(game) Sol.Ops.Deal.to_group(game.Stock[1], game.Tableau, true) end


------

local die_koenigsbergerin                = Sol.copy(gypsy)
die_koenigsbergerin.Info.Name            = "Die Königsbergerin"
die_koenigsbergerin.Foundation.Pile.Rule = Sol.Rules.ace_upsuit_none
die_koenigsbergerin.Tableau.Pile.Initial = Sol.Initial.face_up(3)
die_koenigsbergerin.on_shuffle           = function(game, card, pile)
    if pile.Type == "Tableau" and card.Rank == "Ace" then
        return game.PlaceTop(card, game.Foundation, true)
    end

    return false
end
die_koenigsbergerin.on_after_shuffle     = function(game)
    -- refill Tableau from Stock back to three cards
    local tableau = game.Tableau
    local stock = game.Stock[1]
    local stockCards = stock.Cards
    local idx = #stockCards
    for _, tab in ipairs(tableau) do
        while tab.CardCount < 3 do
            while stockCards[idx].Rank == "Ace" do --skip aces
                idx = idx - 1
            end

            stock:move_cards(tab, idx, 1, false)
            idx = idx - 1
        end
        tab:flip_up_cards()
    end
end


------

local churchill         = Sol.copy(gypsy)
churchill.Info.Name     = "Churchill"
churchill.Stock.Initial = Sol.Initial.face_down(68)
churchill.Reserve       = {
    Initial = Sol.Initial.face_up(6),
    Layout = "Column",
    Rule = Sol.Rules.none_none_top
}
churchill.Tableau       = {
    Size = 10,
    Pile = function(i)
        return {
            Initial = Sol.Initial.top_face_up(i < 5 and i + 1 or 10 - i),
            Layout = "Column",
            Rule = Sol.Rules.king_downac_inseq
        }
    end
}
churchill.deal          = function(game)
    -- don't deal card on sequence with king as first card in pile
    local stock = game.Stock[1]
    local tableau = game.Tableau

    if stock.IsEmpty then return false end
    for _, toPile in ipairs(tableau) do
        if stock.IsEmpty then break end
        local inSeq = true
        local cards = toPile.Cards
        for index, card in ipairs(cards) do
            if index > 13 or card.IsFaceDown or card.Rank ~= Sol.Ranks[14 - index] then
                inSeq = false
                break
            end
        end

        if not inSeq then
            stock:move_cards(toPile, #stock.Cards, 1, false)
            toPile:flip_up_top_card()
        end
    end
    return true
end
churchill.can_play      = function(game, targetPile, targetCardIndex, card, numCards)
    -- reserve only to foundation
    if game:find_pile(card).Type == "Reserve" and targetPile.Type ~= "Foundation" then return false end

    return game:can_play(targetPile, targetCardIndex, card, numCards)
end
churchill.on_init       = Sol.Layout.canfield


------

local pitt_the_younger           = Sol.copy(churchill)
pitt_the_younger.Info.Name       = "Pitt the Younger"
pitt_the_younger.Stock.Initial   = Sol.Initial.face_down(63)
pitt_the_younger.Reserve.Initial = Sol.Initial.face_up(11)


------

local elba         = Sol.copy(gypsy)
elba.Info.Name     = "Elba"
elba.Stock.Initial = Sol.Initial.face_down(54)
elba.Tableau       = {
    Size = 10,
    Pile = {
        Initial = Sol.Initial.top_face_up(5),
        Layout = "Column",
        Rule = Sol.Rules.king_downac_inseq
    }
}
elba.on_init       = Sol.Layout.klondike


------

local eclipse         = Sol.copy(gypsy)
eclipse.Info.Name     = "Eclipse"
eclipse.Stock.Initial = Sol.Initial.face_down(52)
eclipse.Tableau       = {
    Size = 13,
    Pile = {
        Initial = Sol.Initial.face_up(4),
        Layout = "Column",
        Rule = Sol.Rules.king_downsuit_inseq
    }
}
eclipse.on_init       = Sol.Layout.klondike


------

local hypotenuse         = Sol.copy(gypsy)
hypotenuse.Info.Name     = "Hypotenuse"
hypotenuse.Stock.Initial = Sol.Initial.face_down(49)
hypotenuse.Tableau       = {
    Size = 10,
    Pile = function(i)
        return {
            Initial = Sol.Initial.top_face_up(10 - i),
            Layout = "Column",
            Rule = Sol.Rules.king_downac_inseq
        }
    end
}
hypotenuse.on_init       = Sol.Layout.klondike


------

local small_triangle           = Sol.copy(gypsy)
small_triangle.Info.Name       = "Small Triangle"
small_triangle.Info.DeckCount  = 1
small_triangle.Stock.Initial   = Sol.Initial.face_down(24)
small_triangle.Foundation.Size = 4
small_triangle.Tableau         = {
    Size = 7,
    Pile = function(i)
        return {
            Initial = Sol.Initial.top_face_up(i + 1),
            Layout = "Column",
            Rule = Sol.Rules.king_downac_inseq
        }
    end
}
small_triangle.on_init         = Sol.Layout.klondike


------

local eternal_triangle        = Sol.copy(hypotenuse)
eternal_triangle.Info.Name    = "Eternal Triangle"
eternal_triangle.Tableau.Pile = function(i)
    return {
        Initial = Sol.Initial.face_up(10 - i),
        Layout = "Column",
        Rule = Sol.Rules.king_downac_inseq
    }
end


------

local flamenco             = Sol.copy(gypsy)
flamenco.Info.Name         = "Flamenco"
flamenco.Stock.Initial     = Sol.Initial.face_down(72)
flamenco.Tableau.Pile      = {
    Initial = Sol.Initial.face_up(3),
    Layout = "Column",
    Rule = Sol.Rules.any_downac_inseq
}
flamenco.Foundation.Pile   = function(i)
    return { Rule = i < 4 and Sol.Rules.ace_upsuit_top or Sol.Rules.king_downsuit_top }
end
flamenco.on_before_shuffle = function(game, card)
    if card.Rank == "Ace" then
        return game.PlaceTop(card, game.Foundation, 1, 4, true)
    end
    if card.Rank == "King" then
        return game.PlaceTop(card, game.Foundation, 5, 4, true)
    end

    return false
end


------

local giant_check          = function(game)
    if game.Stock[1].IsEmpty then
        local foundation = game.Foundation
        for _, fou in ipairs(foundation) do
            fou.IsPlayable = true
        end
    end
end

local giant                = Sol.copy(gypsy)
giant.Info.Name            = "Giant"
giant.Stock.Initial        = Sol.Initial.face_down(96)
giant.Tableau.Pile.Initial = Sol.Initial.face_up(1)
giant.Foundation.Pile.Rule = {
    Base = Sol.Rules.Base.Ace(),
    Build = Sol.Rules.Build.UpInSuit(),
    Move = {
        Hint = "Top card (if Stock is empty)",
        IsSequence = false,
        IsPlayable = false,
        Func = function(_, target, idx)
            return idx == target.CardCount
        end
    }
}
giant.on_init              = function(game)
    Sol.Layout.gypsy(game)
    giant_check(game)
end
giant.on_end_turn          = function(game)
    giant_check(game)
end


------

local irmgard         = Sol.copy(gypsy)
irmgard.Info.Name     = "Irmgard"
irmgard.Stock.Initial = Sol.Initial.face_down(79)
irmgard.Tableau       = {
    Size = 9,
    Pile = function(i)
        return {
            Initial = Sol.Initial.top_face_up(i < 5 and i + 1 or 9 - i),
            Layout = "Column",
            Rule = Sol.Rules.king_downac_inseq
        }
    end
}
irmgard.on_init       = Sol.Layout.klondike


------

local millie                = Sol.copy(gypsy)
millie.Info.Name            = "Millie"
millie.Stock.Initial        = Sol.Initial.face_down(96)
millie.Tableau.Pile.Initial = Sol.Initial.face_up(1)


------

local phantom_blockade         = Sol.copy(gypsy)
phantom_blockade.Info.Name     = "Phantom Blockade"
phantom_blockade.Stock.Initial = Sol.Initial.face_down(65)
phantom_blockade.Tableau       = {
    Size = 13,
    Pile = {
        Initial = Sol.Initial.face_up(3),
        Layout = "Column",
        Rule = Sol.Rules.king_downac_inseq
    }
}


------

local scarp           = Sol.copy(gypsy)
scarp.Info.Name       = "Scarp"
scarp.Info.DeckCount  = 3
scarp.Stock.Initial   = Sol.Initial.face_down(65)
scarp.Foundation.Size = 12
scarp.Tableau         = {
    Size = 13,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i + 1),
            Layout = "Column",
            Rule = Sol.Rules.any_downac_inseq
        }
    end
}
scarp.on_init         = Sol.Layout.big_harp


------

local yeast_dough                = Sol.copy(gypsy)
yeast_dough.Info.Name            = "Yeast Dough"
yeast_dough.Tableau.Pile.Initial = Sol.Initial.face_up(3)


------

local cone = {
    Info       = {
        Name      = "Cone",
        Family    = "Gypsy",
        DeckCount = 2
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(88)
    },
    Reserve    = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = 0, y = i + 1 },
                Rule = Sol.Rules.none_none_top
            }
        end
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = 8, y = i + 1 },
                Rule = { Base = Sol.Rules.Base.Ace(), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    Tableau    = {
        Size = 7,
        Pile = function(i)
            return {
                Position = { x = i + 1, y = 0 },
                Initial = Sol.Initial.top_face_up(i < 4 and i + 1 or 7 - i),
                Layout = "Column",
                Rule = Sol.Rules.any_downac_inseq
            }
        end
    },
    deal       = function(game)
        --check if tableau is empty
        local tableau = game.Tableau
        for _, tab in ipairs(tableau) do
            if tab.IsEmpty then return false end
        end
        --deal last 4 cards to reserve
        if game.Stock[1].CardCount == 4 then
            return Sol.Ops.Deal.to_group(game.Stock[1], game.Reserve, false)
        end

        return Sol.Ops.Deal.stock_to_tableau(game)
    end
}


------

local leprechaun = {
    Info       = {
        Name      = "Leprechaun",
        Family    = "Gypsy",
        DeckCount = 2
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(48)
    },
    Reserve    = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = 2 + i, y = 0 },
                Initial = Sol.Initial.face_down(4),
                Layout = "Column",
                Rule = Sol.Rules.none_none_top
            }
        end
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = 2 + i, y = 1.5 },
                Rule = Sol.Rules.ace_upsuit_none
            }
        end
    },
    Tableau    = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = 2 + i, y = 2.5 },
                Initial = Sol.Initial.top_face_up(3),
                Layout = "Column",
                Rule = Sol.Rules.any_downac_inseq
            }
        end
    },
    on_drop    = function(game, pile)
        if pile.Type == "Foundation" then
            local idx = game:get_pile_index(pile)
            game.Reserve[idx]:flip_up_top_card()
        end
    end,
    deal       = Sol.Ops.Deal.stock_to_tableau
}


------

local milligan_cell = {
    Info       = {
        Name      = "Milligan Cell",
        Family    = "Gypsy",
        DeckCount = 2
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(96)
    },
    FreeCell   = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = 0, y = i + 1 },
                Rule = Sol.Rules.any_none_top
            }
        end
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = 2 + i, y = 0 },
                Rule = Sol.Rules.ace_upsuit_none
            }
        end
    },
    Tableau    = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = 2 + i, y = 1 },
                Initial = Sol.Initial.top_face_up(1),
                Layout = "Column",
                Rule = Sol.Rules.king_downac_inseq
            }
        end
    },
    deal       = Sol.Ops.Deal.stock_to_tableau
}


------

local miss_milligan = {
    Info       = {
        Name      = "Miss Milligan",
        Family    = "Gypsy",
        DeckCount = 2
    },
    Stock      = {
        Initial = Sol.Initial.face_down(96)
    },
    FreeCell   = {
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.None(), Move = Sol.Rules.Move.FaceUp() }
    },
    Foundation = {
        Size = 8,
        Pile = { Rule = Sol.Rules.ace_upsuit_none }
    },
    Tableau    = {
        Size = 8,
        Pile = {
            Initial = Sol.Initial.top_face_up(1),
            Layout = "Column",
            Rule = Sol.Rules.king_downac_inseq
        }
    },
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        -- FreeCell is usable if Stock is empty
        if targetPile.Type == "FreeCell" and not game.Stock[1].IsEmpty then
            return false
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    on_init    = Sol.Layout.canfield,
    deal       = Sol.Ops.Deal.stock_to_tableau
}


------

local imperial_guards             = Sol.copy(miss_milligan)
imperial_guards.Info.Name         = "Imperial Guards"
imperial_guards.Tableau.Pile.Rule = Sol.Rules.any_downac_inseq


------

local nomad = {
    Info       = {
        Name      = "Nomad",
        Family    = "Gypsy",
        DeckCount = 2
    },
    Stock      = {
        Initial = Sol.Initial.face_down(72)
    },
    FreeCell   = {
        Rule = Sol.Rules.any_none_top
    },
    Foundation = {
        Size = 8,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(4),
                Layout = "Column",
                Rule = Sol.Rules.any_downac_inseq
            }
        end
    },
    deal       = Sol.Ops.Deal.stock_to_tableau,
    on_init    = Sol.Layout.canfield
}


------

local right_triangle = {
    Info       = {
        Name      = "Right Triangle",
        Family    = "Gypsy",
        DeckCount = 2
    },
    Stock      = {
        Initial = Sol.Initial.face_down(49)
    },
    FreeCell   = {
        Rule = Sol.Rules.any_any_top,
        Layout = "Column"
    },
    Foundation = {
        Size = 8,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = Sol.Initial.top_face_up(i + 1),
                Layout = "Column",
                Rule = Sol.Rules.king_downac_inseq
            }
        end
    },
    on_init    = Sol.Layout.canfield,
    deal       = Sol.Ops.Deal.stock_to_tableau,
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == "FreeCell" and not game.Stock[1].IsEmpty then -- block FreeCell until Stock is empty
            return false
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end
}


------

------------------------

Sol.register_game(gypsy)
Sol.register_game(agnes_sorel)
Sol.register_game(blockade)
Sol.register_game(churchill)
Sol.register_game(cone)
Sol.register_game(die_koenigsbergerin)
Sol.register_game(eclipse)
Sol.register_game(elba)
Sol.register_game(eternal_triangle)
Sol.register_game(flamenco)
Sol.register_game(giant)
Sol.register_game(hypotenuse)
Sol.register_game(imperial_guards)
Sol.register_game(irmgard)
Sol.register_game(leprechaun)
Sol.register_game(millie)
Sol.register_game(milligan_cell)
Sol.register_game(miss_milligan)
Sol.register_game(nomad)
Sol.register_game(phantom_blockade)
Sol.register_game(pitt_the_younger)
Sol.register_game(right_triangle)
Sol.register_game(scarp)
Sol.register_game(small_triangle)
Sol.register_game(yeast_dough)
