-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local free_cell                             = {
    Info         = {
        Name      = "FreeCell",
        Family    = "FreeCell",
        DeckCount = 1
    },
    FreeCell     = {
        Size = 4,
        Pile = { Rule = Sol.Rules.any_none_top }
    },
    Foundation   = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau      = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 4 and 7 or 6),
                Layout = "Column",
                Rule = Sol.Rules.any_downac_sm
            }
        end
    },
    get_shuffled = function(game, seed)
        if game.Name ~= "FreeCell" then return {} end
        if seed[1] > 1000000 or (seed[2] + seed[3] + seed[4]) ~= 0 then return {} end

        --MS FreeCell shuffle:
        local suit <const> = { "Clubs", "Diamonds", "Hearts", "Spades" }
        local deck = {}
        for _, r in ipairs(Sol.Ranks) do
            for _, s in ipairs(suit) do
                deck[#deck + 1] = { Rank = r, Suit = s, Deck = 0, IsFaceDown = false }
            end
        end

        seed = seed[1]
        local function rng()
            seed = (214013 * seed + 2531011) % (1 << 31)
            return seed >> 16
        end

        local seq <const> = {
            1, 8, 15, 22, 29, 35, 41, 47,
            2, 9, 16, 23, 30, 36, 42, 48,
            3, 10, 17, 24, 31, 37, 43, 49,
            4, 11, 18, 25, 32, 38, 44, 50,
            5, 12, 19, 26, 33, 39, 45, 51,
            6, 13, 20, 27, 34, 40, 46, 52,
            7, 14, 21, 28 }

        local ret = {}
        for i = 1, 52 do
            local idx = rng() % #deck + 1
            deck[idx], deck[#deck] = deck[#deck], deck[idx]
            ret[53 - seq[i]] = table.remove(deck)
        end
        return ret
    end,
    on_init      = Sol.Layout.free_cell
}

------

local zero_cell                             = Sol.copy(free_cell)
zero_cell.Info.Name                         = "ZeroCell"
zero_cell.FreeCell                          = nil

------

local two_cell                              = Sol.copy(free_cell)
two_cell.Info.Name                          = "TwoCell"
two_cell.FreeCell.Size                      = 2

------

local three_cell                            = Sol.copy(free_cell)
three_cell.Info.Name                        = "ThreeCell"
three_cell.FreeCell.Size                    = 3

------

local double_free_cell                      = {
    Info       = {
        Name      = "Double FreeCell",
        Family    = "FreeCell",
        DeckCount = 2
    },
    FreeCell   = {
        Size = 8,
        Pile = { Rule = Sol.Rules.any_none_top }
    },
    Foundation = {
        Size = 8,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 4 and 11 or 10),
                Layout = "Column",
                Rule = Sol.Rules.any_downac_sm
            }
        end
    },
    on_init    = Sol.Layout.double_free_cell
}

------

local double_free_cell_2                    = Sol.copy(free_cell)
double_free_cell_2.Info.Name                = "Double FreeCell II"
double_free_cell_2.Info.DeckCount           = 2
double_free_cell_2.FreeCell.Size            = 6
double_free_cell_2.Foundation.Pile          = { Rule = { Base = Sol.Rules.Base.Ace(), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.Top(), Limit = 13 } }
double_free_cell_2.Tableau                  = {
    Size = 10,
    Pile = {
        Initial = Sol.Initial.face_up(10),
        Layout = "Column",
        Rule = Sol.Rules.any_downac_sm
    }
}
double_free_cell_2.on_before_shuffle        = Sol.Ops.Shuffle.ace_to_foundation

------

local triple_free_cell                      = {
    Info       = {
        Name      = "Triple FreeCell",
        Family    = "FreeCell",
        DeckCount = 3
    },
    FreeCell   = {
        Size = 10,
        Pile = { Rule = Sol.Rules.any_none_top }
    },
    Foundation = {
        Size = 12,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 13,
        Pile = {
            Initial = Sol.Initial.face_up(12),
            Layout = "Column",
            Rule = Sol.Rules.any_downac_sm
        }
    },
    on_init    = Sol.Layout.double_free_cell
}

------

local bakers_game                           = Sol.copy(free_cell)
bakers_game.Info.Name                       = "Baker's Game"
bakers_game.Tableau.Pile                    = function(i)
    return {
        Initial = Sol.Initial.face_up(i < 4 and 7 or 6),
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownInSuit(), Move = Sol.Rules.Move.SuperMove() }
    }
end

------

