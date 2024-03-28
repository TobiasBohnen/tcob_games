-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local free_cell                       = {
    Info             = {
        Name          = "FreeCell",
        Family        = "FreeCell",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell         = {
        Size = 4,
        Pile = { Rule = rules.any_none_top }
    },
    Foundation       = {
        Size = 4,
        Pile = { Rule = rules.ace_upsuit_top }
    },
    Tableau          = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = ops.Initial.face_up(i < 4 and 7 or 6),
                Layout = "Column",
                Rule = rules.any_downac_sm
            }
        end
    },
    on_piles_created = Sol.Layout.free_cell
}

------

local free_cell_0                     = Sol.copy(free_cell)
free_cell_0.Info.Name                 = "FreeCell (0 cells)"
free_cell_0.FreeCell                  = nil

------

local free_cell_2                     = Sol.copy(free_cell)
free_cell_2.Info.Name                 = "FreeCell (2 cells)"
free_cell_2.FreeCell.Size             = 2

------

local double_free_cell                = {
    Info             = {
        Name          = "Double FreeCell",
        Family        = "FreeCell",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell         = {
        Size = 8,
        Pile = { Rule = rules.any_none_top }
    },
    Foundation       = {
        Size = 8,
        Pile = { Rule = rules.ace_upsuit_top }
    },
    Tableau          = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = ops.Initial.face_up(i < 4 and 11 or 10),
                Layout = "Column",
                Rule = rules.any_downac_sm
            }
        end
    },
    on_piles_created = Sol.Layout.double_free_cell
}

------

local double_free_cell_2              = Sol.copy(free_cell)
double_free_cell_2.Info.Name          = "Double FreeCell II"
double_free_cell_2.Info.DeckCount     = 2
double_free_cell_2.FreeCell.Size      = 6
double_free_cell_2.Foundation.Pile    = { Rule = { Base = rules.Base.Ace, Build = rules.Build.UpInSuit(true), Move = rules.Move.Top() } }
double_free_cell_2.Tableau            = {
    Size = 10,
    Pile = {
        Initial = ops.Initial.face_up(10),
        Layout = "Column",
        Rule = rules.any_downac_sm
    }
}
double_free_cell_2.on_before_shuffle  = ops.Shuffle.ace_to_foundation

------

local triple_free_cell                = {
    Info             = {
        Name          = "Triple FreeCell",
        Family        = "FreeCell",
        DeckCount     = 3,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell         = {
        Size = 10,
        Pile = { Rule = rules.any_none_top }
    },
    Foundation       = {
        Size = 12,
        Pile = { Rule = rules.ace_upsuit_top }
    },
    Tableau          = {
        Size = 13,
        Pile = {
            Initial = ops.Initial.face_up(12),
            Layout = "Column",
            Rule = rules.any_downac_sm
        }
    },
    on_piles_created = Sol.Layout.double_free_cell
}

------

local bakers_game                     = Sol.copy(free_cell)
bakers_game.Info.Name                 = "Baker's Game"
bakers_game.Tableau.Pile              = function(i)
    return {
        Initial = ops.Initial.face_up(i < 4 and 7 or 6),
        Layout = "Column",
        Rule = { Base = rules.Base.Any, Build = rules.Build.DownInSuit(), Move = rules.Move.SuperMove() }
    }
end

------

local bakers_game_ko                  = Sol.copy(free_cell)
bakers_game_ko.Info.Name              = "King Only Baker's Game"
bakers_game_ko.Tableau.Pile           = function(i)
    return {
        Initial = ops.Initial.face_up(i < 4 and 7 or 6),
        Layout = "Column",
        Rule = { Base = rules.Base.King, Build = rules.Build.DownInSuit(), Move = rules.Move.SuperMove() }
    }
end

------

local bath                            = Sol.copy(free_cell)
bath.Info.Name                        = "Bath"
bath.FreeCell.Size                    = 2
bath.Tableau                          = {
    Size = 10,
    Pile = function(i)
        return {
            Initial = ops.Initial.face_up(math.min(i + 1, 8)),
            Layout = "Column",
            Rule = rules.any_downac_sm
        }
    end
}

------

local big_cell                        = {
    Info             = {
        Name          = "Big Cell",
        Family        = "FreeCell",
        DeckCount     = 3,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell         = {
        Size = 4,
        Pile = { Rule = rules.any_none_top }
    },
    Foundation       = {
        Size = 12,
        Pile = { Rule = rules.ace_upsuit_top }
    },
    Tableau          = {
        Size = 13,
        Pile = {
            Initial = ops.Initial.face_up(12),
            Layout = "Column",
            Rule = rules.any_downac_sm
        }
    },
    on_piles_created = Sol.Layout.double_free_cell
}

------

local cell_11                         = {
    Info             = {
        Name          = "Cell 11",
        Family        = "FreeCell",
        DeckCount     = 3,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell         = {
        Size = 11,
        Pile = function(i)
            return {
                Initial = ops.Initial.face_up((i == 0 or i == 10) and 1 or 0),
                Rule = rules.any_none_top
            }
        end
    },
    Foundation       = {
        Size = 12,
        Pile = { Rule = rules.ace_upsuit_top }
    },
    Tableau          = {
        Size = 13,
        Pile = function(i)
            return {
                Initial = ops.Initial.face_up((i == 0 or i == 12) and 11 or 12),
                Layout = "Column",
                Rule = rules.any_downac_sm
            }
        end
    },
    on_piles_created = Sol.Layout.double_free_cell
}

------

local challenge_free_cell             = Sol.copy(free_cell)
challenge_free_cell.Info.Name         = "Challenge FreeCell"
challenge_free_cell.Tableau           = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = ops.Initial.face_up(i < 4 and 6 or 5),
            Layout = "Column",
            Rule = rules.any_downac_sm
        }
    end
}
challenge_free_cell.on_before_shuffle = function(game, card)
    if card.Rank == "Two" then
        return game.PlaceTop(card, game.Tableau, 1, 4, true)
    end
    if card.Rank == "Ace" then
        return game.PlaceTop(card, game.Tableau, 5, 4, true)
    end

    return false
end

------

local super_challenge_free_cell       = Sol.copy(challenge_free_cell)
super_challenge_free_cell.Info.Name   = "Super Challenge FreeCell"
super_challenge_free_cell.Tableau     = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = ops.Initial.face_up(i < 4 and 6 or 5),
            Layout = "Column",
            Rule = { Base = rules.Base.King, Build = rules.Build.DownAlternateColors(), Move = rules.Move.SuperMove() }
        }
    end
}

