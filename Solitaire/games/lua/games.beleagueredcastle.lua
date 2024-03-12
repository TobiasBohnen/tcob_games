-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local piles = require 'base/piles'
local rules = require 'base/rules'


local beleaguered_castle    = {
    Info              = {
        Name          = "Beleaguered Castle",
        Type          = "OpenPacker",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation        = {
        Size   = 4,
        create = { Rule = { Build = rules.Build.UpInSuit, Move = rules.Move.None, Empty = rules.Empty.Ace } }
    },
    Tableau           = {
        Size   = 8,
        create = {
            Initial = piles.Initial.face_up(6),
            Layout = "Row",
            Rule = { Build = rules.Build.DownByRank, Move = rules.Move.Top, Empty = rules.Empty.Any }
        }
    },
    on_before_shuffle = ops.Shuffle.ace_to_foundation,
    on_created        = Sol.Layout.beleaguered_castle
}

------

local castle_mount          = Sol.copy(beleaguered_castle)
castle_mount.Info.Name      = "Castle Mount"
castle_mount.Info.DeckCount = 3
castle_mount.Foundation     = {
    Size   = 12,
    create = { Rule = { Build = rules.Build.UpInSuit, Move = rules.Move.None, Empty = rules.Empty.Ace } }
}
castle_mount.Tableau        = {
    Size   = 12,
    create = {
        Initial = piles.Initial.face_up(12),
        Layout = "Column",
        Rule = { Build = rules.Build.DownByRank, Move = rules.Move.InSeqInSuit, Empty = rules.Empty.King }
    }
}
castle_mount.on_created     = Sol.Layout.canister

------

local citadel               = Sol.copy(beleaguered_castle)
citadel.Info.Name           = "Citadel"
citadel.on_shuffle          = function(game, card)
    local foundation = game.Foundation
    for _, v in ipairs(foundation) do
        if game:drop(v, card) then
            return true
        end
    end

    return false
end


------

local exiled_kings               = Sol.copy(citadel)
exiled_kings.Info.Name           = "Exiled Kings"
exiled_kings.Tableau.create      = {
    Initial = piles.Initial.face_up(6),
    Layout = "Row",
    Rule = { Build = rules.Build.DownByRank, Move = rules.Move.Top, Empty = rules.Empty.King }
}

------

local lightweight                = Sol.copy(beleaguered_castle)
lightweight.Info.Name            = "Lightweight"
lightweight.Info.DeckCount       = 2
lightweight.Foundation           = {
    Size   = 8,
    create = { Rule = { Build = rules.Build.UpInSuit, Move = rules.Move.None, Empty = rules.Empty.Ace } }
}
lightweight.Tableau              = {
    Size   = 12,
    create = {
        Initial = piles.Initial.face_up(8),
        Layout = "Column",
        Rule = { Build = rules.Build.DownByRank, Move = rules.Move.InSeq, Empty = rules.Empty.King }
    }
}
lightweight.on_created           = Sol.Layout.canister

------

local fastness                   = {
    Info       = {
        Name          = "Fastness",
        Type          = "OpenPacker",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell   = {
        Size   = 2,
        create = { Rule = { Build = rules.Build.NoBuilding, Move = rules.Move.Top, Empty = rules.Empty.Any } }
    },
    Foundation = {
        Size   = 4,
        create = { Rule = { Build = rules.Build.UpInSuit, Move = rules.Move.None, Empty = rules.Empty.Ace } }
    },
    Tableau    = {
        Size   = 8,
        create = function(i)
            return {
                Initial = piles.Initial.face_up(i < 4 and 7 or 6),
                Layout = "Row",
                Rule = { Build = rules.Build.DownByRank, Move = rules.Move.Top, Empty = rules.Empty.Any }
            }
        end
    },
    on_created = Sol.Layout.fastness
}

------

local siegecraft                 = Sol.copy(fastness)
siegecraft.Info.Name             = "Siegecraft"
siegecraft.FreeCell              = { Rule = { Build = rules.Build.NoBuilding, Move = rules.Move.Top, Empty = rules.Empty.Any } }
siegecraft.Tableau.create        = {
    Initial = piles.Initial.face_up(6),
    Layout = "Row",
    Rule = { Build = rules.Build.DownByRank, Move = rules.Move.Top, Empty = rules.Empty.Any }
}
siegecraft.on_before_shuffle     = ops.Shuffle.ace_to_foundation

------

local stronghold                 = Sol.copy(fastness)
stronghold.Info.Name             = "Stronghold"
stronghold.FreeCell              = { Rule = { Build = rules.Build.NoBuilding, Move = rules.Move.Top, Empty = rules.Empty.Any } }

------

local bastion                    = {
    Info       = {
        Name          = "Bastion",
        Type          = "OpenPacker",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell   = {
        Size   = 2,
        create = {
            Initial = piles.Initial.face_up(1),
            Rule = { Build = rules.Build.NoBuilding, Move = rules.Move.Top, Empty = rules.Empty.Any }
        }
    },
    Foundation = {
        Size   = 4,
        create = piles.ace_upsuit_top
    },
    Tableau    = {
        Size   = 10,
        create = {
            Initial = piles.Initial.face_up(5),
            Layout = "Column",
            Rule = { Build = rules.Build.UpOrDownInSuit, Move = rules.Move.Top, Empty = rules.Empty.Any }
        }
    },
    on_created = Sol.Layout.free_cell
}

------

local castles_end                = Sol.copy(bastion)
castles_end.Info.Name            = "Castles End"
castles_end.Foundation.create    = { Rule = { Build = rules.Build.UpInSuit, Wrap = true, Move = rules.Move.Top, Empty = function(game) return rules.Empty.FirstFoundation(game) end } }
castles_end.Tableau.create       = {
    Initial = piles.Initial.face_up(5),
    Layout = "Column",
    Rule = { Build = rules.Build.UpOrDownAlternateColors, Move = rules.Move.Top, Empty = rules.Empty.Any }
}
castles_end.check_drop           = function(game, targetPile, targetIndex, drop, numCards)
    local foundation1 = game.Foundation[1]
    if foundation1.IsEmpty then -- block card drops if foundation is empty
        return targetPile == foundation1
    end

    return game:can_drop(targetPile, targetIndex, drop, numCards)
end

------

local ten_by_one                 = Sol.copy(bastion)
ten_by_one.Info.Name             = "Ten by One"
ten_by_one.FreeCell              = { Rule = { Build = rules.Build.NoBuilding, Move = rules.Move.Top, Empty = rules.Empty.Any } }
ten_by_one.Tableau.create        = function(i)
    return {
        Initial = piles.Initial.face_up(i < 2 and 6 or 5),
        Layout = "Column",
        Rule = { Build = rules.Build.UpOrDownInSuit, Move = rules.Move.Top, Empty = rules.Empty.Any }
    }
end

------

local canister                   = {
    Info       = {
        Name          = "Canister",
        Type          = "OpenPacker",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size   = 4,
        create = { Rule = { Build = rules.Build.UpInSuit, Move = rules.Move.None, Empty = rules.Empty.Ace } }
    },
    Tableau    = {
        Size   = 8,
        create = function(i)
            return {
                Initial = piles.Initial.face_up((i < 2 or i >= 6) and 6 or 7),
                Layout = "Column",
                Rule = { Build = rules.Build.DownByRank, Move = rules.Move.InSeq, Empty = rules.Empty.Any }
            }
        end
    },
    on_created = Sol.Layout.canister
}

------

local american_canister          = Sol.copy(canister)
american_canister.Info.Name      = "American Canister"
american_canister.Tableau.create = function(i)
    return {
        Initial = piles.Initial.face_up((i < 2 or i >= 6) and 6 or 7),
        Layout = "Column",
        Rule = { Build = rules.Build.DownAlternateColors, Move = rules.Move.InSeq, Empty = rules.Empty.Any }
    }
end

------

local british_canister           = Sol.copy(canister)
british_canister.Info.Name       = "British Canister"
british_canister.Tableau.create  = function(i)
    return {
        Initial = piles.Initial.face_up((i < 2 or i >= 6) and 6 or 7),
        Layout = "Column",
        Rule = { Build = rules.Build.DownAlternateColors, Move = rules.Move.Top, Empty = rules.Empty.King }
    }
end

------

local chessboard                 = {
    Info       = {
        Name          = "Chessboard",
        Type          = "OpenPacker",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size   = 4,
        create = { Rule = { Build = rules.Build.UpInSuit, Wrap = true, Move = rules.Move.None, Empty = function(game) return rules.Empty.FirstFoundation(game) end } }
    },
    Tableau    = {
        Size   = 10,
        create = function(i)
            return {
                Initial = piles.Initial.face_up(i < 2 and 6 or 5),
                Layout = "Column",
                Rule = { Build = rules.Build.UpOrDownInSuit, Wrap = true, Move = rules.Move.Top, Empty = rules.Empty.Any }
            }
        end
    },
    check_drop = function(game, targetPile, targetIndex, drop, numCards)
        local foundation1 = game.Foundation[1]
        if foundation1.IsEmpty and targetPile == foundation1 then -- allow any card on first foundation
            return true
        end

        return game:can_drop(targetPile, targetIndex, drop, numCards)
    end,
    on_created = Sol.Layout.canister
}

------

local lasker                     = Sol.copy(chessboard)
lasker.Info.Name                 = "Lasker"
lasker.Foundation.create         = { Rule = { Build = rules.Build.UpInSuit, Wrap = true, Move = rules.Move.Top, Empty = function(game) return rules.Empty.FirstFoundation(game) end } }
lasker.Tableau.create            = function(i)
    return {
        Initial = piles.Initial.face_up(i < 2 and 6 or 5),
        Layout = "Column",
        Rule = { Build = rules.Build.UpOrDownInSuit, Wrap = true, Move = rules.Move.InSeq, Empty = rules.Empty.Any }
    }
end

------

local fortress                   = {
    Info       = {
        Name          = "Fortress",
        Type          = "OpenPacker",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size   = 4,
        create = piles.ace_upsuit_top
    },
    Tableau    = {
        Size   = 10,
        create = function(i)
            return {
                Initial = piles.Initial.face_up(i < 2 and 6 or 5),
                Layout = "Column",
                Rule = { Build = rules.Build.UpOrDownInSuit, Move = rules.Move.Top, Empty = rules.Empty.Any }
            }
        end
    },
    on_created = Sol.Layout.canister
}

------

local morehead                   = {
    Info = {
        Name          = "Morehead",
        Type          = "OpenPacker",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size   = 4,
        create = piles.ace_upsuit_top
    },
    Tableau = {
        Size   = 10,
        create = function(i)
            return {
                Initial = piles.Initial.face_up(i < 7 and i + 1 or 8),
                Layout = "Column",
                Rule = { Build = rules.Build.DownAnyButOwnSuit, Move = rules.Move.Top, Empty = rules.Empty.Any }
            }
        end
    },
    on_created = Sol.Layout.canister
}

------

local penelopes_web              = {
    Info       = {
        Name          = "Penelope's Web",
        Type          = "OpenPacker",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size   = 4,
        create = { Rule = { Build = rules.Build.UpInSuit, Move = rules.Move.None, Empty = rules.Empty.Ace } }
    },
    Tableau    = {
        Size   = 8,
        create = function(i)
            return {
                Initial = piles.Initial.face_up(i < 4 and 7 or 6),
                Layout = "Row",
                Rule = { Build = rules.Build.DownByRank, Move = rules.Move.Top, Empty = rules.Empty.King }
            }
        end
    },
    on_created = Sol.Layout.beleaguered_castle
}

------

local selective_castle           = {
    Info       = {
        Name          = "Selective Castle",
        Type          = "OpenPacker",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size   = 4,
        create = { Rule = { Build = rules.Build.UpInSuit, Wrap = true, Move = rules.Move.None, Empty = function(game) return rules.Empty.FirstFoundation(game) end } }
    },
    Tableau    = {
        Size   = 8,
        create = function(i)
            return {
                Initial = piles.Initial.face_up(i < 4 and 7 or 6),
                Layout = "Row",
                Rule = { Build = rules.Build.DownByRank, Move = rules.Move.Top, Empty = rules.Empty.Any }
            }
        end
    },
    check_drop = function(game, targetPile, targetIndex, drop, numCards)
        local foundation1 = game.Foundation[1]
        if foundation1.IsEmpty and targetPile == foundation1 then -- allow any card on first foundation
            return true
        end

        return game:can_drop(targetPile, targetIndex, drop, numCards)
    end,
    on_created = Sol.Layout.beleaguered_castle
}

------

local streets_and_alleys         = {
    Info       = {
        Name          = "Streets and Alleys",
        Type          = "OpenPacker",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size   = 4,
        create = { Rule = { Build = rules.Build.UpInSuit, Move = rules.Move.None, Empty = rules.Empty.Ace } }
    },
    Tableau    = {
        Size   = 8,
        create = function(i)
            return {
                Initial = piles.Initial.face_up(i < 4 and 7 or 6),
                Layout = "Row",
                Rule = { Build = rules.Build.DownByRank, Move = rules.Move.Top, Empty = rules.Empty.Any }
            }
        end
    },
    on_created = Sol.Layout.beleaguered_castle
}

------

local chequers                   = {
    Info       = {
        Name          = "Chequers",
        Type          = "OpenPacker",
        Family        = "BeleagueredCastle",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 0
    },
    Reserve    = {
        Position = { x = 0, y = 0 },
        Initial = piles.Initial.face_down(4),
        Rule = { Build = rules.Build.NoBuilding, Move = rules.Move.None, Empty = rules.Empty.None }
    },
    Foundation = {
        Size   = 8,
        create = function(i)
            if i < 4 then
                return {
                    Position = { x = i + 2, y = 0 },
                    Rule = { Build = rules.Build.UpInSuit, Move = rules.Move.Top, Empty = rules.Empty.Ace }
                }
            else
                return {
                    Position = { x = i + 2, y = 0 },
                    Rule = { Build = rules.Build.DownInSuit, Move = rules.Move.Top, Empty = rules.Empty.King }
                }
            end
        end
    },
    Tableau    = {
        Size   = 25,
        create = function(i)
            return {
                Position = { x = i % 5 * 2.5, y = i // 5 + 1 },
                Initial = piles.Initial.face_up(4),
                Layout = "Row",
                Rule = { Build = rules.Build.UpOrDownInSuit, Move = rules.Move.Top, Empty = rules.Empty.Any }
            }
        end
    },
    on_change  = function(game)
        local reserve1 = game.Reserve[1]
        reserve1:deal_to_group(game.Tableau, true)
        if not reserve1.IsEmpty then
            reserve1:flip_down_top_card()
        end
    end
}

------

local castle_of_indolence        = {
    Info       = {
        Name          = "Castle of Indolence",
        Type          = "OpenPacker",
        Family        = "BeleagueredCastle",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 0
    },
    Reserve    = {
        Size   = 4,
        create = function(i)
            return {
                Position = { x = i + 2, y = 4 },
                Initial = piles.Initial.face_up(13),
                Rule = { Build = rules.Build.NoBuilding, Move = rules.Move.None, Empty = rules.Empty.None }
            }
        end
    },
    Foundation = {
        Size   = 8,
        create = function(i)
            return {
                Position = { x = i % 2 + 3, y = i // 2 },
                Rule = { Build = rules.Build.UpInSuit, Move = rules.Move.Top, Empty = rules.Empty.Ace }
            }
        end
    },
    Tableau    = {
        Size   = 8,
        create = function(i)
            if i < 4 then
                return {
                    Position = { x = 0, y = i },
                    Initial = piles.Initial.face_up(7),
                    Layout = "Row",
                    Rule = { Build = rules.Build.DownByRank, Move = rules.Move.Top, Empty = rules.Empty.Any }
                }
            else
                return {
                    Position = { x = 5, y = i - 4 },
                    Initial = piles.Initial.face_up(6),
                    Layout = "Row",
                    Rule = { Build = rules.Build.DownByRank, Move = rules.Move.Top, Empty = rules.Empty.Any }
                }
            end
        end
    }
}

------

local zerline                    = {
    Info       = {
        Name          = "Zerline",
        Type          = "OpenPacker",
        Family        = "BeleagueredCastle",
        DeckCount     = 2,
        CardDealCount = 1,
        Redeals       = 0
    },
    Stock      = {
        Position = { x = 3, y = 0 },
        Initial = piles.Initial.face_down(64)
    },
    Waste      = {
        Position = { x = 4, y = 0 }
    },
    FreeCell   = {
        Position = { x = 5, y = 0 },
        Layout = "Row",
        Rule = { Build = rules.Build.Any, Move = rules.Move.Top, Empty = rules.Empty.Any, Limit = 4 }
    },
    Foundation = {
        Size   = 8,
        create = function(i)
            return {
                Position = { x = i % 2 + 3.5, y = i // 2 + 1 },
                Rule = { Build = rules.Build.UpInSuit, Wrap = true, Move = rules.Move.Top, Empty = rules.Empty.King }
            }
        end
    },
    Tableau    = {
        Size   = 8,
        create = function(i)
            if i < 4 then
                return {
                    Position = { x = 0, y = i + 1 },
                    Initial = piles.Initial.face_up(5),
                    Layout = "Row",
                    Rule = { Build = rules.Build.DownByRank, Move = rules.Move.Top, Empty = function() return rules.Empty.Ranks({ "Queen" }) end }
                }
            else
                return {
                    Position = { x = 6, y = i - 3 },
                    Initial = piles.Initial.face_up(5),
                    Layout = "Row",
                    Rule = { Build = rules.Build.DownByRank, Move = rules.Move.Top, Empty = function() return rules.Empty.Ranks({ "Queen" }) end }
                }
            end
        end
    },
    on_deal    = ops.Deal.stock_to_waste,
    check_drop = function(game, targetPile, targetIndex, drop, numCards)
        if targetPile.Type == "FreeCell" then
            local srcPile = game:find_pile(drop)
            if srcPile.Type == "Tableau" then
                return game:can_drop(targetPile, targetIndex, drop, numCards)
            end
            return false
        end

        return game:can_drop(targetPile, targetIndex, drop, numCards)
    end
}

------

local zerline_3_decks            = Sol.copy(zerline)
zerline_3_decks.Info.Name        = "Zerline (3 Decks)"
zerline_3_decks.Info.DeckCount   = 3
zerline_3_decks.Stock.Initial    = piles.Initial.face_down(116)
zerline_3_decks.FreeCell         = {
    Position = { x = 5, y = 0 },
    Layout = "Row",
    Rule = { Build = rules.Build.Any, Move = rules.Move.Top, Empty = rules.Empty.Any, Limit = 6 }
}
zerline_3_decks.Foundation       = {
    Size   = 12,
    create = function(i)
        return {
            Position = { x = i % 3 + 3, y = i // 3 + 1 },
            Rule = { Build = rules.Build.UpInSuit, Wrap = true, Move = rules.Move.Top, Empty = rules.Empty.King }
        }
    end
}
zerline_3_decks.Tableau          = {
    Size   = 8,
    create = function(i)
        if i < 4 then
            return {
                Position = { x = 0, y = i + 1 },
                Initial = piles.Initial.face_up(5),
                Layout = "Row",
                Rule = { Build = rules.Build.DownByRank, Move = rules.Move.Top, Empty = function() return rules.Empty.Ranks({ "Queen" }) end }
            }
        else
            return {
                Position = { x = 7, y = i - 3 },
                Initial = piles.Initial.face_up(5),
                Layout = "Row",
                Rule = { Build = rules.Build.DownByRank, Move = rules.Move.Top, function() return rules.Empty.Ranks({ "Queen" }) end }
            }
        end
    end
}

------------

Sol.register_game(beleaguered_castle)
Sol.register_game(american_canister)
Sol.register_game(bastion)
Sol.register_game(british_canister)
Sol.register_game(canister)
Sol.register_game(castle_mount)
Sol.register_game(castle_of_indolence)
Sol.register_game(castles_end)
Sol.register_game(chequers)
Sol.register_game(chessboard)
Sol.register_game(citadel)
Sol.register_game(exiled_kings)
Sol.register_game(fastness)
Sol.register_game(fortress)
Sol.register_game(lasker)
Sol.register_game(lightweight)
Sol.register_game(morehead)
Sol.register_game(penelopes_web)
Sol.register_game(selective_castle)
Sol.register_game(siegecraft)
Sol.register_game(streets_and_alleys)
Sol.register_game(stronghold)
Sol.register_game(ten_by_one)
Sol.register_game(zerline)
Sol.register_game(zerline_3_decks)
