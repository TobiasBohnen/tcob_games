-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local piles = require 'base/piles'
local rules = require 'base/rules'


local fan                    = {
    Info       = {
        Name          = "Fan",
        Type          = "OpenPacker",
        Family        = "Fan",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size   = 4,
        create = piles.ace_upsuit_top
    },
    Tableau    = {
        Size   = 18,
        create = function(i)
            return {
                Initial = piles.Initial.face_up(i < 17 and 3 or 1),
                Layout = "Row",
                Rule = { Build = "DownInSuit", Move = "Top", Empty = rules.Empty.king }
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
    create = function(i)
        return {
            Initial = piles.Initial.face_up(i == 0 and 1 or 0),
            Rule = { Build = "UpInSuit", Wrap = true, Move = "None", Empty = function(game) return rules.Empty.first_foundation(game) end }
        }
    end
}
bear_river.Tableau           = {
    Size   = 18,
    create = function(i)
        local lastInRow = i % 6 == 5
        return {
            Initial = piles.Initial.face_up(lastInRow and 2 or 3),
            Layout = "Row",
            Rule = { Build = "UpOrDownInSuit", Wrap = true, Move = "Top", Empty = lastInRow and "Any" or "None", Limit = 3 }
        }
    end
}
bear_river.on_created        = function(game) Sol.Layout.fan(game, 6) end

------

local box_fan                = Sol.copy(fan)
box_fan.Info.Name            = "Box Fan"
box_fan.Tableau              = {
    Size   = 16,
    create = {
        Initial = piles.Initial.face_up(3),
        Layout = "Row",
        Rule = { Build = "DownAlternateColors", Move = "Top", Empty = rules.Empty.king }
    }
}
box_fan.on_before_shuffle    = ops.Shuffle.ace_to_foundation
box_fan.on_created           = function(game) Sol.Layout.fan(game, 4) end

------

local ceiling_fan            = Sol.copy(fan)
ceiling_fan.Info.Name        = "Ceiling Fan"
ceiling_fan.Tableau          = {
    Size   = 18,
    create = function(i)
        return {
            Initial = piles.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Build = "DownAlternateColors", Move = "Top", Empty = rules.Empty.king }
        }
    end
}

------

local clover_leaf            = {
    Info              = {
        Name          = "Clover Leaf",
        Type          = "OpenPacker",
        Family        = "Fan",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation        = {
        Size   = 4,
        create = function(i)
            if i < 2 then
                return { Rule = { Build = "UpInSuit", Move = "Top", Empty = function() return rules.Empty.card_color("Black", "Ace") end } }
            else
                return { Rule = { Build = "DownInSuit", Move = "Top", Empty = function() return rules.Empty.card_color("Red", "King") end } }
            end
        end
    },
    Tableau           = {
        Size   = 16,
        create = {
            Initial = piles.Initial.face_up(3),
            Layout = "Row",
            Rule = { Build = "UpOrDownInSuit", Move = "Top", Empty = function() return rules.Empty.ranks({ "Ace", "King" }) end }
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
    create = {
        Initial = piles.Initial.face_up(4),
        Layout = "Row",
        Rule = { Build = "InRank", Move = "Top", Empty = rules.Empty.any, Limit = 4 }
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
    create = function(i)
        return {
            Initial = piles.Initial.face_up(i < 12 and 4 or 0),
            Layout = "Row",
            Rule = { Build = "InRank", Move = "Top", Empty = rules.Empty.any, Limit = 4 }
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
        Type          = "OpenPacker",
        Family        = "Fan",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size   = 4,
        create = function(i)
            return {
                Position = { x = (i + 0.5) * 2, y = 0 },
                Rule = { Build = "UpInSuit", Move = "Top", Empty = rules.Empty.ace }
            }
        end
    },
    Tableau    = {
        Size   = 13,
        create = function(i)
            return {
                Position = { x = lucky_piles_pos[i + 1][1], y = lucky_piles_pos[i + 1][2] },
                Initial = piles.Initial.face_up(4),
                Layout = "Row",
                Rule = { Build = "UpOrDownInSuit", Move = "Top", Empty = rules.Empty.king }
            }
        end
    }
}

------

local scotch_patience        = Sol.copy(fan)
scotch_patience.Info.Name    = "Scotch Patience"
scotch_patience.Foundation   = {
    Size   = 4,
    create = { Rule = { Build = "UpAlternateColors", Move = "Top", Empty = rules.Empty.ace } }
}
scotch_patience.Tableau      = {
    Size   = 18,
    create = function(i)
        return {
            Initial = piles.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Build = "DownByRank", Move = "Top", Empty = rules.Empty.none }
        }
    end
}

------

local shamrocks              = Sol.copy(fan)
shamrocks.Info.Name          = "Shamrocks"
shamrocks.Tableau            = {
    Size   = 18,
    create = function(i)
        return {
            Initial = piles.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Build = "UpOrDownByRank", Move = "Top", Empty = rules.Empty.none, Limit = 3 }
        }
    end
}

------

local shamrocks_2            = Sol.copy(fan)
shamrocks_2.Info.Name        = "Shamrocks II"
shamrocks_2.Tableau          = {
    Size   = 18,
    create = function(i)
        return {
            Initial = piles.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Build = "UpOrDownByRank", Move = "Top", Empty = rules.Empty.none, Limit = 3 }
        }
    end
}
shamrocks_2.on_after_shuffle = ops.Shuffle.kings_to_bottom

------

local troika                 = Sol.copy(fan)
troika.Info.Name             = "Troika"
troika.Tableau               = {
    Size   = 18,
    create = function(i)
        return {
            Initial = piles.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Build = "InRank", Move = "Top", Empty = rules.Empty.none, Limit = 3 }
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
