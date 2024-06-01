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
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.any_downac_inseq
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
            Rule    = Sol.Rules.ff_upsuit_none_l13
        }
    end
}
agnes_sorel.Tableau        = {
    Size = 7,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i + 1),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.DownInColor(), Move = Sol.Rules.Move.InSeq() }
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
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downsuit_inseq
    }
}
blockade.on_init       = Sol.Layout.klondike
blockade.on_end_turn   = function(game) Sol.Ops.Deal.to_group(game.Stock[1], game.Tableau, Sol.DealMode.IfEmpty) end


------

local die_koenigsbergerin                = Sol.copy(gypsy)
die_koenigsbergerin.Info.Name            = "Die Königsbergerin"
die_koenigsbergerin.Foundation.Pile.Rule = Sol.Rules.ace_upsuit_none
die_koenigsbergerin.Tableau.Pile.Initial = Sol.Initial.face_up(3)
die_koenigsbergerin.on_shuffle           = function(game, card, pile)
    if pile.Type == Sol.Pile.Type.Tableau and card.Rank == "Ace" then
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

local elba         = Sol.copy(gypsy)
elba.Info.Name     = "Elba"
elba.Stock.Initial = Sol.Initial.face_down(54)
elba.Tableau       = {
    Size = 10,
    Pile = {
        Initial = Sol.Initial.top_face_up(5),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.king_downac_inseq
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
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.king_downsuit_inseq
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
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.king_downac_inseq
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
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.king_downac_inseq
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
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.king_downac_inseq
    }
end


------

local flamenco             = Sol.copy(gypsy)
flamenco.Info.Name         = "Flamenco"
flamenco.Stock.Initial     = Sol.Initial.face_down(72)
flamenco.Tableau.Pile      = {
    Initial = Sol.Initial.face_up(3),
    Layout  = Sol.Pile.Layout.Column,
    Rule    = Sol.Rules.any_downac_inseq
}
flamenco.Foundation.Pile   = function(i)
    return { Rule = i < 4 and Sol.Rules.ace_upsuit_top_l13 or Sol.Rules.king_downsuit_top_l13 }
end
flamenco.on_before_shuffle = Sol.Ops.Shuffle.ace_and_king_to_foundation


------

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
        IsPlayable = function(game) return game.Stock[1].IsEmpty end,
        Func = function(_, target, idx) return idx == target.CardCount end
    }
}
giant.on_init              = Sol.Layout.gypsy


------

local irmgard         = Sol.copy(gypsy)
irmgard.Info.Name     = "Irmgard"
irmgard.Stock.Initial = Sol.Initial.face_down(79)
irmgard.Tableau       = {
    Size = 9,
    Pile = function(i)
        return {
            Initial = Sol.Initial.top_face_up(i < 5 and i + 1 or 9 - i),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.king_downac_inseq
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
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.king_downac_inseq
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
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.any_downac_inseq
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
                Rule     = Sol.Rules.none_none_top
            }
        end
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = 8, y = i + 1 },
                Rule     = { Base = Sol.Rules.Base.Ace(), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    Tableau    = {
        Size = 7,
        Pile = function(i)
            return {
                Position = { x = i + 1, y = 0 },
                Initial  = Sol.Initial.top_face_up(i < 4 and i + 1 or 7 - i),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.any_downac_inseq
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
            return Sol.Ops.Deal.to_group(game.Stock[1], game.Reserve)
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
                Initial  = Sol.Initial.face_down(4),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.none_none_top
            }
        end
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = 2 + i, y = 1.5 },
                Rule     = Sol.Rules.ace_upsuit_none
            }
        end
    },
    Tableau    = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = 2 + i, y = 2.5 },
                Initial  = Sol.Initial.top_face_up(3),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.any_downac_inseq
            }
        end
    },
    on_drop    = function(game, pile)
        if pile.Type == Sol.Pile.Type.Foundation then
            game.Reserve[pile.Index]:flip_up_top_card()
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
                Rule     = Sol.Rules.any_none_top
            }
        end
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = 2 + i, y = 0 },
                Rule     = Sol.Rules.ace_upsuit_none
            }
        end
    },
    Tableau    = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = 2 + i, y = 1 },
                Initial  = Sol.Initial.top_face_up(1),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.king_downac_inseq
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
        Layout = Sol.Pile.Layout.Column,
        Rule   = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.None(), Move = Sol.Rules.Move.FaceUp() }
    },
    Foundation = {
        Size = 8,
        Pile = { Rule = Sol.Rules.ace_upsuit_none }
    },
    Tableau    = {
        Size = 8,
        Pile = {
            Initial = Sol.Initial.top_face_up(1),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.king_downac_inseq
        }
    },
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        -- FreeCell is usable if Stock is empty
        if targetPile.Type == Sol.Pile.Type.FreeCell and not game.Stock[1].IsEmpty then
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
                Layout  = Sol.Pile.Layout.Column,
                Rule    = Sol.Rules.any_downac_inseq
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
    Stock      = { --Stock turns into FreeCell when empty
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(49)
    },
    FreeCell   = {
        Position = { x = 0, y = 0 },
        Rule     = Sol.Rules.any_any_top
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Rule = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 10,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial  = Sol.Initial.top_face_up(i + 1),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.king_downac_inseq
            }
        end
    },
    deal       = Sol.Ops.Deal.stock_to_tableau
}


