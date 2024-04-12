-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local fan                 = {
    Info       = {
        Name          = "Fan",
        Family        = "Fan",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 18,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 17 and 3 or 1),
                Layout = "Row",
                Rule = Sol.Rules.king_downsuit_top
            }
        end
    },
    on_init    = function(game) Sol.Layout.fan(game, 5) end
}

------

local bear_river          = Sol.copy(fan)
bear_river.Info.Name      = "Bear River"
bear_river.Foundation     = {
    Size = 4,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i == 0 and 1 or 0),
            Rule = Sol.Rules.ff_upsuit_none_l13
        }
    end
}
bear_river.Tableau        = {
    Size = 18,
    Pile = function(i)
        local lastInRow = i % 6 == 5
        return {
            Initial = Sol.Initial.face_up(lastInRow and 2 or 3),
            Layout = "Row",
            Rule = { Base = lastInRow and Sol.Rules.Base.Any() or Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownInSuit(true), Move = Sol.Rules.Move.Top(), Limit = 3 }
        }
    end
}
bear_river.on_init        = function(game) Sol.Layout.fan(game, 6) end

------

local box_fan             = Sol.copy(fan)
box_fan.Info.Name         = "Box Fan"
box_fan.Tableau           = {
    Size = 16,
    Pile = {
        Initial = Sol.Initial.face_up(3),
        Layout = "Row",
        Rule = Sol.Rules.king_downac_top
    }
}
box_fan.on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation
box_fan.on_init           = function(game) Sol.Layout.fan(game, 4) end

------

local ceiling_fan         = Sol.copy(fan)
ceiling_fan.Info.Name     = "Ceiling Fan"
ceiling_fan.Tableau       = {
    Size = 18,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = Sol.Rules.king_downac_top
        }
    end
}

------

local clover_leaf         = {
    Info              = {
        Name          = "Clover Leaf",
        Family        = "Fan",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation        = {
        Size = 4,
        Pile = function(i)
            if i < 2 then
                return { Rule = { Base = Sol.Rules.Base.CardColor("Black", "Ace"), Build = Sol.Rules.Build.UpInSuit(), Move = Sol.Rules.Move.Top() } }
            else
                return { Rule = { Base = Sol.Rules.Base.CardColor("Red", "King"), Build = Sol.Rules.Build.DownInSuit(), Move = Sol.Rules.Move.Top() } }
            end
        end
    },
    Tableau           = {
        Size = 16,
        Pile = {
            Initial = Sol.Initial.face_up(3),
            Layout = "Row",
            Rule = { Base = Sol.Rules.Base.Ranks({ "Ace", "King" }), Build = Sol.Rules.Build.UpOrDownInSuit(), Move = Sol.Rules.Move.Top() }
        }
    },
    on_before_shuffle = function(game, card)
        if card.Rank == "Ace" and card.Color == "Black" then
            return game.PlaceTop(card, game.Foundation, 1, 2, true)
        end
        if card.Rank == "King" and card.Color == "Red" then
            return game.PlaceTop(card, game.Foundation, 3, 2, true)
        end

        return false
    end,
    on_init           = function(game) Sol.Layout.fan(game, 4) end
}


------

local alexander_the_great         = Sol.copy(clover_leaf)
alexander_the_great.Info.Name     = "Alexander the Great"
alexander_the_great.Tableau       = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.face_up(4),
        Layout = "Row",
        Rule = { Base = Sol.Rules.Base.Ranks({ "Ace", "King" }), Build = Sol.Rules.Build.UpOrDownInSuit(), Move = Sol.Rules.Move.Top() }
    }
}
alexander_the_great.on_init       = function(game) Sol.Layout.fan(game, 3) end

------

local house_in_the_wood           = Sol.copy(fan)
house_in_the_wood.Info.Name       = "House in the Wood"
house_in_the_wood.Info.DeckCount  = 2
house_in_the_wood.Foundation.Size = 8
house_in_the_wood.Tableau         = {
    Size = 35,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 34 and 3 or 2),
            Layout = "Row",
            Rule = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownInSuit(), Move = Sol.Rules.Move.Top() }
        }
    end
}
house_in_the_wood.on_init         = function(game) Sol.Layout.fan(game, 6) end

