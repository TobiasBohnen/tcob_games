-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local canfield = {
    Info        = {
        Name      = "Canfield",
        Family    = "Canfield",
        DeckCount = 1,
        Redeals   = -1
    },
    Stock       = { Initial = Sol.Initial.face_down(34) },
    Waste       = {},
    Reserve     = {
        Initial = Sol.Initial.top_face_up(13),
        Layout = Sol.Pile.Layout.Column
    },
    Foundation  = {
        Size = 4,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i == 0 and 1 or 0),
                Rule    = Sol.Rules.ff_upsuit_none
            }
        end
    },
    Tableau     = {
        Size = 4,
        Pile = {
            Initial = Sol.Initial.face_up(1),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownAlternateColors(true), Move = Sol.Rules.Move.TopOrPile() }
        }
    },
    on_end_turn = function(game) Sol.Ops.Deal.to_group(game.Reserve[1], game.Tableau, Sol.DealMode.IfEmpty) end,
    redeal      = Sol.Ops.Redeal.waste_to_stock,
    deal        = Sol.Ops.Deal.stock_to_waste_by_3,
    on_init     = Sol.Layout.canfield
}


------

local canfield_rush        = Sol.copy(canfield)
canfield_rush.Info.Name    = "Canfield Rush"
canfield_rush.Info.Redeals = 2
canfield_rush.deal         = Sol.Ops.Deal.stock_to_waste_by_redeals_left


------

local double_canfield           = Sol.copy(canfield)
double_canfield.Info.Name       = "Double Canfield"
double_canfield.Info.DeckCount  = 2
double_canfield.Stock.Initial   = Sol.Initial.face_down(85)
double_canfield.Foundation.Size = 8
double_canfield.Tableau.Size    = 5


------

local triple_canfield           = Sol.copy(canfield)
triple_canfield.Info.Name       = "Triple Canfield"
triple_canfield.Info.DeckCount  = 3
triple_canfield.Stock.Initial   = Sol.Initial.face_down(122)
triple_canfield.Reserve.Initial = Sol.Initial.top_face_up(26)
triple_canfield.Foundation.Size = 12
triple_canfield.Tableau.Size    = 7


------

local quadruple_canfield           = Sol.copy(canfield)
quadruple_canfield.Info.Name       = "Quadruple Canfield"
quadruple_canfield.Info.DeckCount  = 4
quadruple_canfield.Stock.Initial   = Sol.Initial.face_down(160)
quadruple_canfield.Reserve.Initial = Sol.Initial.top_face_up(39)
quadruple_canfield.Foundation.Size = 16
quadruple_canfield.Tableau.Size    = 8


------

local superior_canfield           = Sol.copy(canfield)
superior_canfield.Info.Name       = "Superior Canfield"
superior_canfield.Reserve.Initial = Sol.Initial.face_up(13)
superior_canfield.on_end_turn     = nil


------

local variegated_canfield           = Sol.copy(double_canfield)
variegated_canfield.Info.Name       = "Variegated Canfield"
variegated_canfield.Info.Redeals    = 2
variegated_canfield.Reserve.Initial = Sol.Initial.face_up(13)


------

local acme           = Sol.copy(canfield)
acme.Info.Name       = "Acme"
acme.Info.Redeals    = 1
acme.Stock.Initial   = Sol.Initial.face_down(31)
acme.Foundation.Pile = { Rule = Sol.Rules.ace_upsuit_none }
acme.Tableau.Pile    = {
    Initial = Sol.Initial.face_up(1),
    Layout  = Sol.Pile.Layout.Column,
    Rule    = Sol.Rules.any_downsuit_top
}
acme.on_before_setup = Sol.Ops.Setup.ace_to_foundation
acme.deal            = Sol.Ops.Deal.stock_to_waste


------

local american_toad           = Sol.copy(canfield)
american_toad.Info.Name       = "American Toad"
american_toad.Info.DeckCount  = 2
american_toad.Info.Redeals    = 1
american_toad.Stock.Initial   = Sol.Initial.face_down(75)
american_toad.Reserve         = {
    Initial = Sol.Initial.face_up(20),
    Layout = Sol.Pile.Layout.Column
}
american_toad.Foundation.Size = 8
american_toad.Tableau         = {
    Size = 8,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownInSuit(true), Move = Sol.Rules.Move.TopOrPile() }
    }
}
american_toad.deal            = Sol.Ops.Deal.stock_to_waste


------