------

local clink                           = Sol.copy(free_cell)
clink.Info.Name                       = "Clink"
clink.FreeCell                        = {
    Size = 2,
    Pile = {
        Initial = ops.Initial.face_up(1),
        Rule = rules.any_none_top
    }
}
clink.Foundation                      = {
    Size = 2,
    Pile = { Rule = { Base = rules.Base.Ace, Build = rules.Build.UpInSuit(true), Move = rules.Move.Top() } }
}
clink.Tableau                         = {
    Size = 8,
    Pile = {
        Initial = ops.Initial.face_up(6),
        Layout  = "Column",
        Rule    = rules.any_downac_inseq
    }
}
clink.on_before_shuffle               = function(game, card)
    if card.Rank == "Ace" and (card.Suit == "Clubs" or card.Suit == "Hearts") then
        return game.PlaceTop(card, game.Foundation, true)
    end

    return false
end

------

local deep                            = {
    Info       = {
        Name          = "Deep",
        Family        = "FreeCell",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell   = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i, y = 0 },
                Rule = rules.any_none_top
            }
        end
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i % 2 + 8, y = i // 2 },
                Rule = rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial = ops.Initial.face_up(13),
                Layout = "Column",
                Rule = rules.any_downac_inseq
            }
        end
    }
}

------

local eight_off                       = {
    Info             = {
        Name          = "Eight Off",
        Family        = "FreeCell",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell         = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = i % 2 == 0 and ops.Initial.face_up(1) or {},
                Rule    = rules.any_none_top
            }
        end
    },
    Foundation       = {
        Size = 4,
        Pile = { Rule = rules.ace_upsuit_top }
    },
    Tableau          = {
        Size = 8,
        Pile = {
            Initial = ops.Initial.face_up(6),
            Layout = "Column",
            Rule = { Base = rules.Base.King, Build = rules.Build.DownInSuit(), Move = rules.Move.SuperMove() }
        }
    },
    on_piles_created = Sol.Layout.double_free_cell
}

------

local footling                        = Sol.copy(free_cell)
footling.Info.Name                    = "Footling"
footling.Foundation                   = { Rule = { Base = rules.Base.Ace, Build = rules.Build.RankPack(), Move = rules.Move.Top() } }
footling.Tableau                      = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = ops.Initial.face_up(i < 4 and 7 or 6),
            Layout = "Column",
            Rule = rules.any_downac_inseq
        }
    end
}

------

local fore_cell                       = Sol.copy(free_cell)
fore_cell.Info.Name                   = "ForeCell"
fore_cell.FreeCell.Pile               = {
    Initial = ops.Initial.face_up(1),
    Rule = rules.any_none_top
}
fore_cell.Tableau                     = {
    Size = 8,
    Pile = {
        Initial = ops.Initial.face_up(6),
        Layout = "Column",
        Rule = { Base = rules.Base.King, Build = rules.Build.DownAlternateColors(), Move = rules.Move.SuperMove() }
    }
}

