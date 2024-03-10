-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local piles = require 'base/piles'


local gypsy                           = {
    Info       = {
        Name          = "Gypsy",
        Type          = "Packer",
        Family        = "Gypsy",
        DeckCount     = 2,
        CardDealCount = 8,
        Redeals       = 0
    },
    Stock      = {
        Initial = piles.Initial.face_down(80)
    },
    Foundation = {
        Size   = 8,
        create = piles.ace_upsuit_top
    },
    Tableau    = {
        Size   = 8,
        create = {
            Initial = piles.Initial.top_face_up(3),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
        }
    },
    on_created = Sol.Layout.gypsy,
    on_deal    = function(game) return game.Stock[1]:deal_to_group(game.Tableau, false) end
}

------

local agnes_sorel                     = Sol.copy(gypsy)
agnes_sorel.Info.Name                 = "Agnes Sorel"
agnes_sorel.Info.DeckCount            = 1
agnes_sorel.Info.CardDealCount        = 7
agnes_sorel.Stock.Initial             = piles.Initial.face_down(23)
agnes_sorel.Foundation                = {
    Size   = 4,
    create = function(i)
        return {
            Initial = i == 0 and piles.Initial.face_up(1) or {},
            Rule = { Build = "UpInSuit", Wrap = true, Move = "None", Empty = "FirstFoundation" }
        }
    end
}
agnes_sorel.Tableau                   = {
    Size   = 7,
    create = function(i)
        return {
            Initial = piles.Initial.face_up(i + 1),
            Layout = "Column",
            Rule = { Build = "DownInColor", Move = "InSequence", Empty = "None" }
        }
    end
}
agnes_sorel.on_created                = Sol.Layout.klondike

------

local blockade                        = Sol.copy(gypsy)
blockade.Info.Name                    = "Blockade"
blockade.Info.CardDealCount           = 12
blockade.Stock.Initial                = piles.Initial.face_down(92)
blockade.Tableau                      = {
    Size   = 12,
    create = {
        Initial = piles.Initial.face_up(1),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "InSequence", Empty = "Any" }
    }
}
blockade.on_created                   = Sol.Layout.klondike
blockade.on_change                    = function(game) game.Stock[1]:deal_to_group(game.Tableau, true) end

------

local die_koenigsbergerin             = Sol.copy(gypsy)
die_koenigsbergerin.Info.Name         = "Die Königsbergerin"
die_koenigsbergerin.Tableau.create    = {
    Initial = piles.Initial.face_up(3),
    Layout = "Column",
    Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
}
die_koenigsbergerin.Foundation.create = { Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" } }
die_koenigsbergerin.on_shuffle        = function(game, card, pileType)
    if pileType == "Tableau" and card.Rank == "Ace" then
        return game.PlaceTop(card, game.Foundation, true)
    end

    return false
end
die_koenigsbergerin.on_after_shuffle  = function(game)
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

    return false
end

------

local elba                            = Sol.copy(gypsy)
elba.Info.Name                        = "Elba"
elba.Info.CardDealCount               = 10
elba.Stock.Initial                    = piles.Initial.face_down(54)
elba.Tableau                          = {
    Size   = 10,
    create = {
        Initial = piles.Initial.top_face_up(5),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "King" }
    }
}
elba.on_created                       = Sol.Layout.klondike

------

local hypotenuse                      = Sol.copy(gypsy)
hypotenuse.Info.Name                  = "Hypotenuse"
hypotenuse.Info.CardDealCount         = 10
hypotenuse.Stock.Initial              = piles.Initial.face_down(49)
hypotenuse.Tableau                    = {
    Size   = 10,
    create = function(i)
        return {
            Initial = piles.Initial.top_face_up(10 - i),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "King" }
        }
    end
}
hypotenuse.on_created                 = Sol.Layout.klondike

------

local irmgard                         = Sol.copy(gypsy)
irmgard.Info.Name                     = "Irmgard"
irmgard.Info.CardDealCount            = 9
irmgard.Stock.Initial                 = piles.Initial.face_down(79)
irmgard.Tableau                       = {
    Size   = 9,
    create = function(i)
        return {
            Initial = piles.Initial.top_face_up(i < 5 and i + 1 or 9 - i),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "King" }
        }
    end
}
irmgard.on_created                    = Sol.Layout.klondike

------

local lexington_harp                  = Sol.copy(gypsy)
lexington_harp.Info.Name              = "Lexington Harp"
--lexington_harp.Info.Family = "Gypsy/Yukon"
lexington_harp.Stock.Initial          = piles.Initial.face_down(68)
lexington_harp.Foundation             = {
    Size   = 8,
    create = { Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" } }
}
lexington_harp.Tableau                = {
    Size   = 8,
    create = function(i)
        return {
            Initial = piles.Initial.top_face_up(i + 1),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "FaceUp", Empty = "Any" }
        }
    end
}