------

local house_on_the_hill           = Sol.copy(house_in_the_wood)
house_on_the_hill.Info.Name       = "House on the Hill"
house_on_the_hill.Foundation.Pile = function(i)
    if (i < 4) then
        return {
            Rule = Sol.Rules.ace_upsuit_top
        }
    else
        return {
            Rule = Sol.Rules.king_downsuit_top
        }
    end
end

------

local quads                       = Sol.copy(fan)
quads.Info.Name                   = "Quads"
quads.Tableau                     = {
    Size = 13,
    Pile = {
        Initial = Sol.Initial.face_up(4),
        Layout = "Row",
        Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.Top(), Limit = 4 }
    }
}
quads.on_shuffle                  = function(game, card, pile)
    if pile.Type == "Tableau" and card.Rank == "Ace" then
        return game.PlaceTop(card, game.Foundation, true)
    end

    return false
end

------

local quads_plus                  = Sol.copy(fan)
quads_plus.Info.Name              = "Quads+"
quads_plus.Tableau                = {
    Size = 13,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 12 and 4 or 0),
            Layout = "Row",
            Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.Top(), Limit = 4 }
        }
    end
}
quads_plus.on_before_shuffle      = Sol.Ops.Shuffle.ace_to_foundation

------

local lucky_piles_pos             = {
    { 0, 1 }, { 2, 1 }, { 4, 1 }, { 6, 1 }, { 8, 1 },
    { 2, 2 }, { 4, 2 }, { 6, 2 },
    { 0, 3 }, { 2, 3 }, { 4, 3 }, { 6, 3 }, { 8, 3 } }

local lucky_piles                 = {
    Info       = {
        Name          = "Lucky Piles",
        Family        = "Fan",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = (i + 0.5) * 2, y = 0 },
                Rule = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 13,
        Pile = function(i)
            return {
                Position = { x = lucky_piles_pos[i + 1][1], y = lucky_piles_pos[i + 1][2] },
                Initial = Sol.Initial.face_up(4),
                Layout = "Row",
                Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.UpOrDownInSuit(), Move = Sol.Rules.Move.Top() }
            }
        end
    }
}

------

local scotch_patience             = Sol.copy(fan)
scotch_patience.Info.Name         = "Scotch Patience"
scotch_patience.Foundation        = {
    Size = 4,
    Pile = { Rule = Sol.Rules.ace_upac_top }
}
scotch_patience.Tableau           = {
    Size = 18,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = Sol.Rules.none_downrank_top
        }
    end
}

------

local shamrocks                   = Sol.copy(fan)
shamrocks.Info.Name               = "Shamrocks"
shamrocks.Tableau                 = {
    Size = 18,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownByRank(), Move = Sol.Rules.Move.Top(), Limit = 3 }
        }
    end
}

------

local shamrocks_2                 = Sol.copy(fan)
shamrocks_2.Info.Name             = "Shamrocks II"
shamrocks_2.Tableau               = {
    Size = 18,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownByRank(), Move = Sol.Rules.Move.Top(), Limit = 3 }
        }
    end
}
shamrocks_2.on_after_shuffle      = Sol.Ops.Shuffle.kings_to_bottom

------

local troika                      = Sol.copy(fan)
troika.Info.Name                  = "Troika"
troika.Tableau                    = {
    Size = 18,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.Top(), Limit = 3 }
        }
    end
}
troika.on_shuffle                 = function(game, card, pile)
    if pile.Type == "Tableau" and card.Rank == "Ace" then
        return game.PlaceTop(card, game.Foundation, true)
    end

    return false
end

------------------------

Sol.register_game(fan)
Sol.register_game(alexander_the_great)
Sol.register_game(bear_river)
Sol.register_game(box_fan)
Sol.register_game(ceiling_fan)
Sol.register_game(clover_leaf)
Sol.register_game(house_in_the_wood)
Sol.register_game(house_on_the_hill)
Sol.register_game(quads)
Sol.register_game(quads_plus)
Sol.register_game(lucky_piles)
Sol.register_game(scotch_patience)
Sol.register_game(shamrocks)
Sol.register_game(shamrocks_2)
Sol.register_game(troika)