------

local four_colours                    = Sol.copy(free_cell)
four_colours.Info.Name                = "Four Colours"
four_colours.Tableau                  = {
    Size = 7,
    Pile = function(i)
        return {
            Initial = i < 4 and ops.Initial.face_up(13) or {},
            Layout = "Column",
            Rule = rules.any_downac_inseq
        }
    end
}

------
local german_free_cell_empty          = { "Clubs", "Spades", "Hearts", "Diamonds" }

local german_free_cell                = Sol.copy(free_cell)
german_free_cell.Info.Name            = "German FreeCell"
german_free_cell.FreeCell             = {
    Size = 4,
    Pile = function(i)
        return {
            Layout = "Column",
            Rule = { Base = function(_, card, _) return rules.Base.Suits(card, { german_free_cell_empty[i + 1] }) end, Build = rules.Build.None(), Move = rules.Move.Top() }
        }
    end
}

------

local king_cell                       = Sol.copy(free_cell)
king_cell.Info.Name                   = "KingCell"
king_cell.Tableau                     = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = ops.Initial.face_up(i < 4 and 7 or 6),
            Layout = "Column",
            Rule = { Base = rules.Base.King, Build = rules.Build.DownAlternateColors(), Move = rules.Move.SuperMove() }
        }
    end
}

------

local flipper                         = {
    Info             = {
        Name          = "Flipper",
        Family        = "FreeCell",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell         = {
        Size = 7,
        Pile = { Rule = rules.any_none_top }
    },
    Foundation       = {
        Size = 4,
        Pile = { Rule = rules.ace_upsuit_top }
    },
    Tableau          = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = ops.Initial.face_up(i % 3 == 0 and 8 or 7),
                Layout = "Column",
                Rule = rules.any_downac_inseq
            }
        end
    },
    on_end_turn      = function(game)
        local tableau = game.Tableau
        local freeCell = game.FreeCell
        for i, tab in ipairs(tableau) do
            if not tab.IsEmpty then
                if freeCell[i].IsEmpty then
                    tab:flip_up_top_card()
                else
                    tab:flip_down_top_card()
                end
            end
        end
    end,
    on_piles_created = Sol.Layout.flipper
}

------

local penguin                         = {
    Info              = {
        Name          = "Penguin",
        Family        = "FreeCell",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell          = {
        Size = 7,
        Pile = { Rule = rules.any_none_top }
    },
    Foundation        = {
        Size = 4,
        Pile = { Rule = rules.ff_upsuit_none }
    },
    Tableau           = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = ops.Initial.face_up(i > 0 and 7 or 6),
                Layout = "Column",
                Rule = {
                    Base = function(game, card, _) return rules.Base.FirstFoundation(game, card, -1) end,
                    Build = rules.Build.DownInSuit(true),
                    Move = rules.Move.InSeq()
                }
            }
        end
    },
    on_before_shuffle = function(game, card)
        local tableau1 = game.Tableau[1]
        if tableau1.IsEmpty then
            return game.PlaceTop(card, game.Tableau, true)
        else
            if card.Rank == tableau1.Cards[1].Rank then
                return game.PlaceTop(card, game.Foundation, true)
            end
        end
        return false
    end,
    on_piles_created  = Sol.Layout.flipper
}

------

local relaxed_free_cell               = Sol.copy(free_cell)
relaxed_free_cell.Info.Name           = "Relaxed FreeCell"
relaxed_free_cell.Tableau             = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = ops.Initial.face_up(i < 4 and 7 or 6),
            Layout = "Column",
            Rule = rules.any_downac_inseq
        }
    end
}

------

------------------------

Sol.register_game(free_cell)
Sol.register_game(free_cell_0)
Sol.register_game(free_cell_2)
Sol.register_game(double_free_cell)
Sol.register_game(double_free_cell_2)
Sol.register_game(triple_free_cell)
Sol.register_game(relaxed_free_cell)
Sol.register_game(challenge_free_cell)
Sol.register_game(super_challenge_free_cell)
Sol.register_game(bakers_game)
Sol.register_game(bakers_game_ko)
Sol.register_game(bath)
Sol.register_game(big_cell)
Sol.register_game(cell_11)
Sol.register_game(clink)
Sol.register_game(deep)
Sol.register_game(eight_off)
Sol.register_game(flipper)
Sol.register_game(footling)
Sol.register_game(fore_cell)
Sol.register_game(four_colours)
Sol.register_game(german_free_cell)
Sol.register_game(king_cell)
Sol.register_game(penguin)
