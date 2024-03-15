-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local fan                    = {
    Info       = {
        Name          = "Fan",
        Family        = "Fan",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size   = 4,
        Create = { Rule = rules.ace_upsuit_top }
    },
    Tableau    = {
        Size   = 18,
        Create = function(i)
            return {
                Initial = ops.Initial.face_up(i < 17 and 3 or 1),
                Layout = "Row",
                Rule = rules.king_downsuit_top
            }
        end
    },
    on_created = function(game) Sol.Layout.fan(game, 5) end
}

------

local bear_river             = Sol.copy(fan)
bear_river.Info.Name         = "Bear River"
bear_river.Foundation        = {
    Size   = 4,
    Create = function(i)
        return {
            Initial = ops.Initial.face_up(i == 0 and 1 or 0),
            Rule = rules.ff_upsuit_none
        }
    end
}
bear_river.Tableau           = {
    Size   = 18,
    Create = function(i)
        local lastInRow = i % 6 == 5
        return {
            Initial = ops.Initial.face_up(lastInRow and 2 or 3),
            Layout = "Row",
            Rule = { Base = lastInRow and rules.Base.Any or rules.Base.None, Build = rules.Build.UpOrDownInSuit(true), Move = rules.Move.Top(), Limit = 3 }
        }
    end
}
bear_river.on_created        = function(game) Sol.Layout.fan(game, 6) end

------

local box_fan                = Sol.copy(fan)
box_fan.Info.Name            = "Box Fan"
box_fan.Tableau              = {
    Size   = 16,
    Create = {
        Initial = ops.Initial.face_up(3),
        Layout = "Row",
        Rule = rules.king_downac_top
    }
}
box_fan.on_before_shuffle    = ops.Shuffle.ace_to_foundation
box_fan.on_created           = function(game) Sol.Layout.fan(game, 4) end

------

local ceiling_fan            = Sol.copy(fan)
ceiling_fan.Info.Name        = "Ceiling Fan"
ceiling_fan.Tableau          = {
    Size   = 18,
    Create = function(i)
        return {
            Initial = ops.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = rules.king_downac_top
        }
    end
}

------

local clover_leaf            = {
    Info              = {
        Name          = "Clover Leaf",
        Family        = "Fan",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation        = {
        Size   = 4,
        Create = function(i)
            if i < 2 then
                return { Rule = { Base = function(_, card, _) return rules.Base.CardColor(card, "Black", "Ace") end, Build = rules.Build.UpInSuit(), Move = rules.Move.Top() } }
            else
                return { Rule = { Base = function(_, card, _) return rules.Base.CardColor(card, "Red", "King") end, Build = rules.Build.DownInSuit(), Move = rules.Move.Top() } }
            end
        end
    },
    Tableau           = {
        Size   = 16,
        Create = {
            Initial = ops.Initial.face_up(3),
            Layout = "Row",
            Rule = { Base = function(_, card, _) return rules.Base.Ranks(card, { "Ace", "King" }) end, Build = rules.Build.UpOrDownInSuit(), Move = rules.Move.Top() }
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
    on_created        = function(game) Sol.Layout.fan(game, 4) end
}

------

local quads                  = Sol.copy(fan)
quads.Info.Name              = "Quads"
quads.Tableau                = {
    Size   = 13,
    Create = {
        Initial = ops.Initial.face_up(4),
        Layout = "Row",
        Rule = { Base = rules.Base.Any, Build = rules.Build.InRank(), Move = rules.Move.Top(), Limit = 4 }
    }
}
quads.on_shuffle             = function(game, card, pileType)
    if pileType == "Tableau" and card.Rank == "Ace" then
        return game.PlaceTop(card, game.Foundation, true)
    end

    return false
end

------

local quads_plus             = Sol.copy(fan)
quads_plus.Info.Name         = "Quads+"
quads_plus.Tableau           = {
    Size   = 13,
    Create = function(i)
        return {
            Initial = ops.Initial.face_up(i < 12 and 4 or 0),
            Layout = "Row",
            Rule = { Base = rules.Base.Any, Build = rules.Build.InRank(), Move = rules.Move.Top(), Limit = 4 }
        }
    end
}
quads_plus.on_before_shuffle = ops.Shuffle.ace_to_foundation

------

local lucky_piles_pos        = {
    { 0, 1 }, { 2, 1 }, { 4, 1 }, { 6, 1 }, { 8, 1 },
    { 2, 2 }, { 4, 2 }, { 6, 2 },
    { 0, 3 }, { 2, 3 }, { 4, 3 }, { 6, 3 }, { 8, 3 } }

local lucky_piles            = {
    Info       = {
        Name          = "Lucky Piles",
        Family        = "Fan",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size   = 4,
        Create = function(i)
            return {
                Position = { x = (i + 0.5) * 2, y = 0 },
                Rule = rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size   = 13,
        Create = function(i)
            return {
                Position = { x = lucky_piles_pos[i + 1][1], y = lucky_piles_pos[i + 1][2] },
                Initial = ops.Initial.face_up(4),
                Layout = "Row",
                Rule = { Base = rules.Base.King, Build = rules.Build.UpOrDownInSuit(), Move = rules.Move.Top() }
            }
        end
    }
}

------

local scotch_patience        = Sol.copy(fan)
scotch_patience.Info.Name    = "Scotch Patience"
scotch_patience.Foundation   = {
    Size   = 4,
    Create = { Rule = rules.ace_upac_top }
}
scotch_patience.Tableau      = {
    Size   = 18,
    Create = function(i)
        return {
            Initial = ops.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = rules.none_downrank_top
        }
    end
}

------

local shamrocks              = Sol.copy(fan)
shamrocks.Info.Name          = "Shamrocks"
shamrocks.Tableau            = {
    Size   = 18,
    Create = function(i)
        return {
            Initial = ops.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Base = rules.Base.None, Build = rules.Build.UpOrDownByRank(), Move = rules.Move.Top(), Limit = 3 }
        }
    end
}

------

local shamrocks_2            = Sol.copy(fan)
shamrocks_2.Info.Name        = "Shamrocks II"
shamrocks_2.Tableau          = {
    Size   = 18,
    Create = function(i)
        return {
            Initial = ops.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Base = rules.Base.None, Build = rules.Build.UpOrDownByRank(), Move = rules.Move.Top(), Limit = 3 }
        }
    end
}
shamrocks_2.on_after_shuffle = ops.Shuffle.kings_to_bottom

------

local troika                 = Sol.copy(fan)
troika.Info.Name             = "Troika"
troika.Tableau               = {
    Size   = 18,
    Create = function(i)
        return {
            Initial = ops.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Base = rules.Base.None, Build = rules.Build.InRank(), Move = rules.Move.Top(), Limit = 3 }
        }
    end
}
troika.on_shuffle            = function(game, card, pileType)
    if pileType == "Tableau" and card.Rank == "Ace" then
        return game.PlaceTop(card, game.Foundation, true)
    end

    return false
end

------------------------

Sol.register_game(fan)
Sol.register_game(bear_river)
Sol.register_game(box_fan)
Sol.register_game(ceiling_fan)
Sol.register_game(clover_leaf)
Sol.register_game(quads)
Sol.register_game(quads_plus)
Sol.register_game(lucky_piles)
Sol.register_game(scotch_patience)
Sol.register_game(shamrocks)
Sol.register_game(shamrocks_2)
Sol.register_game(troika)