local beehive         = Sol.copy(canfield)
beehive.Info.Name     = "Beehive"
beehive.Stock.Initial = Sol.Initial.face_down(33)
beehive.Foundation    = { Rule = Sol.Rules.none_none_none }
beehive.Tableau       = {
    Size = 6,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.InSeq() }
    }
}
beehive.on_end_turn   = function(game)
    local tableau = game.Tableau
    for _, tab in ipairs(tableau) do
        if tab.CardCount == 4 then
            tab:move_cards(game.Foundation[1], 1, 4, false)
        end
    end

    Sol.Ops.Deal.to_group(game.Reserve[1], tableau, Sol.DealMode.IfEmpty)
end


------

local chameleon         = Sol.copy(canfield)
chameleon.Info.Name     = "Chameleon"
chameleon.Info.Redeals  = 0
chameleon.Stock.Initial = Sol.Initial.face_down(36)
chameleon.Reserve.Pile  = {
    Initial = Sol.Initial.top_face_up(12),
    Layout = Sol.Pile.Layout.Column
}
chameleon.Tableau       = {
    Size = 3,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(true), Move = Sol.Rules.Move.TopOrPile() }
    }
}
chameleon.deal          = Sol.Ops.Deal.stock_to_waste


------

local demon           = Sol.copy(canfield)
demon.Info.Name       = "Demon"
demon.Info.DeckCount  = 2
demon.Stock.Initial   = Sol.Initial.face_down(55)
demon.Reserve.Pile    = {
    Initial = Sol.Initial.top_face_up(40),
    Layout = Sol.Pile.Layout.Column
}
demon.Foundation.Size = 8
demon.Tableau         = {
    Size = 8,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownAlternateColors(true), Move = Sol.Rules.Move.InSeq() }
    }
}


------

local eagle_wing_pos = {
    { 0, 1.75 }, { 1, 1.5 }, { 2, 1.25 }, { 3, 1.5 }, { 5, 1.5 }, { 6, 1.25 }, { 7, 1.5 }, { 8, 1.75 }
}

local eagle_wing     = {
    Info        = {
        Name      = "Eagle Wing",
        Family    = "Canfield",
        DeckCount = 1,
        Redeals   = 2
    },
    Stock       = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(30)
    },
    Waste       = { Position = { x = 1, y = 0 } },
    Reserve     = {
        Position = { x = 4, y = 2 },
        Initial = Sol.Initial.face_down(13),
        Layout = Sol.Pile.Layout.Squared
    },
    Foundation  = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Initial  = Sol.Initial.face_up(i == 0 and 1 or 0),
                Rule     = Sol.Rules.ff_upsuit_none
            }
        end
    },
    Tableau     = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = eagle_wing_pos[i + 1][1], y = eagle_wing_pos[i + 1][2] },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.DownInSuit(true), Move = Sol.Rules.Move.Top(), Limit = 3 }
            }
        end
    },
    on_end_turn = function(game)
        Sol.Ops.Deal.to_group(game.Reserve[1], game.Tableau, Sol.DealMode.IfEmpty)
        game.Reserve[1]:flip_down_top_card()
    end,
    redeal      = Sol.Ops.Redeal.waste_to_stock,
    deal        = Sol.Ops.Deal.stock_to_waste
}


------

local lafayette         = Sol.copy(canfield)
lafayette.Info.Name     = "Lafayette"
lafayette.Stock.Initial = Sol.Initial.face_down(35)
lafayette.Foundation    = {
    Size = 8,
    Pile = function(i)
        if i < 4 then
            return {
                Rule = Sol.Rules.ace_upsuit_none
            }
        else
            return {
                Rule = Sol.Rules.king_downsuit_none
            }
        end
    end
}
lafayette.Tableau       = {
    Size = 4,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = { Base = Sol.Rules.Base.Ranks({ "Seven" }), Build = Sol.Rules.Build.DownAlternateColors(), Move = Sol.Rules.Move.InSeq() }
    }
}


------

local minerva           = Sol.copy(canfield)
minerva.Info.Name       = "Minerva"
minerva.Info.Redeals    = 1
minerva.Stock.Initial   = Sol.Initial.face_down(13)
minerva.Reserve.Pile    = {
    Initial = Sol.Initial.top_face_up(11),
    Layout = Sol.Pile.Layout.Column
}
minerva.Foundation.Pile = { Rule = Sol.Rules.ace_upsuit_none }
minerva.Tableau         = {
    Size = 7,
    Pile = {
        Initial = Sol.Initial.alternate(4, false),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.king_downac_inseq
    }
}
minerva.on_end_turn     = nil
minerva.deal            = Sol.Ops.Deal.stock_to_waste


------

