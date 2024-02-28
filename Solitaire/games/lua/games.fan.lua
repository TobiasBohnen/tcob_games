-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local piles = require 'base/piles'


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
                Initial = piles.initial.face_up(i < 17 and 3 or 1),
                Layout = "Row",
                Rule = { Build = "DownInSuit", Move = "Top", Empty = "King" }
            }
        end
    },
    on_created = function(game) Layout.fan(game, 5) end
}

------

local bear_river             = Copy(fan)
bear_river.Info.Name         = "Bear River"
bear_river.Foundation        = {
    Size   = 4,
    create = function(i)
        return {
            Initial = piles.initial.face_up(i == 0 and 1 or 0),
            Rule = { Build = "UpInSuit", Wrap = true, Move = "None", Empty = "FirstFoundation" }
        }
    end
}
bear_river.Tableau           = {
    Size   = 18,
    create = function(i)
        local lastInRow = i % 6 == 5
        return {
            Initial = piles.initial.face_up(lastInRow and 2 or 3),
            Layout = "Row",
            Rule = { Build = "UpOrDownInSuit", Wrap = true, Move = "Top", Empty = lastInRow and "Any" or "None", Limit = 3 }
        }
    end
}
bear_river.on_created        = function(game) Layout.fan(game, 6) end

------

local box_fan                = Copy(fan)
box_fan.Info.Name            = "Box Fan"
box_fan.Tableau              = {
    Size   = 16,
    create = {
        Initial = piles.initial.face_up(3),
        Layout = "Row",
        Rule = { Build = "DownAlternateColors", Move = "Top", Empty = "King" }
    }
}
box_fan.on_before_shuffle    = ops.shuffle.ace_to_foundation
box_fan.on_created           = function(game) Layout.fan(game, 4) end

------

local ceiling_fan            = Copy(fan)
ceiling_fan.Info.Name        = "Ceiling Fan"
ceiling_fan.Tableau          = {
    Size   = 18,
    create = function(i)
        return {
            Initial = piles.initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Build = "DownAlternateColors", Move = "Top", Empty = "King" }
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
                return { Rule = { Build = "UpInSuit", Move = "Top", Empty = { Type = "Card", Color = "Black", Rank = "Ace" } } }
            else
                return { Rule = { Build = "DownInSuit", Move = "Top", Empty = { Type = "Card", Color = "Red", Rank = "King" } } }
            end
        end
    },
    Tableau           = {
        Size   = 16,
        create = {
            Initial = piles.initial.face_up(3),
            Layout = "Row",
            Rule = { Build = "UpOrDownInSuit", Move = "Top", Empty = { Type = "Ranks", Ranks = { "Ace", "King" } } }
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
    on_created        = function(game) Layout.fan(game, 4) end
}

------

local quads                  = Copy(fan)
quads.Info.Name              = "Quads"
quads.Tableau                = {
    Size   = 13,
    create = {
        Initial = piles.initial.face_up(4),
        Layout = "Row",
        Rule = { Build = "InRank", Move = "Top", Empty = "Any", Limit = 4 }
    }
}
quads.on_shuffle             = function(game, card, pileType)
    if pileType == "Tableau" and card.Rank == "Ace" then
        return game.PlaceTop(card, game.Foundation, true)
    end

    return false
end

------

local quads_plus             = Copy(fan)
quads_plus.Info.Name         = "Quads+"
quads_plus.Tableau           = {
    Size   = 13,
    create = function(i)
        return {
            Initial = piles.initial.face_up(i < 12 and 4 or 0),
            Layout = "Row",
            Rule = { Build = "InRank", Move = "Top", Empty = "Any", Limit = 4 }
        }
    end
}
quads_plus.on_before_shuffle = ops.shuffle.ace_to_foundation

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
                Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" }
            }
        end
    },
    Tableau    = {
        Size   = 13,
        create = function(i)
            return {
                Position = { x = lucky_piles_pos[i + 1][1], y = lucky_piles_pos[i + 1][2] },
                Initial = piles.initial.face_up(4),
                Layout = "Row",
                Rule = { Build = "UpOrDownInSuit", Move = "Top", Empty = "King" }
            }
        end
    }
}

------

local scotch_patience        = Copy(fan)
scotch_patience.Info.Name    = "Scotch Patience"
scotch_patience.Foundation   = {
    Size   = 4,
    create = { Rule = { Build = "UpAlternateColors", Move = "Top", Empty = "Ace" } }
}
scotch_patience.Tableau      = {
    Size   = 18,
    create = function(i)
        return {
            Initial = piles.initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Build = "DownByRank", Move = "Top", Empty = "None" }
        }
    end
}

------

local shamrocks              = Copy(fan)
shamrocks.Info.Name          = "Shamrocks"
shamrocks.Tableau            = {
    Size   = 18,
    create = function(i)
        return {
            Initial = piles.initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Build = "UpOrDownByRank", Move = "Top", Empty = "None", Limit = 3 }
        }
    end
}

------

local shamrocks_2            = Copy(fan)
shamrocks_2.Info.Name        = "Shamrocks II"
shamrocks_2.Tableau          = {
    Size   = 18,
    create = function(i)
        return {
            Initial = piles.initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Build = "UpOrDownByRank", Move = "Top", Empty = "None", Limit = 3 }
        }
    end
}
shamrocks_2.on_after_shuffle = ops.shuffle.kings_to_bottom

------

local troika                 = Copy(fan)
troika.Info.Name             = "Troika"
troika.Tableau               = {
    Size   = 18,
    create = function(i)
        return {
            Initial = piles.initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Build = "InRank", Move = "Top", Empty = "None", Limit = 3 }
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

RegisterGame(fan)
RegisterGame(bear_river)
RegisterGame(box_fan)
RegisterGame(ceiling_fan)
RegisterGame(clover_leaf)
RegisterGame(quads)
RegisterGame(quads_plus)
RegisterGame(lucky_piles)
RegisterGame(scotch_patience)
RegisterGame(shamrocks)
RegisterGame(shamrocks_2)
RegisterGame(troika)