------

local surprise         = Sol.copy(right_triangle)
surprise.Info.Name     = "Surprise"
surprise.Stock.Initial = Sol.Initial.face_down(68)
surprise.FreeCell.Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.Any(), Move = Sol.Rules.Move.Top(), Limit = 3 }
surprise.Tableau       = {
    Size = 11,
    Pile = function(i)
        return {
            Initial = Sol.Initial.top_face_up(i < 6 and i + 1 or 11 - i),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.king_downac_inseq
        }
    end
}


------
local function swiss_patience_get_rank(card)
    local rank_values <const> = { Two = 1, Three = 2, Four = 3, Five = 4, Six = 5, Seven = 6, Eight = 7, Nine = 8, Ten = 9, Jack = 10, Queen = 11, King = 12, Ace = 13 }
    local rank <const> = { "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King", "Ace" }
    return rank[rank_values[card.Rank] - 1]
end

local swiss_patience = {
    Info       = {
        Name      = "Swiss Patience",
        Family    = "Gypsy",
        DeckCount = 1
    },
    Stock      = {
        Initial = Sol.Initial.face_down(27)
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = { Base = Sol.Rules.Base.Ranks({ "Two" }), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.Top(), Limit = 13 } }
    },
    Tableau    = {
        Size = 9,
        Pile = function(i)
            return {
                Initial = Sol.Initial.top_face_up(i < 5 and i + 1 or 9 - i),
                Layout  = Sol.Pile.Layout.Column,
                Rule    = {
                    Base = Sol.Rules.Base.Ace(),
                    Build = {
                        Hint = "Down by alternate color",
                        Func = function(_, base, drop)
                            return base.Color ~= drop.Color and swiss_patience_get_rank(base) == drop.Rank
                        end
                    },
                    Move = Sol.Rules.Move.InSeq()
                }
            }
        end
    },
    on_init    = Sol.Layout.canfield,
    deal       = Sol.Ops.Deal.stock_to_tableau
}


------

local trapdoor = {
    Info       = {
        Name      = "Trapdoor",
        Family    = "Gypsy",
        DeckCount = 2
    },
    Stock      = {
        Position = { x = 8.5, y = 4 },
        Initial = Sol.Initial.face_down(72)
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i % 2 + 8, y = i // 2 },
                Rule = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Reserve    = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i, y = 4 },
                Initial = Sol.Initial.face_up(1),
                Rule = Sol.Rules.none_none_top
            }
        end
    },
    Tableau    = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i, y = 0 },
                Initial  = Sol.Initial.top_face_up(3),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.any_downac_inseq
            }
        end
    },
    deal       = function(game)
        local stock = game.Stock[1]
        if stock.IsEmpty then return false end

        --move reserve to tableau
        local tableau = game.Tableau
        for _, res in ipairs(game.Reserve) do
            if not res.IsEmpty then
                res:move_cards(tableau[res.Index], 1, 1, false)
            end
        end

        return Sol.Ops.Deal.to_group(stock, game.Reserve)
    end


}


------


------------------------

Sol.register_game(gypsy)
Sol.register_game(agnes_sorel)
Sol.register_game(blockade)
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
Sol.register_game(right_triangle)
Sol.register_game(scarp)
Sol.register_game(small_triangle)
Sol.register_game(surprise)
Sol.register_game(swiss_patience)
Sol.register_game(trapdoor)
Sol.register_game(yeast_dough)
