-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local layout = require 'base/layout'
local ops    = require 'base/ops'
local piles  = require 'base/piles'
require 'base/common'

local beleaguered_castle    = {
    Info           = {
        Name          = "Beleaguered Castle",
        Type          = "OpenPacker",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation     = {
        Size   = 4,
        create = function() return { Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" } } end
    },
    Tableau        = {
        Size   = 8,
        create = function()
            return {
                Initial = piles.initial.face_up(6),
                Layout = "Row",
                Rule = { Build = "DownByRank", Move = "Top", Empty = "Any" }
            }
        end
    },
    before_shuffle = ops.shuffle.ace_to_foundation,
    layout         = layout.beleaguered_castle
}

------

local castle_mount          = Copy(beleaguered_castle)
castle_mount.Info.Name      = "Castle Mount"
castle_mount.Info.DeckCount = 3
castle_mount.Foundation     = {
    Size   = 12,
    create = function() return { Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" } } end
}
castle_mount.Tableau        = {
    Size   = 12,
    create = function()
        return {
            Initial = piles.initial.face_up(12),
            Layout = "Column",
            Rule = { Build = "DownByRank", Move = "InSequenceInSuit", Empty = "King" }
        }
    end
}
castle_mount.layout         = layout.canister

------

local citadel               = Copy(beleaguered_castle)
citadel.Info.Name           = "Citadel"
citadel.shuffle             = function(game, card)
    local foundation = game.Foundation
    for k, v in pairs(foundation) do
        if v:drop(game, card) then
            return true
        end
    end

    return false
end


------

local exiled_kings               = Copy(citadel)
exiled_kings.Info.Name           = "Exiled Kings"
exiled_kings.Tableau.create      = function()
    return {
        Initial = piles.initial.face_up(6),
        Layout = "Row",
        Rule = { Build = "DownByRank", Move = "Top", Empty = "King" }
    }
end

------

local lightweight                = Copy(beleaguered_castle)
lightweight.Info.Name            = "Lightweight"
lightweight.Info.DeckCount       = 2
lightweight.Foundation           = {
    Size   = 8,
    create = function() return { Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" } } end
}
lightweight.Tableau              = {
    Size   = 12,
    create = function()
        return {
            Initial = piles.initial.face_up(8),
            Layout = "Column",
            Rule = { Build = "DownByRank", Move = "InSequence", Empty = "King" }
        }
    end
}
lightweight.layout               = layout.canister

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
        create = function() return { Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" } } end
    },
    Foundation = {
        Size   = 4,
        create = function() return { Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" } } end
    },
    Tableau    = {
        Size   = 8,
        create = function(i)
            return {
                Initial = piles.initial.face_up(i < 4 and 7 or 6),
                Layout = "Row",
                Rule = { Build = "DownByRank", Move = "Top", Empty = "Any" }
            }
        end
    },
    layout     = layout.fastness
}

------

local siegecraft                 = Copy(fastness)
siegecraft.Info.Name             = "Siegecraft"
siegecraft.FreeCell              = { Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" } }
siegecraft.Tableau.create        = function()
    return {
        Initial = piles.initial.face_up(6),
        Layout = "Row",
        Rule = { Build = "DownByRank", Move = "Top", Empty = "Any" }
    }
end
siegecraft.before_shuffle        = ops.shuffle.ace_to_foundation

------

local stronghold                 = Copy(fastness)
stronghold.Info.Name             = "Stronghold"
stronghold.FreeCell              = { Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" } }

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
        create = function()
            return {
                Initial = piles.initial.face_up(1),
                Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" }
            }
        end
    },
    Foundation = {
        Size   = 4,
        create = piles.ace_upsuit_top
    },
    Tableau    = {
        Size   = 10,
        create = function()
            return {
                Initial = piles.initial.face_up(5),
                Layout = "Column",
                Rule = { Build = "UpOrDownInSuit", Move = "Top", Empty = "Any" }
            }
        end
    },
    layout     = layout.free_cell
}

------

local castles_end                = Copy(bastion)
castles_end.Info.Name            = "Castles End"
castles_end.Foundation.create    = function() return { Rule = { Build = "UpInSuit", Wrap = true, Move = "Top", Empty = "FirstFoundation" } } end
castles_end.Tableau.create       = function()
    return {
        Initial = piles.initial.face_up(5),
        Layout = "Column",
        Rule = { Build = "UpOrDownAlternateColors", Move = "Top", Empty = "Any" }
    }
end
castles_end.can_drop             = function(game, targetPile, targetIndex, drop, numCards)
    local foundation1 = game.Foundation[1]
    if foundation1.Empty then -- block card drops if foundation is empty
        return targetPile == foundation1
    end

    return game:can_drop(targetPile, targetIndex, drop, numCards)
end

------

local ten_by_one                 = Copy(bastion)
ten_by_one.Info.Name             = "Ten by One"
ten_by_one.FreeCell              = { Rule = { Build = "NoBuilding", Move = "Top", Empty = "Any" } }
ten_by_one.Tableau.create        = function(i)
    return {
        Initial = piles.initial.face_up(i < 2 and 6 or 5),
        Layout = "Column",
        Rule = { Build = "UpOrDownInSuit", Move = "Top", Empty = "Any" }
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
        create = function() return { Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" } } end
    },
    Tableau    = {
        Size   = 8,
        create = function(i)
            return {
                Initial = piles.initial.face_up((i < 2 or i >= 6) and 6 or 7),
                Layout = "Column",
                Rule = { Build = "DownByRank", Move = "InSequence", Empty = "Any" }
            }
        end
    },
    layout     = layout.canister
}

------

local american_canister          = Copy(canister)
american_canister.Info.Name      = "American Canister"
american_canister.Tableau.create = function(i)
    return {
        Initial = piles.initial.face_up((i < 2 or i >= 6) and 6 or 7),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "InSequence", Empty = "Any" }
    }
end

------

local british_canister           = Copy(canister)
british_canister.Info.Name       = "British Canister"
british_canister.Tableau.create  = function(i)
    return {
        Initial = piles.initial.face_up((i < 2 or i >= 6) and 6 or 7),
        Layout = "Column",
        Rule = { Build = "DownAlternateColors", Move = "Top", Empty = "King" }
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
        create = function() return { Rule = { Build = "UpInSuit", Wrap = true, Move = "None", Empty = "FirstFoundation" } } end
    },
    Tableau    = {
        Size   = 10,
        create = function(i)
            return {
                Initial = piles.initial.face_up(i < 2 and 6 or 5),
                Layout = "Column",
                Rule = { Build = "UpOrDownInSuit", Wrap = true, Move = "Top", Empty = "Any" }
            }
        end
    },
    can_drop   = function(game, targetPile, targetIndex, drop, numCards)
        local foundation1 = game.Foundation[1]
        if foundation1.Empty and targetPile == foundation1 then -- allow any card on first foundation
            return true
        end

        return game:can_drop(targetPile, targetIndex, drop, numCards)
    end,
    layout     = layout.canister
}

------

local lasker                     = Copy(chessboard)
lasker.Info.Name                 = "Lasker"
lasker.Foundation.create         = function() return { Rule = { Build = "UpInSuit", Wrap = true, Move = "Top", Empty = "FirstFoundation" } } end
lasker.Tableau.create            = function(i)
    return {
        Initial = piles.initial.face_up(i < 2 and 6 or 5),
        Layout = "Column",
        Rule = { Build = "UpOrDownInSuit", Wrap = true, Move = "InSequence", Empty = "Any" }
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
                Initial = piles.initial.face_up(i < 2 and 6 or 5),
                Layout = "Column",
                Rule = { Build = "UpOrDownInSuit", Move = "Top", Empty = "Any" }
            }
        end
    },
    layout     = layout.canister
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
                Initial = piles.initial.face_up(i < 7 and i + 1 or 8),
                Layout = "Column",
                Rule = { Build = "DownAnyButOwnSuit", Move = "Top", Empty = "Any" }
            }
        end
    },
    layout = layout.canister
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
        create = function() return { Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" } } end
    },
    Tableau    = {
        Size   = 8,
        create = function(i)
            return {
                Initial = piles.initial.face_up(i < 4 and 7 or 6),
                Layout = "Row",
                Rule = { Build = "DownByRank", Move = "Top", Empty = "King" }
            }
        end
    },
    layout     = layout.beleaguered_castle
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
        create = function() return { Rule = { Build = "UpInSuit", Wrap = true, Move = "None", Empty = "FirstFoundation" } } end
    },
    Tableau    = {
        Size   = 8,
        create = function(i)
            return {
                Initial = piles.initial.face_up(i < 4 and 7 or 6),
                Layout = "Row",
                Rule = { Build = "DownByRank", Move = "Top", Empty = "Any" }
            }
        end
    },
    can_drop   = function(game, targetPile, targetIndex, drop, numCards)
        local foundation1 = game.Foundation[1]
        if foundation1.Empty and targetPile == foundation1 then -- allow any card on first foundation
            return true
        end

        return game:can_drop(targetPile, targetIndex, drop, numCards)
    end,
    layout     = layout.beleaguered_castle
}