local munger         = Sol.copy(minerva)
munger.Info.Name     = "Munger"
munger.Info.Redeals  = 0
munger.Stock.Initial = Sol.Initial.face_down(17)
munger.Reserve.Pile  = {
    Initial = Sol.Initial.top_face_up(7),
    Layout = Sol.Pile.Layout.Column
}


------

local mystique         = Sol.copy(minerva)
mystique.Info.Name     = "Mystique"
mystique.Info.Redeals  = 0
mystique.Stock.Initial = Sol.Initial.face_down(15)
mystique.Reserve.Pile  = {
    Initial = Sol.Initial.top_face_up(9),
    Layout = Sol.Pile.Layout.Column
}
mystique.Tableau       = {
    Size = 7,
    Pile = {
        Initial = Sol.Initial.alternate(4, false),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downac_inseq
    }
}


------

local rainbow        = Sol.copy(canfield)
rainbow.Info.Name    = "Rainbow"
rainbow.Info.Redeals = 0
rainbow.Tableau.Pile = {
    Initial = Sol.Initial.face_up(1),
    Layout  = Sol.Pile.Layout.Column,
    Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(true), Move = Sol.Rules.Move.TopOrPile() }
}
rainbow.deal         = Sol.Ops.Deal.stock_to_waste


------

local rainfall        = Sol.copy(canfield)
rainfall.Info.Name    = "Rainfall"
rainfall.Info.Redeals = 2
rainfall.deal         = Sol.Ops.Deal.stock_to_waste


------

local storehouse        = Sol.copy(canfield)
storehouse.Info.Name    = "Storehouse"
storehouse.Info.Redeals = 2
storehouse.Tableau.Pile = {
    Initial = Sol.Initial.face_up(1),
    Layout  = Sol.Pile.Layout.Column,
    Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownInSuit(true), Move = Sol.Rules.Move.TopOrPile() }
}
storehouse.deal         = Sol.Ops.Deal.stock_to_waste


------

local the_plot = {
    Info = {
        Name      = "The Plot",
        Family    = "Canfield",
        DeckCount = 2,
        Redeals   = 0
    },
    Stock = { Initial = Sol.Initial.face_down(78) },
    Waste = {},
    Reserve = {
        Initial = Sol.Initial.top_face_up(13),
        Layout = Sol.Pile.Layout.Column
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i == 0 and 1 or 0),
                Rule    = {
                    Base = Sol.Rules.Base.FirstFoundation(),
                    Build = Sol.Rules.Build.UpByRank(true),
                    Move = Sol.Rules.Move.None()
                }
            }
        end
    },
    Tableau = {
        Size = 12,
        Pile = {
            Initial = Sol.Initial.face_up(1),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = {
                Base = {
                    Hint = { "Any" },
                    Func = function(game, card, numCards)
                        if numCards > 1 then return false end
                        -- base is first foundation card until first pile is complete
                        if game.Foundation[1].CardCount ~= 13 then
                            return card.Rank == game.Foundation[1].Cards[1].Rank
                        end
                        return true
                    end
                },
                Build = Sol.Rules.Build.DownByRank(true),
                Move = Sol.Rules.Move.Top()
            }
        }
    },
    deal = Sol.Ops.Deal.stock_to_waste,
    on_init = Sol.Layout.canfield,
    can_play = function(game, targetPile, targetCardIndex, card, numCards)
        -- block other foundation piles until first pile is complete
        if targetPile.Type == Sol.Pile.Type.Foundation and targetPile.Index ~= 1 then
            if game.Foundation[1].CardCount ~= 13 then return false end
        end
        --empty tableau piles can only be filled from the waste pile
        if targetPile.Type == Sol.Pile.Type.Tableau and targetPile.IsEmpty then
            if game:find_pile(card).Type ~= Sol.Pile.Type.Waste then return false end
        end
        --reserve only to foundation pile
        if targetPile.Type == Sol.Pile.Type.Reserve then
            if game:find_pile(card).Type ~= Sol.Pile.Type.Foundation then return false end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end
}


------

------------------------

Sol.register_game(canfield)
Sol.register_game(acme)
Sol.register_game(american_toad)
Sol.register_game(beehive)
Sol.register_game(canfield_rush)
Sol.register_game(chameleon)
Sol.register_game(demon)
Sol.register_game(double_canfield)
Sol.register_game(eagle_wing)
Sol.register_game(lafayette)
Sol.register_game(minerva)
Sol.register_game(munger)
Sol.register_game(mystique)
Sol.register_game(quadruple_canfield)
Sol.register_game(rainbow)
Sol.register_game(rainfall)
Sol.register_game(storehouse)
Sol.register_game(superior_canfield)
Sol.register_game(the_plot)
Sol.register_game(triple_canfield)
Sol.register_game(variegated_canfield)
