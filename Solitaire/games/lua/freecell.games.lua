-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local layout = require 'base/layout'
local ops    = require 'base/ops'
local piles  = require 'base/piles'
require 'base/common'

local free_cell                      = {
    Info       = {
        Name          = "FreeCell",
        Type          = "OpenPacker",
        Family        = "FreeCell",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell   = {
        Size   = 4,
        create = { Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" } }
    },
    Foundation = {
        Size   = 4,
        create = piles.ace_upsuit_top
    },
    Tableau    = {
        Size   = 8,
        create = function(i)
            return {
                Initial = piles.initial.face_up(i < 4 and 7 or 6),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "SuperMove", Empty = "Any" }
            }
        end
    },
    layout     = layout.free_cell
}

------

local free_cell_0                    = Copy(free_cell)
free_cell_0.Info.Name                = "FreeCell (0 cells)"
free_cell_0.FreeCell                 = nil

------

local free_cell_2                    = Copy(free_cell)
free_cell_2.Info.Name                = "FreeCell (2 cells)"
free_cell_2.FreeCell.Size            = 2

------

local double_free_cell               = {
    Info       = {
        Name          = "Double FreeCell",
        Type          = "OpenPacker",
        Family        = "FreeCell",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell   = {
        Size   = 8,
        create = { Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" } }
    },
    Foundation = {
        Size   = 8,
        create = { Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" } }
    },
    Tableau    = {
        Size   = 10,
        create = function(i)
            return {
                Initial = piles.initial.face_up(i < 4 and 11 or 10),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "SuperMove", Empty = "Any" }
            }
        end
    },
    layout     = layout.double_free_cell
}

------

local double_free_cell_2             = Copy(free_cell)
double_free_cell_2.Info.Name         = "Double FreeCell II"
double_free_cell_2.Info.DeckCount    = 2
double_free_cell_2.FreeCell.Size     = 6
double_free_cell_2.Foundation.create = { Rule = { Build = "UpInSuit", Wrap = true, Move = "Top", Empty = "Ace" } }
double_free_cell_2.Tableau           = {
    Size   = 10,
    create = {
        Initial = piles.initial.face_up(10),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "SuperMove", Empty = "Any" }
    }
}
double_free_cell_2.before_shuffle    = ops.shuffle.ace_to_foundation

------

local triple_free_cell               = {
    Info       = {
        Name          = "Triple FreeCell",
        Type          = "OpenPacker",
        Family        = "FreeCell",
        DeckCount     = 3,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell   = {
        Size   = 10,
        create = { Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" } }
    },
    Foundation = {
        Size   = 12,
        create = { Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" } }
    },
    Tableau    = {
        Size   = 13,
        create = {
            Initial = piles.initial.face_up(12),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "SuperMove", Empty = "Any" }
        }
    },
    layout     = layout.double_free_cell
}

------

local bakers_game                    = Copy(free_cell)
bakers_game.Info.Name                = "Baker's Game"
bakers_game.Tableau.create           = function(i)
    return {
        Initial = piles.initial.face_up(i < 4 and 7 or 6),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "SuperMove", Empty = "Any" }
    }
end

------

local bakers_game_ko                 = Copy(free_cell)
bakers_game_ko.Info.Name             = "King Only Baker's Game"
bakers_game_ko.Tableau.create        = function(i)
    return {
        Initial = piles.initial.face_up(i < 4 and 7 or 6),
        Layout = "Column",
        Rule = { Build = "DownInSuit", Move = "SuperMove", Empty = "King" }
    }
end

------

local bath                           = Copy(free_cell)
bath.Info.Name                       = "Bath"
bath.FreeCell.Size                   = 2
bath.Tableau                         = {
    Size = 10,
    create = function(i)
        return {
            Initial = piles.initial.face_up(math.min(i + 1, 8)),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "SuperMove", Empty = "Any" }
        }
    end
}

------

local big_cell                       = {
    Info       = {
        Name          = "Big Cell",
        Type          = "OpenPacker",
        Family        = "FreeCell",
        DeckCount     = 3,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell   = {
        Size   = 4,
        create = { Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" } }
    },
    Foundation = {
        Size   = 12,
        create = { Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" } }
    },
    Tableau    = {
        Size   = 13,
        create = {
            Initial = piles.initial.face_up(12),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "SuperMove", Empty = "Any" }
        }
    },
    layout     = layout.double_free_cell
}

------

local cell_11                        = {
    Info       = {
        Name          = "Cell 11",
        Type          = "OpenPacker",
        Family        = "FreeCell",
        DeckCount     = 3,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell   = {
        Size   = 11,
        create = function(i)
            return {
                Initial = piles.initial.face_up((i == 0 or i == 10) and 1 or 0),
                Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" }
            }
        end
    },
    Foundation = {
        Size   = 12,
        create = { Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" } }
    },
    Tableau    = {
        Size   = 13,
        create = function(i)
            return {
                Initial = piles.initial.face_up((i == 0 or i == 12) and 11 or 12),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "SuperMove", Empty = "Any" }
            }
        end
    },
    layout     = layout.double_free_cell
}

------

local challenge_free_cell            = Copy(free_cell)
challenge_free_cell.Info.Name        = "Challenge FreeCell"
challenge_free_cell.Tableau          = {
    Size = 8,
    create = function(i)
        return {
            Initial = piles.initial.face_up(i < 4 and 6 or 5),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "SuperMove", Empty = "Any" }
        }
    end
}
challenge_free_cell.before_shuffle   = function(game, card)
    if card.Rank == "Two" then
        return game.PlaceTop(card, game.Tableau, 1, 4, true)
    end
    if card.Rank == "Ace" then
        return game.PlaceTop(card, game.Tableau, 5, 4, true)
    end

    return false
end

------

local super_challenge_free_cell      = Copy(challenge_free_cell)
super_challenge_free_cell.Info.Name  = "Super Challenge FreeCell"
super_challenge_free_cell.Tableau    = {
    Size = 8,
    create = function(i)
        return {
            Initial = piles.initial.face_up(i < 4 and 6 or 5),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "SuperMove", Empty = "King" }
        }
    end
}

------

local clink                          = Copy(free_cell)
clink.Info.Name                      = "Clink"
clink.FreeCell                       = {
    Size = 2,
    create = {
        Initial = piles.initial.face_up(1),
        Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" }
    }
}
clink.Foundation                     = {
    Size = 2,
    create = { Rule = { Build = "UpInSuit", Wrap = true, Move = "Top", Empty = "Ace" } }
}
clink.Tableau                        = {
    Size = 8,
    create = {
        Initial = piles.initial.face_up(6),
        Layout  = "Column",
        Rule    = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
    }
}
clink.before_shuffle                 = function(game, card)
    if card.Rank == "Ace" and (card.Suit == "Clubs" or card.Suit == "Hearts") then
        return game.PlaceTop(card, game.Foundation, true)
    end

    return false
end

------

local deep                           = {
    Info       = {
        Name          = "Deep",
        Type          = "OpenPacker",
        Family        = "FreeCell",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell   = {
        Size   = 8,
        create = function(i)
            return {
                Position = { x = i, y = 0 },
                Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" }
            }
        end
    },
    Foundation = {
        Size   = 8,
        create = function(i)
            return {
                Position = { x = i % 2 + 8, y = i // 2 },
                Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" }
            }
        end
    },
    Tableau    = {
        Size   = 8,
        create = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial = piles.initial.face_up(13),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
            }
        end
    }
}

------

local eight_off                      = {
    Info       = {
        Name          = "Eight Off",
        Type          = "OpenPacker",
        Family        = "FreeCell",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell   = {
        Size   = 8,
        create = function(i)
            return {
                Initial = i % 2 == 0 and piles.initial.face_up(1) or {},
                Rule    = { Build = "NoBuilding", Move = "Top", Empty = "Any" }
            }
        end
    },
    Foundation = {
        Size   = 4,
        create = { Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" } }
    },
    Tableau    = {
        Size   = 8,
        create = {
            Initial = piles.initial.face_up(6),
            Layout = "Column",
            Rule = { Build = "DownInSuit", Move = "SuperMove", Empty = "King" }
        }
    },
    layout     = layout.double_free_cell
}

------

local footling                       = Copy(free_cell)
footling.Info.Name                   = "Footling"
footling.Foundation                  = { Rule = { Build = "RankPack", Move = "Top", Empty = "Ace" } }
footling.Tableau                     = {
    Size = 8,
    create = function(i)
        return {
            Initial = piles.initial.face_up(i < 4 and 7 or 6),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
        }
    end
}

------

local fore_cell                      = Copy(free_cell)
fore_cell.Info.Name                  = "ForeCell"
fore_cell.FreeCell.create            = {
    Initial = piles.initial.face_up(1),
    Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" }
}
fore_cell.Tableau                    = {
    Size = 8,
    create = {
        Initial = piles.initial.face_up(6),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "SuperMove", Empty = "King" }
    }
}

------

local four_colours                   = Copy(free_cell)
four_colours.Info.Name               = "Four Colours"
four_colours.Tableau                 = {
    Size = 7,
    create = function(i)
        return {
            Initial = i < 4 and piles.initial.face_up(13) or {},
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
        }
    end
}

------
local german_free_cell_empty         = { "Clubs", "Spades", "Hearts", "Diamonds" }

local german_free_cell               = Copy(free_cell)
german_free_cell.Info.Name           = "German FreeCell"
german_free_cell.FreeCell            = {
    Size = 4,
    create = function(i)
        return {
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = { Type = "Suits", Suits = { german_free_cell_empty[i + 1] } } }
        }
    end
}

------

local king_cell                      = Copy(free_cell)
king_cell.Info.Name                  = "KingCell"
king_cell.Tableau                    = {
    Size = 8,
    create = function(i)
        return {
            Initial = piles.initial.face_up(i < 4 and 7 or 6),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "SuperMove", Empty = "King" }
        }
    end
}

------

local flipper                        = {
    Info          = {
        Name          = "Flipper",
        Type          = "OpenPacker",
        Family        = "FreeCell",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell      = {
        Size   = 7,
        create = { Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" } }
    },
    Foundation    = {
        Size   = 4,
        create = { Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" } }
    },
    Tableau       = {
        Size   = 7,
        create = function(i)
            return {
                Initial = piles.initial.face_up(i % 3 == 0 and 8 or 7),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
            }
        end
    },
    before_layout = function(game)
        local tableau = game.Tableau
        local freeCell = game.FreeCell
        for k, v in pairs(tableau) do
            if not v.Empty then
                if freeCell[k].Empty then
                    v:flip_up_top_card()
                else
                    v:flip_down_top_card()
                end
            end
        end
    end,
    layout        = layout.flipper
}

------

local penguin                        = {
    Info           = {
        Name          = "Penguin",
        Type          = "OpenPacker",
        Family        = "FreeCell",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell       = {
        Size   = 7,
        create = { Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" } }
    },
    Foundation     = {
        Size   = 4,
        create = { Rule = { Build = "UpInSuit", Wrap = true, Move = "None", Empty = "FirstFoundation" } }
    },
    Tableau        = {
        Size   = 7,
        create = function(i)
            return {
                Initial = piles.initial.face_up(i > 0 and 7 or 6),
                Layout = "Column",
                Rule = { Build = "DownInSuit", Wrap = true, Move = "InSequence", Empty = { Type = "FirstFoundation", Interval = -1 } }
            }
        end
    },
    before_shuffle = function(game, card)
        local tableau1 = game.Tableau[1]
        if tableau1.Empty then
            return game.PlaceTop(card, game.Tableau, true)
        else
            if card.Rank == tableau1.Cards[1].Rank then
                return game.PlaceTop(card, game.Foundation, true)
            end
        end
        return false
    end,
    layout         = layout.flipper
}

------

local relaxed_free_cell              = Copy(free_cell)
relaxed_free_cell.Info.Name          = "Relaxed FreeCell"
relaxed_free_cell.Tableau            = {
    Size = 8,
    create = function(i)
        return {
            Initial = piles.initial.face_up(i < 4 and 7 or 6),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
        }
    end
}

------

local snake                          = {
    Info           = {
        Name          = "Snake",
        Type          = "OpenPacker",
        Family        = "FreeCell",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell       = {
        Size   = 7,
        create = function(i)
            return {
                Position = { x = i + 1, y = 0 },
                Rule     = { Build = "NoBuilding", Move = "Top", Empty = "Any" }
            }
        end
    },
    Foundation     = {
        Size   = 8,
        create = function(i)
            return {
                Position = { x = i % 2 + 9, y = i // 2 },
                Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" }
            }
        end
    },
    Tableau        = {
        Size   = 9,
        create = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial = piles.initial.face_up(i == 0 and 96 or 0),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "SuperMove", Empty = "None" }
            }
        end
    },
    before_shuffle = ops.shuffle.ace_to_foundation,
    after_shuffle  = function(game)
        -- kings start new pile
        local tableau = game.Tableau
        local tabIdx = 2
        local tableau1 = tableau[1]
        for cardIdx = tableau1.CardCount, 2, -1 do
            local card = tableau1.Cards[cardIdx]
            if card.Rank == "King" then
                tableau1:move_cards(tableau[tabIdx], cardIdx, tableau1.CardCount - cardIdx + 1, false)
                tabIdx = tabIdx + 1
            end
        end
    end
}

------

local cats_tail                      = Copy(snake)
cats_tail.Info.Name                  = "Cat's Tail"
cats_tail.Tableau.create             = function(i)
    return {
        Position = { x = i, y = 1 },
        Initial = piles.initial.face_up(i == 0 and 104 or 0),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "None" }
    }
end
cats_tail.before_shuffle             = nil

------------------------

RegisterGame(free_cell)
RegisterGame(free_cell_0)
RegisterGame(free_cell_2)
RegisterGame(double_free_cell)
RegisterGame(double_free_cell_2)
RegisterGame(triple_free_cell)
RegisterGame(relaxed_free_cell)
RegisterGame(challenge_free_cell)
RegisterGame(super_challenge_free_cell)
RegisterGame(bakers_game)
RegisterGame(bakers_game_ko)
RegisterGame(bath)
RegisterGame(big_cell)
RegisterGame(cell_11)
RegisterGame(cats_tail)
RegisterGame(clink)
RegisterGame(deep)
RegisterGame(eight_off)
RegisterGame(flipper)
RegisterGame(footling)
RegisterGame(fore_cell)
RegisterGame(four_colours)
RegisterGame(german_free_cell)
RegisterGame(king_cell)
RegisterGame(penguin)
RegisterGame(snake)