------

local brunswick                       = Sol.copy(lexington_harp)
brunswick.Info.Name                   = "Brunswick"
--brunswick.Info.Family = "Gypsy/Yukon"
brunswick.Tableau.create              = function(i)
    return {
        Initial = piles.Initial.face_up(i + 1),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "FaceUp", Empty = "Any" }
    }
end

------

local milligan_harp                   = Sol.copy(lexington_harp)
milligan_harp.Info.Name               = "Milligan Harp"
--milligan_harp.Info.Family = "Gypsy/Yukon"
milligan_harp.Tableau.create          = function(i)
    return {
        Initial = piles.Initial.top_face_up(i + 1),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
    }
end

------

local carlton                         = Sol.copy(lexington_harp)
carlton.Info.Name                     = "Carlton"
carlton.Tableau.create                = function(i)
    return {
        Initial = piles.Initial.face_up(i + 1),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
    }
end

------

local mississippi                     = Sol.copy(lexington_harp)
mississippi.Info.Name                 = "Mississippi"
--milligan_harp.Info.Family = "Gypsy/Yukon"
mississippi.Stock.Initial             = piles.Initial.face_down(76)
mississippi.Tableau                   = {
    Size   = 7,
    create = function(i)
        return {
            Initial = piles.Initial.top_face_up(i + 1),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "FaceUp", Empty = "Any" }
        }
    end
}

------

local cone                            = {
    Info       = {
        Name          = "Cone",
        Type          = "Packer",
        Family        = "Gypsy",
        DeckCount     = 2,
        CardDealCount = 7,
        Redeals       = 0
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = piles.Initial.face_down(88)
    },
    Reserve    = {
        Size   = 4,
        create = function(i)
            return {
                Position = { x = 0, y = i + 1 },
                Rule = { Build = "NoBuilding", Move = "Top", Empty = "None" }
            }
        end
    },
    Foundation = {
        Size   = 4,
        create = function(i)
            return {
                Position = { x = 8, y = i + 1 },
                Rule = { Build = "UpInSuit", Wrap = true, Move = "Top", Empty = "Ace" }
            }
        end
    },
    Tableau    = {
        Size   = 7,
        create = function(i)
            return {
                Position = { x = i + 1, y = 0 },
                Initial = piles.Initial.top_face_up(i < 4 and i + 1 or 7 - i),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
            }
        end
    },
    on_deal    = function(game)
        --check if tableau is empty
        local tableau = game.Tableau
        for _, tab in ipairs(tableau) do
            if tab.IsEmpty then return false end
        end
        --deal last 4 cards to reserve
        if game.Stock[1].CardCount == 4 then
            return game.Stock[1]:deal_to_group(game.Reserve, false)
        end

        return game.Stock[1]:deal_to_group(game.Tableau, false)
    end
}

------

local easthaven                       = {
    Info       = {
        Name          = "Easthaven",
        Type          = "Packer",
        Family        = "Gypsy",
        DeckCount     = 1,
        CardDealCount = 7,
        Redeals       = 0
    },
    Stock      = {
        Initial = piles.Initial.face_down(31)
    },
    Foundation = {
        Size   = 4,
        create = piles.ace_upsuit_top
    },
    Tableau    = {
        Size   = 7,
        create = {
            Initial = piles.Initial.top_face_up(3),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
        }
    },
    on_created = Sol.Layout.klondike,
    on_deal    = function(game) return game.Stock[1]:deal_to_group(game.Tableau, false) end
}

------

local double_easthaven                = Sol.copy(easthaven)
double_easthaven.Info.Name            = "Double Easthaven"
double_easthaven.Info.DeckCount       = 2
double_easthaven.Info.CardDealCount   = 8
double_easthaven.Stock.Initial        = piles.Initial.face_down(80)
double_easthaven.Tableau.Size         = 8

------

------------------------

Sol.register_game(gypsy)
Sol.register_game(agnes_sorel)
Sol.register_game(blockade)
Sol.register_game(brunswick)
Sol.register_game(carlton)
Sol.register_game(cone)
Sol.register_game(die_koenigsbergerin)
Sol.register_game(easthaven)
Sol.register_game(double_easthaven)
Sol.register_game(elba)
Sol.register_game(hypotenuse)
Sol.register_game(irmgard)
Sol.register_game(lexington_harp)
Sol.register_game(milligan_harp)
Sol.register_game(mississippi)