local bakers_game_ko                        = Sol.copy(free_cell)
bakers_game_ko.Info.Name                    = "King Only Baker's Game"
bakers_game_ko.Tableau.Pile                 = function(i)
    return {
        Initial = Sol.Initial.face_up(i < 4 and 7 or 6),
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownInSuit(), Move = Sol.Rules.Move.SuperMove() }
    }
end

------

local bath                                  = Sol.copy(free_cell)
bath.Info.Name                              = "Bath"
bath.FreeCell.Size                          = 2
bath.Tableau                                = {
    Size = 10,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(math.min(i + 1, 8)),
            Layout = "Column",
            Rule = Sol.Rules.any_downac_sm
        }
    end
}

------

local big_cell                              = {
    Info       = {
        Name      = "Big Cell",
        Family    = "FreeCell",
        DeckCount = 3
    },
    FreeCell   = {
        Size = 4,
        Pile = { Rule = Sol.Rules.any_none_top }
    },
    Foundation = {
        Size = 12,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 13,
        Pile = {
            Initial = Sol.Initial.face_up(12),
            Layout = "Column",
            Rule = Sol.Rules.any_downac_sm
        }
    },
    on_init    = Sol.Layout.double_free_cell
}

------

local cell_11                               = {
    Info       = {
        Name      = "Cell 11",
        Family    = "FreeCell",
        DeckCount = 3
    },
    FreeCell   = {
        Size = 11,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up((i == 0 or i == 10) and 1 or 0),
                Rule = Sol.Rules.any_none_top
            }
        end
    },
    Foundation = {
        Size = 12,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 13,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up((i == 0 or i == 12) and 11 or 12),
                Layout = "Column",
                Rule = Sol.Rules.any_downac_sm
            }
        end
    },
    on_init    = Sol.Layout.double_free_cell
}

------

local challenge_free_cell                   = Sol.copy(free_cell)
challenge_free_cell.Info.Name               = "Challenge FreeCell"
challenge_free_cell.Tableau                 = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 4 and 6 or 5),
            Layout = "Column",
            Rule = Sol.Rules.any_downac_sm
        }
    end
}
challenge_free_cell.on_before_shuffle       = function(game, card)
    if card.Rank == "Two" then
        return game.PlaceTop(card, game.Tableau, 1, 4, true)
    end
    if card.Rank == "Ace" then
        return game.PlaceTop(card, game.Tableau, 5, 4, true)
    end

    return false
end

------

local super_challenge_free_cell             = Sol.copy(challenge_free_cell)
super_challenge_free_cell.Info.Name         = "Super Challenge FreeCell"
super_challenge_free_cell.Tableau           = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 4 and 6 or 5),
            Layout = "Column",
            Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownAlternateColors(), Move = Sol.Rules.Move.SuperMove() }
        }
    end
}

------

local clink                                 = Sol.copy(free_cell)
clink.Info.Name                             = "Clink"
clink.FreeCell                              = {
    Size = 2,
    Pile = {
        Initial = Sol.Initial.face_up(1),
        Rule = Sol.Rules.any_none_top
    }
}
clink.Foundation                            = {
    Size = 2,
    Pile = { Rule = { Base = Sol.Rules.Base.Ace(), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.Top() } }
}
clink.Tableau                               = {
    Size = 8,
    Pile = {
        Initial = Sol.Initial.face_up(6),
        Layout  = "Column",
        Rule    = Sol.Rules.any_downac_inseq
    }
}
clink.on_before_shuffle                     = function(game, card)
    if card.Rank == "Ace" and (card.Suit == "Clubs" or card.Suit == "Hearts") then
        return game.PlaceTop(card, game.Foundation, true)
    end

    return false
end

------

local deep                                  = {
    Info       = {
        Name      = "Deep",
        Family    = "FreeCell",
        DeckCount = 2
    },
    FreeCell   = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i, y = 0 },
                Rule = Sol.Rules.any_none_top
            }
        end
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i % 2 + 8, y = i // 2 },
                Rule = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial = Sol.Initial.face_up(13),
                Layout = "Column",
                Rule = Sol.Rules.any_downac_inseq
            }
        end
    }
}

------

local eight_off                             = {
    Info       = {
        Name      = "Eight Off",
        Family    = "FreeCell",
        DeckCount = 1
    },
    FreeCell   = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = i % 2 == 0 and Sol.Initial.face_up(1) or {},
                Rule    = Sol.Rules.any_none_top
            }
        end
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 8,
        Pile = {
            Initial = Sol.Initial.face_up(6),
            Layout = "Column",
            Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownInSuit(), Move = Sol.Rules.Move.SuperMove() }
        }
    },
    on_init    = Sol.Layout.double_free_cell
}

