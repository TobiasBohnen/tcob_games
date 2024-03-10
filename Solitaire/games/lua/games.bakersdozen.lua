-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local piles = require 'base/piles'


local bakers_dozen                     = {
    Info             = {
        Name          = "Baker's Dozen",
        Type          = "OpenPacker",
        Family        = "BakersDozen",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation       = {
        Size   = 4,
        create = piles.ace_upsuit_top
    },
    Tableau          = {
        Size   = 13,
        create = {
            Initial = piles.Initial.face_up(4),
            Layout = "Column",
            Rule = { Build = "DownByRank", Move = "Top", Empty = "None" }
        }
    },
    on_after_shuffle = ops.Shuffle.kings_to_bottom,
    on_created       = Sol.Layout.bakers_dozen
}

------

local bakers_dozen_two_decks           = Sol.copy(bakers_dozen)
bakers_dozen_two_decks.Info.Name       = "Baker's Dozen (2 Decks)"
bakers_dozen_two_decks.Info.DeckCount  = 2
bakers_dozen_two_decks.Foundation.Size = 8
bakers_dozen_two_decks.Tableau.Size    = 26

------

local good_measure                     = Sol.copy(bakers_dozen)
good_measure.Info.Name                 = "Good Measure"
good_measure.Tableau                   = {
    Size   = 10,
    create = {
        Initial = piles.Initial.face_up(5),
        Layout = "Column",
        Rule = { Build = "DownByRank", Move = "Top", Empty = "None" }
    }
}
good_measure.on_before_shuffle         = function(game, card)
    if card.Rank == "Ace" then
        return game.PlaceTop(card, game.Foundation, 1, 2, true)
    end

    return false
end

------

local capricieuse                      = {
    Info              = {
        Name          = "Capricieuse",
        Type          = "OpenPacker",
        Family        = "BakersDozen",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 2
    },
    Stock             = {},
    Foundation        = {
        Size   = 8,
        create = function(i)
            if i < 4 then
                return { Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" } }
            else
                return { Rule = { Build = "DownInSuit", Move = "Top", Empty = "King" } }
            end
        end
    },
    Tableau           = {
        Size   = 12,
        create = {
            Initial = piles.Initial.face_up(8),
            Layout = "Column",
            Rule = { Build = "UpOrDownInSuit", Move = "Top", Empty = "Any" }
        }
    },
    on_redeal         = function(game)
        local cards = Sol.shuffle_tableau(game)
        if #cards == 0 then return false end

        local tableau = game.Tableau
        local tabIdx = 1
        while #cards > 0 do
            game.PlaceTop(table.remove(cards), tableau[tabIdx], false)
            tabIdx = tabIdx + 1
            if tabIdx > #tableau then tabIdx = 1 end
        end

        return true
    end,
    on_before_shuffle = function(game, card)
        if card.Rank == "Ace" then
            return game.PlaceTop(card, game.Foundation, 1, 4, true)
        end
        if card.Rank == "King" then
            return game.PlaceTop(card, game.Foundation, 5, 4, true)
        end

        return false
    end,
    on_created        = Sol.Layout.capricieuse
}

------

local strata                           = Sol.copy(capricieuse)
strata.Info.Name                       = "Strata"
strata.Foundation.create               = piles.ace_upsuit_top
strata.Tableau                         = {
    Size   = 8,
    create = {
        Initial = piles.Initial.face_up(8),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "Top", Empty = "Any" }
    }
}
strata.on_before_shuffle               = function(_, card)
    local rank = card.Rank
    return rank == "Two" or rank == "Three" or rank == "Four" or rank == "Five" or rank == "Six"
end

------

local cruel                            = Sol.copy(capricieuse)
cruel.Info.Name                        = "Cruel"
cruel.Info.DeckCount                   = 1
cruel.Info.Redeals                     = -1
cruel.Foundation                       = {
    Size   = 4,
    create = piles.ace_upsuit_top
}
cruel.Tableau                          = {
    Size   = 12,
    create = {
        Initial = piles.Initial.face_up(4),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "Top", Empty = "None" }
    }
}
cruel.on_before_shuffle                = ops.Shuffle.ace_to_foundation
cruel.on_redeal                        = function(game)
    local cards = {}

    local tableau = game.Tableau
    for _, tab in ipairs(tableau) do
        for j = #tab.Cards, 1, -1 do
            cards[#cards + 1] = tab.Cards[j]
        end
        tab:clear()
    end

    if #cards == 0 then return false end

    for _, tab in ipairs(tableau) do
        for _ = 1, 4 do
            game.PlaceBottom(table.remove(cards, 1), tab, false)
        end
        if #cards == 0 then break end
    end

    return true
end
cruel.on_created                       = Sol.Layout.bakers_dozen

------

local indefatigable                    = Sol.copy(cruel)
indefatigable.Info.Name                = "Indefatigable"
indefatigable.Info.Redeals             = 2
indefatigable.Tableau.create           = {
    Initial = piles.Initial.face_up(4),
    Layout = "Column",
    Rule = { Build = "UpOrDownInSuit", Move = "Top", Empty = "Any" }
}

------

local perseverance                     = Sol.copy(cruel)
perseverance.Info.Name                 = "Perseverance"
perseverance.Info.Redeals              = 2
perseverance.Tableau.create            = {
    Initial = piles.Initial.face_up(4),
    Layout = "Column",
    Rule = { Build = "DownInSuit", Move = "InSequence", Empty = "None" }
}

------

local royal_family                     = Sol.copy(cruel)
royal_family.Info.Name                 = "Royal Family"
royal_family.Info.Redeals              = 1
royal_family.Foundation.create         = { Rule = { Build = "DownInSuit", Move = "Top", Empty = "King" } }
royal_family.Tableau.create            = {
    Initial = piles.Initial.face_up(4),
    Layout = "Column",
    Rule = { Build = "UpOrDownAlternateColors", Move = "Top", Empty = "Any" }
}
royal_family.on_before_shuffle         = ops.Shuffle.king_to_foundation

------

local ripple_fan                       = Sol.copy(cruel)
ripple_fan.Info.Name                   = "Ripple Fan"
ripple_fan.Tableau.Size                = 13
ripple_fan.on_before_shuffle           = nil

------

local unusual                          = Sol.copy(cruel)
unusual.Info.Name                      = "Unusual"
unusual.Info.DeckCount                 = 2
unusual.Foundation.Size                = 8
unusual.Tableau.Size                   = 24

------

local fifteen                          = {
    Info       = {
        Name          = "Fifteen",
        Type          = "OpenPacker",
        Family        = "BakersDozen",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size   = 8,
        create = piles.ace_upsuit_top
    },
    Tableau    = {
        Size   = 15,
        create = function(i)
            return {
                Initial = piles.Initial.face_up(i ~= 14 and 7 or 6),
                Layout = "Column",
                Rule = { Build = "UpOrDownInSuit", Move = "Top", Empty = "Any" }
            }
        end
    },
    on_created = Sol.Layout.canister
}

------

local nationale                        = {
    Info              = {
        Name          = "Nationale",
        Type          = "OpenPacker",
        Family        = "BakersDozen",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation        = {
        Size   = 8,
        create = function(i)
            if i < 4 then
                return { Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" } }
            else
                return { Rule = { Build = "DownInSuit", Move = "Top", Empty = "King" } }
            end
        end
    },
    Tableau           = {
        Size   = 12,
        create = {
            Initial = piles.Initial.face_up(8),
            Layout = "Column",
            Rule = { Build = "UpOrDownInSuit", Move = "Top", Empty = "Any" }
        }
    },
    on_before_shuffle = function(game, card)
        if card.Rank == "Ace" then
            return game.PlaceTop(card, game.Foundation, 1, 4, true)
        end
        if card.Rank == "King" then
            return game.PlaceTop(card, game.Foundation, 5, 4, true)
        end

        return false
    end,
    on_created        = Sol.Layout.canister
}

------

local castles_in_spain                 = {
    Info       = {
        Name          = "Castles In Spain",
        Type          = "Packer",
        Family        = "BakersDozen",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size   = 4,
        create = piles.ace_upsuit_top
    },
    Tableau    = {
        Size   = 13,
        create = {
            Initial = piles.Initial.top_face_up(4),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "Top", Empty = "Any" }
        }
    },
    on_created = Sol.Layout.bakers_dozen
}

------

local martha                           = Sol.copy(castles_in_spain)
martha.Info.Name                       = "Martha"
martha.Tableau                         = {
    Size   = 12,
    create = {
        Initial = piles.Initial.alternate(4, false),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "AnySingle" }
    }
}
martha.on_before_shuffle               = ops.Shuffle.ace_to_foundation

------

local portuguese_solitaire             = Sol.copy(castles_in_spain)
portuguese_solitaire.Info.Name         = "Portuguese Solitaire"
portuguese_solitaire.Info.Type         = "OpenPacker"
portuguese_solitaire.Tableau.create    = {
    Initial = piles.Initial.face_up(4),
    Layout = "Column",
    Rule = { Build = "DownByRank", Move = "Top", Empty = "King" }
}

------

local spanish_patience                 = Sol.copy(castles_in_spain)
spanish_patience.Info.Name             = "Spanish Patience"
spanish_patience.Info.Type             = "OpenPacker"
spanish_patience.Foundation.create     = { Rule = { Build = "UpAlternateColors", Move = "Top", Empty = "Ace" } }
spanish_patience.Tableau.create        = {
    Initial = piles.Initial.face_up(4),
    Layout = "Column",
    Rule = { Build = "DownByRank", Move = "Top", Empty = "None" }
}

------

local spanish_patience_2               = Sol.copy(castles_in_spain)
spanish_patience_2.Info.Name           = "Spanish Patience II"
spanish_patience_2.Info.Type           = "OpenPacker"
spanish_patience_2.Tableau.create      = {
    Initial = piles.Initial.face_up(4),
    Layout = "Column",
    Rule = { Build = "DownByRank", Move = "Top", Empty = "Any" }
}

------

local vineyard                         = Sol.copy(castles_in_spain)
vineyard.Info.Name                     = "Vineyard"
vineyard.Info.Type                     = "OpenPacker"
vineyard.Tableau                       = {
    Size   = 10,
    create = function(i)
        return {
            Initial = piles.Initial.face_up(i < 2 and 6 or 5),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "Top", Empty = "Any" }
        }
    end
}

------------------------

Sol.register_game(bakers_dozen)
Sol.register_game(bakers_dozen_two_decks)
Sol.register_game(capricieuse)
Sol.register_game(castles_in_spain)
Sol.register_game(cruel)
Sol.register_game(fifteen)
Sol.register_game(good_measure)
Sol.register_game(indefatigable)
Sol.register_game(martha)
Sol.register_game(nationale)
Sol.register_game(perseverance)
Sol.register_game(portuguese_solitaire)
Sol.register_game(ripple_fan)
Sol.register_game(royal_family)
Sol.register_game(spanish_patience)
Sol.register_game(spanish_patience_2)
Sol.register_game(strata)
Sol.register_game(unusual)
Sol.register_game(vineyard)