------

local somerset                   = {
    Info       = {
        Name          = "Somerset",
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
                Initial = piles.initial.face_up(i < 7 and i + 1 or 8),
                Layout = "Column",
                Rule = { Build = "DownAlternateColors", Move = "Top", Empty = "Any" }
            }
        end
    },
    layout     = layout.canister
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
        create = function() return { Rule = { Build = "UpInSuit", Move = "None", Empty = "Ace" } } end
    },
    Tableau    = {
        Size   = 8,
        create = function(i)
            return {
                Initial = piles.initial.face_up(i < 4 and 7 or 6),
                Layout = "Row",
                Rule = { Build = "DownByRank", Move = "Top", Empty = "Any" }
            }
        end
    },
    layout     = layout.beleaguered_castle
}

------

local chequers                   = {
    Info          = {
        Name          = "Chequers",
        Type          = "OpenPacker",
        Family        = "BeleagueredCastle",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 0
    },
    Reserve       = {
        Position = { x = 0, y = 0 },
        Initial = piles.initial.face_down(4),
        Rule = { Build = "NoBuilding", Move = "None", Empty = "None" }
    },
    Foundation    = {
        Size   = 8,
        create = function(i)
            if i < 4 then
                return {
                    Position = { x = i + 2, y = 0 },
                    Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" }
                }
            else
                return {
                    Position = { x = i + 2, y = 0 },
                    Rule = { Build = "DownInSuit", Move = "Top", Empty = "King" }
                }
            end
        end
    },
    Tableau       = {
        Size   = 25,
        create = function(i)
            return {
                Position = { x = i % 5 * 2.5, y = i // 5 + 1 },
                Initial = piles.initial.face_up(4),
                Layout = "Row",
                Rule = { Build = "UpOrDownInSuit", Move = "Top", Empty = "Any" }
            }
        end
    },
    before_layout = function(game)
        local reserve1 = game.Reserve[1]
        reserve1:fill_group(game.Tableau)
        if not reserve1.Empty then
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
                Initial = piles.initial.face_up(13),
                Rule = { Build = "NoBuilding", Move = "None", Empty = "None" }
            }
        end
    },
    Foundation = {
        Size   = 8,
        create = function(i)
            return {
                Position = { x = i % 2 + 3, y = i // 2 },
                Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" }
            }
        end
    },
    Tableau    = {
        Size   = 8,
        create = function(i)
            if i < 4 then
                return {
                    Position = { x = 0, y = i },
                    Initial = piles.initial.face_up(7),
                    Layout = "Row",
                    Rule = { Build = "DownByRank", Move = "Top", Empty = "Any" }
                }
            else
                return {
                    Position = { x = 5, y = i - 4 },
                    Initial = piles.initial.face_up(6),
                    Layout = "Row",
                    Rule = { Build = "DownByRank", Move = "Top", Empty = "Any" }
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
        Initial = piles.initial.face_down(64)
    },
    Waste      = {
        Position = { x = 4, y = 0 }
    },
    FreeCell   = {
        Position = { x = 5, y = 0 },
        Layout = "Row",
        Rule = { Build = "Any", Move = "Top", Empty = "Any", Limit = 4 }
    },
    Foundation = {
        Size   = 8,
        create = function(i)
            return {
                Position = { x = i % 2 + 3.5, y = i // 2 + 1 },
                Rule = { Build = "UpInSuit", Wrap = true, Move = "Top", Empty = "King" }
            }
        end
    },
    Tableau    = {
        Size   = 8,
        create = function(i)
            if i < 4 then
                return {
                    Position = { x = 0, y = i + 1 },
                    Initial = piles.initial.face_up(5),
                    Layout = "Row",
                    Rule = { Build = "DownByRank", Move = "Top", Empty = { Type = "Ranks", Ranks = { "Queen" } } }
                }
            else
                return {
                    Position = { x = 6, y = i - 3 },
                    Initial = piles.initial.face_up(5),
                    Layout = "Row",
                    Rule = { Build = "DownByRank", Move = "Top", Empty = { Type = "Ranks", Ranks = { "Queen" } } }
                }
            end
        end
    },
    deal       = ops.deal.stock_to_waste,
    can_drop   = function(game, targetPile, targetIndex, drop, numCards)
        if targetPile.Type == "FreeCell" then
            local srcPile = game:find_pile(drop)
            if srcPile.Type == "Tableau" then
                return game:can_drop(targetPile, targetIndex, drop, numCards)
            end
            return false;
        end

        return game:can_drop(targetPile, targetIndex, drop, numCards)
    end
}

------

local zerline_3_decks            = Copy(zerline)
zerline_3_decks.Info.Name        = "Zerline (3 Decks)"
zerline_3_decks.Info.DeckCount   = 3
zerline_3_decks.Stock.Initial    = piles.initial.face_down(116)
zerline_3_decks.FreeCell         = {
    Position = { x = 5, y = 0 },
    Layout = "Row",
    Rule = { Build = "Any", Move = "Top", Empty = "Any", Limit = 6 }
}
zerline_3_decks.Foundation       = {
    Size   = 12,
    create = function(i)
        return {
            Position = { x = i % 3 + 3, y = i // 3 + 1 },
            Rule = { Build = "UpInSuit", Wrap = true, Move = "Top", Empty = "King" }
        }
    end
}
zerline_3_decks.Tableau          = {
    Size   = 8,
    create = function(i)
        if i < 4 then
            return {
                Position = { x = 0, y = i + 1 },
                Initial = piles.initial.face_up(5),
                Layout = "Row",
                Rule = { Build = "DownByRank", Move = "Top", Empty = { Type = "Ranks", Ranks = { "Queen" } } }
            }
        else
            return {
                Position = { x = 7, y = i - 3 },
                Initial = piles.initial.face_up(5),
                Layout = "Row",
                Rule = { Build = "DownByRank", Move = "Top", Empty = { Type = "Ranks", Ranks = { "Queen" } } }
            }
        end
    end
}

------------

register_game(beleaguered_castle)
register_game(american_canister)
register_game(bastion)
register_game(british_canister)
register_game(canister)
register_game(castle_mount)
register_game(castle_of_indolence)
register_game(castles_end)
register_game(chequers)
register_game(chessboard)
register_game(citadel)
register_game(exiled_kings)
register_game(fastness)
register_game(fortress)
register_game(lasker)
register_game(lightweight)
register_game(morehead)
register_game(penelopes_web)
register_game(selective_castle)
register_game(siegecraft)
register_game(somerset)
register_game(streets_and_alleys)
register_game(stronghold)
register_game(ten_by_one)
register_game(zerline)
register_game(zerline_3_decks)