------

local footling                              = Sol.copy(free_cell)
footling.Info.Name                          = "Footling"
footling.Foundation                         = { Rule = { Base = Sol.Rules.Base.Ace(), Build = Sol.Rules.Build.RankPack(), Move = Sol.Rules.Move.Top() } }
footling.Tableau                            = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 4 and 7 or 6),
            Layout = "Column",
            Rule = Sol.Rules.any_downac_inseq
        }
    end
}

------

local double_footling                       = Sol.copy(footling)
double_footling.Info.Name                   = "Double Footling"
double_footling.Info.DeckCount              = 2
double_footling.FreeCell.Size               = 5
double_footling.Tableau                     = {
    Size = 10,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 4 and 11 or 10),
            Layout = "Column",
            Rule = Sol.Rules.any_downac_inseq
        }
    end
}

------

local fore_cell                             = Sol.copy(free_cell)
fore_cell.Info.Name                         = "ForeCell"
fore_cell.FreeCell.Pile                     = {
    Initial = Sol.Initial.face_up(1),
    Rule = Sol.Rules.any_none_top
}
fore_cell.Tableau                           = {
    Size = 8,
    Pile = {
        Initial = Sol.Initial.face_up(6),
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownAlternateColors(), Move = Sol.Rules.Move.SuperMove() }
    }
}

------

local four_colours                          = Sol.copy(free_cell)
four_colours.Info.Name                      = "Four Colours"
four_colours.Tableau                        = {
    Size = 7,
    Pile = function(i)
        return {
            Initial = i < 4 and Sol.Initial.face_up(13) or {},
            Layout = "Column",
            Rule = Sol.Rules.any_downac_inseq
        }
    end
}

------
local german_free_cell_empty                = { "Clubs", "Spades", "Hearts", "Diamonds" }

local german_free_cell                      = Sol.copy(free_cell)
german_free_cell.Info.Name                  = "German FreeCell"
german_free_cell.FreeCell                   = {
    Size = 4,
    Pile = function(i)
        return {
            Layout = "Column",
            Rule = { Base = Sol.Rules.Base.Suits({ german_free_cell_empty[i + 1] }), Build = Sol.Rules.Build.None(), Move = Sol.Rules.Move.Top() }
        }
    end
}

------

local king_cell                             = Sol.copy(free_cell)
king_cell.Info.Name                         = "KingCell"
king_cell.Tableau                           = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 4 and 7 or 6),
            Layout = "Column",
            Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownAlternateColors(), Move = Sol.Rules.Move.SuperMove() }
        }
    end
}

------

local petal                                 = Sol.copy(free_cell)
petal.Info.Name                             = "Petal"
petal.FreeCell.Pile.Initial                 = Sol.Initial.face_up(1)
petal.Foundation.Pile.Rule                  = Sol.Rules.ff_upsuit_none_l13
petal.Tableau.Pile                          = function(i)
    return {
        Initial = Sol.Initial.face_up(i < 4 and 6 or 5),
        Layout = "Column",
        Rule = Sol.Rules.any_downac_sm
    }
end
petal.on_before_shuffle                     = function(game, card)
    local foundation = game.Foundation
    if foundation[1].IsEmpty then
        return game.PlaceTop(card, foundation[1], true)
    else
        local rank = foundation[1].Cards[1].Rank
        if card.Rank == rank then
            return game.PlaceTop(card, foundation, true)
        end
    end

    return false
end

------

local relaxed_free_cell                     = Sol.copy(free_cell)
relaxed_free_cell.Info.Name                 = "Relaxed FreeCell"
relaxed_free_cell.Tableau                   = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 4 and 7 or 6),
            Layout = "Column",
            Rule = Sol.Rules.any_downac_inseq
        }
    end
}

------

local repair                                = Sol.copy(free_cell)
repair.Info.Name                            = "Repair"
repair.Info.DeckCount                       = 2
repair.FreeCell.Pile.Initial                = Sol.Initial.face_up(1)
repair.Foundation.Size                      = 8
repair.Tableau                              = {
    Size = 10,
    Pile = {
        Initial = Sol.Initial.face_up(10),
        Layout = "Column",
        Rule = Sol.Rules.any_downac_inseq
    }
}

------

