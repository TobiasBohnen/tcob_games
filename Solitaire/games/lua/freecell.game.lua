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
        create = function() return { Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" } } end
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
        create = function() return { Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" } } end
    },
    Foundation = {
        Size   = 8,
        create = function() return { Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" } } end
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
double_free_cell_2.Foundation.create = function() return { Rule = { Build = "UpInSuit", Wrap = true, Move = "Top", Empty = "Ace" } } end
double_free_cell_2.Tableau           = {
    Size   = 10,
    create = function()
        return {
            Initial = piles.initial.face_up(10),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "SuperMove", Empty = "Any" }
        }
    end
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
        create = function() return { Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" } } end
    },
    Foundation = {
        Size   = 12,
        create = function() return { Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" } } end
    },
    Tableau    = {
        Size   = 13,
        create = function()
            return {
                Initial = piles.initial.face_up(12),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "SuperMove", Empty = "Any" }
            }
        end
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
        create = function() return { Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" } } end
    },
    Foundation = {
        Size   = 12,
        create = function() return { Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" } } end
    },
    Tableau    = {
        Size   = 13,
        create = function()
            return {
                Initial = piles.initial.face_up(12),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "SuperMove", Empty = "Any" }
            }
        end
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
        create = function() return { Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" } } end
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
        return game.put_card(card, game.Tableau, 0, 4)
    end
    if card.Rank == "Ace" then
        return game.put_card(card, game.Tableau, 4, 4)
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
    create = function()
        return {
            Initial = piles.initial.face_up(1),
            Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" }
        }
    end
}
clink.Foundation                     = {
    Size = 2,
    create = function() return { Rule = { Build = "UpInSuit", Wrap = true, Move = "Top", Empty = "Ace" } } end
}
clink.Tableau                        = {
    Size = 8,
    create = function()
        return {
            Initial = piles.initial.face_up(6),
            Layout  = "Column",
            Rule    = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
        }
    end
}
clink.before_shuffle                 = function(game, card)
    if card.Rank == "Ace" and (card.Suit == "Clubs" or card.Suit == "Hearts") then
        return game.put_card(card, game.Foundation)
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
                Position = { x = i, y = 4 },
                Initial  = i % 2 == 0 and piles.initial.face_up(1) or {},
                Rule     = { Build = "NoBuilding", Move = "Top", Empty = "Any" }
            }
        end
    },
    Foundation = {
        Size   = 4,
        create = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" }
            }
        end
    },
    Tableau    = {
        Size   = 8,
        create = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial = piles.initial.face_up(6),
                Layout = "Column",
                Rule = { Build = "DownInSuit", Move = "SuperMove", Empty = "King" }
            }
        end
    }
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
        create = function(i)
            return {
                Position = { x = i, y = 3 },
                Rule     = { Build = "NoBuilding", Move = "Top", Empty = "Any" }
            }
        end
    },
    Foundation    = {
        Size   = 4,
        create = function(i)
            return {
                Position = { x = 7, y = i },
                Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" }
            }
        end
    },
    Tableau       = {
        Size   = 7,
        create = function(i)
            return {
                Position = { x = i, y = 0 },
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
    end
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
fore_cell.FreeCell.create            = function()
    return {
        Initial = piles.initial.face_up(1),
        Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" }
    }
end
fore_cell.Tableau                    = {
    Size = 8,
    create = function()
        return {
            Initial = piles.initial.face_up(6),
            Layout = "Column",
            Rule = { Build = "DownAlternateColors", Move = "SuperMove", Empty = "King" }
        }
    end
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
        create = function(i)
            return {
                Position = { x = i, y = 3 },
                Rule     = { Build = "NoBuilding", Move = "Top", Empty = "Any" }
            }
        end
    },
    Foundation     = {
        Size   = 4,
        create = function(i)
            return {
                Position = { x = 7, y = i },
                Rule = { Build = "UpInSuit", Wrap = true, Move = "None", Empty = "FirstFoundation" }
            }
        end
    },
    Tableau        = {
        Size   = 7,
        create = function(i)
            return {
                Position = { x = i, y = 0 },
                Initial = piles.initial.face_up(i > 0 and 7 or 6),
                Layout = "Column",
                Rule = { Build = "DownInSuit", Wrap = true, Move = "InSequence", Empty = { Type = "FirstFoundation", Interval = -1 } }
            }
        end
    },
    before_shuffle = function(game, card)
        local tableau1 = game.Tableau[1]
        if tableau1.Empty then
            return game.put_card(card, game.Tableau)
        else
            if card.Rank == tableau1.Cards[1].Rank then
                return game.put_card(card, game.Foundation)
            end
        end
        return false
    end
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
        local tableau = game.Tableau
        local tabIdx = 2;
        local tableau1 = tableau[1]
        for cardIdx = tableau1.CardCount, 2, -1 do
            local card = tableau1.Cards[cardIdx]
            if card.Rank == "King" then
                tableau1:move_cards(tableau[tabIdx], cardIdx, tableau1.CardCount - cardIdx + 1, false);
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

register_game(free_cell)
register_game(free_cell_0)
register_game(free_cell_2)
register_game(double_free_cell)
register_game(double_free_cell_2)
register_game(triple_free_cell)
register_game(relaxed_free_cell)
register_game(challenge_free_cell)
register_game(super_challenge_free_cell)
register_game(bakers_game)
register_game(bakers_game_ko)
register_game(bath)
register_game(big_cell)
register_game(cell_11)
register_game(cats_tail)
register_game(clink)
register_game(deep)
register_game(eight_off)
register_game(flipper)
register_game(footling)
register_game(fore_cell)
register_game(four_colours)
register_game(german_free_cell)
register_game(king_cell)
register_game(penguin)
register_game(snake)