local seven_x_five                          = Sol.copy(free_cell)
seven_x_five.Info.Name                      = "Seven by Five"
seven_x_five.FreeCell.Size                  = 5
seven_x_five.Tableau                        = {
    Size = 7,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 3 and 8 or 7),
            Layout = "Column",
            Rule = Sol.Rules.any_downac_sm
        }
    end
}

------

local seven_x_four                          = Sol.copy(free_cell)
seven_x_four.Info.Name                      = "Seven by Four"
seven_x_four.Tableau                        = {
    Size = 7,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 3 and 8 or 7),
            Layout = "Column",
            Rule = Sol.Rules.any_downac_sm
        }
    end
}

------

local seahaven_towers                       = Sol.copy(free_cell)
seahaven_towers.Info.Name                   = "Seahaven Towers"
seahaven_towers.FreeCell                    = {
    Size = 4,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up((i == 1 or i == 2) and 1 or 0),
            Rule = Sol.Rules.any_none_top
        }
    end
}
seahaven_towers.Tableau                     = {
    Size = 10,
    Pile = {
        Initial = Sol.Initial.face_up(5),
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownInSuit(), Move = Sol.Rules.Move.SuperMove() }
    }
}

------

local double_seahaven_towers                = Sol.copy(seahaven_towers)
double_seahaven_towers.Info.Name            = "Double Seahaven Towers"
double_seahaven_towers.Info.DeckCount       = 2
double_seahaven_towers.FreeCell             = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up((i > 0 and i < 7) and 1 or 0),
            Rule = Sol.Rules.any_none_top
        }
    end
}
double_seahaven_towers.Foundation.Size      = 8
double_seahaven_towers.Tableau.Size         = 14
double_seahaven_towers.Tableau.Pile.Initial = Sol.Initial.face_up(7)

------

local relaxed_seahaven_towers               = Sol.copy(seahaven_towers)
relaxed_seahaven_towers.Info.Name           = "Relaxed Seahaven Towers"
relaxed_seahaven_towers.Tableau.Pile.Rule   = Sol.Rules.king_downsuit_inseq

------

local flipper                               = {
    Info        = {
        Name      = "Flipper",
        Family    = "FreeCell",
        DeckCount = 1
    },
    FreeCell    = {
        Size = 7,
        Pile = { Rule = Sol.Rules.any_none_top }
    },
    Foundation  = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau     = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i % 3 == 0 and 8 or 7),
                Layout = "Column",
                Rule = Sol.Rules.any_downac_inseq
            }
        end
    },
    on_end_turn = function(game)
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
    on_init     = Sol.Layout.flipper
}

------

local penguin                               = {
    Info              = {
        Name      = "Penguin",
        Family    = "FreeCell",
        DeckCount = 1
    },
    FreeCell          = {
        Size = 7,
        Pile = { Rule = Sol.Rules.any_none_top }
    },
    Foundation        = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ff_upsuit_none_l13 }
    },
    Tableau           = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i > 0 and 7 or 6),
                Layout = "Column",
                Rule = { Base = Sol.Rules.Base.FirstFoundation(-1), Build = Sol.Rules.Build.DownInSuit(true), Move = Sol.Rules.Move.InSeq() }
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
    on_init           = Sol.Layout.flipper
}

------

------------------------

Sol.register_game(free_cell)
Sol.register_game(bakers_game)
Sol.register_game(bakers_game_ko)
Sol.register_game(bath)
Sol.register_game(big_cell)
Sol.register_game(cell_11)
Sol.register_game(challenge_free_cell)
Sol.register_game(clink)
Sol.register_game(deep)
Sol.register_game(double_footling)
Sol.register_game(double_free_cell)
Sol.register_game(double_free_cell_2)
Sol.register_game(double_seahaven_towers)
Sol.register_game(eight_off)
Sol.register_game(flipper)
Sol.register_game(footling)
Sol.register_game(fore_cell)
Sol.register_game(four_colours)
Sol.register_game(zero_cell)
Sol.register_game(two_cell)
Sol.register_game(german_free_cell)
Sol.register_game(king_cell)
Sol.register_game(penguin)
Sol.register_game(petal)
Sol.register_game(relaxed_free_cell)
Sol.register_game(relaxed_seahaven_towers)
Sol.register_game(repair)
Sol.register_game(seahaven_towers)
Sol.register_game(seven_x_five)
Sol.register_game(seven_x_four)
Sol.register_game(super_challenge_free_cell)
Sol.register_game(three_cell)
Sol.register_game(triple_free_cell)
