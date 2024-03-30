-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local beleaguered_castle       = {
    Info              = {
        Name          = "Beleaguered Castle",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation        = {
        Size = 4,
        Pile = { Rule = rules.ace_upsuit_none }
    },
    Tableau           = {
        Size = 8,
        Pile = {
            Initial = ops.Initial.face_up(6),
            Layout = "Row",
            Rule = rules.any_downrank_top
        }
    },
    on_before_shuffle = ops.Shuffle.ace_to_foundation,
    on_piles_created  = Sol.Layout.beleaguered_castle
}

------

local castle_mount             = Sol.copy(beleaguered_castle)
castle_mount.Info.Name         = "Castle Mount"
castle_mount.Info.DeckCount    = 3
castle_mount.Foundation        = {
    Size = 12,
    Pile = { Rule = rules.ace_upsuit_none }
}
castle_mount.Tableau           = {
    Size = 12,
    Pile = {
        Initial = ops.Initial.face_up(12),
        Layout = "Column",
        Rule = { Base = rules.Base.King, Build = rules.Build.DownByRank(), Move = rules.Move.InSeqInSuit() }
    }
}
castle_mount.on_piles_created  = Sol.Layout.canister

------

local citadel                  = Sol.copy(beleaguered_castle)
citadel.Info.Name              = "Citadel"
citadel.on_shuffle             = function(game, card)
    local foundation = game.Foundation
    for _, v in ipairs(foundation) do
        if v:play_card(game, card) then return true end
    end

    return false
end

------

local exiled_kings             = Sol.copy(citadel)
exiled_kings.Info.Name         = "Exiled Kings"
exiled_kings.Tableau.Pile      = {
    Initial = ops.Initial.face_up(6),
    Layout = "Row",
    Rule = rules.king_downrank_top
}

------

local lightweight              = Sol.copy(beleaguered_castle)
lightweight.Info.Name          = "Lightweight"
lightweight.Info.DeckCount     = 2
lightweight.Foundation         = {
    Size = 8,
    Pile = { Rule = rules.ace_upsuit_none }
}
lightweight.Tableau            = {
    Size = 12,
    Pile = {
        Initial = ops.Initial.face_up(8),
        Layout = "Column",
        Rule = { Base = rules.Base.King, Build = rules.Build.DownByRank(), Move = rules.Move.InSeq() }
    }
}
lightweight.on_piles_created   = Sol.Layout.canister

------

local chessboard               = {
    Info             = {
        Name          = "Chessboard",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation       = {
        Size = 4,
        Pile = { Rule = rules.ff_upsuit_none_l13 }
    },
    Tableau          = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = ops.Initial.face_up(i < 2 and 6 or 5),
                Layout = "Column",
                Rule = { Base = rules.Base.Any, Build = rules.Build.UpOrDownInSuit(true), Move = rules.Move.Top() }
            }
        end
    },
    check_playable   = function(game, targetPile, targetIndex, drop, numCards)
        local foundation1 = game.Foundation[1]
        if foundation1.IsEmpty and targetPile == foundation1 then -- allow any card on first foundation
            return true
        end

        return game:can_play(targetPile, targetIndex, drop, numCards)
    end,
    on_piles_created = Sol.Layout.canister
}

------

local lasker                   = Sol.copy(chessboard)
lasker.Info.Name               = "Lasker"
lasker.Foundation.Pile         = { Rule = rules.ff_upsuit_top_l13 }
lasker.Tableau.Pile            = function(i)
    return {
        Initial = ops.Initial.face_up(i < 2 and 6 or 5),
        Layout = "Column",
        Rule = { Base = rules.Base.Any, Build = rules.Build.UpOrDownInSuit(true), Move = rules.Move.InSeq() }
    }
end

------

local fortress                 = {
    Info             = {
        Name          = "Fortress",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation       = {
        Size = 4,
        Pile = { Rule = rules.ace_upsuit_top }
    },
    Tableau          = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = ops.Initial.face_up(i < 2 and 6 or 5),
                Layout = "Column",
                Rule = rules.any_updownsuit_top
            }
        end
    },
    on_piles_created = Sol.Layout.canister
}

------

local morehead                 = {
    Info = {
        Name          = "Morehead",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = rules.ace_upsuit_top }
    },
    Tableau = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = ops.Initial.face_up(i < 7 and i + 1 or 8),
                Layout = "Column",
                Rule = { Base = rules.Base.Any, Build = rules.Build.DownAnyButOwnSuit(), Move = rules.Move.Top() }
            }
        end
    },
    on_piles_created = Sol.Layout.canister
}

------

local penelopes_web            = {
    Info             = {
        Name          = "Penelope's Web",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation       = {
        Size = 4,
        Pile = { Rule = rules.ace_upsuit_none }
    },
    Tableau          = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = ops.Initial.face_up(i < 4 and 7 or 6),
                Layout = "Row",
                Rule = rules.king_downrank_top
            }
        end
    },
    on_piles_created = Sol.Layout.beleaguered_castle
}

------

local selective_castle         = {
    Info             = {
        Name          = "Selective Castle",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation       = {
        Size = 4,
        Pile = { Rule = rules.ff_upsuit_none_l13 }
    },
    Tableau          = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = ops.Initial.face_up(i < 4 and 7 or 6),
                Layout = "Row",
                Rule = rules.any_downrank_top
            }
        end
    },
    check_playable   = function(game, targetPile, targetIndex, drop, numCards)
        local foundation1 = game.Foundation[1]
        if foundation1.IsEmpty and targetPile == foundation1 then -- allow any card on first foundation
            return true
        end

        return game:can_play(targetPile, targetIndex, drop, numCards)
    end,
    on_piles_created = Sol.Layout.beleaguered_castle
}

------

local streets_and_alleys       = {
    Info             = {
        Name          = "Streets and Alleys",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation       = {
        Size = 4,
        Pile = { Rule = rules.ace_upsuit_none }
    },
    Tableau          = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = ops.Initial.face_up(i < 4 and 7 or 6),
                Layout = "Row",
                Rule = rules.any_downrank_top
            }
        end
    },
    on_piles_created = Sol.Layout.beleaguered_castle
}

------

local chequers                 = {
    Info        = {
        Name          = "Chequers",
        Family        = "BeleagueredCastle",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 0
    },
    Reserve     = {
        Position = { x = 0, y = 0 },
        Initial = ops.Initial.face_down(4),
        Rule = rules.none_none_none
    },
    Foundation  = {
        Size = 8,
        Pile = function(i)
            if i < 4 then
                return {
                    Position = { x = i + 2, y = 0 },
                    Rule = rules.ace_upsuit_top
                }
            else
                return {
                    Position = { x = i + 2, y = 0 },
                    Rule = rules.king_downsuit_top
                }
            end
        end
    },
    Tableau     = {
        Size = 25,
        Pile = function(i)
            return {
                Position = { x = i % 5 * 2.5, y = i // 5 + 1 },
                Initial = ops.Initial.face_up(4),
                Layout = "Row",
                Rule = rules.any_updownsuit_top
            }
        end
    },
    on_end_turn = function(game)
        local reserve1 = game.Reserve[1]
        ops.Deal.to_group(reserve1, game.Tableau, true)
        if not reserve1.IsEmpty then
            reserve1:flip_down_top_card()
        end
    end
}

------

local castle_of_indolence      = {
    Info       = {
        Name          = "Castle of Indolence",
        Family        = "BeleagueredCastle",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 0
    },
    Reserve    = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 4 },
                Initial = ops.Initial.face_up(13),
                Rule = rules.none_none_none
            }
        end
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i % 2 + 3, y = i // 2 },
                Rule = rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 8,
        Pile = function(i)
            if i < 4 then
                return {
                    Position = { x = 0, y = i },
                    Initial = ops.Initial.face_up(7),
                    Layout = "Row",
                    Rule = rules.any_downrank_top
                }
            else
                return {
                    Position = { x = 5, y = i - 4 },
                    Initial = ops.Initial.face_up(6),
                    Layout = "Row",
                    Rule = rules.any_downrank_top
                }
            end
        end
    }
}

------

local zerline                  = {
    Info           = {
        Name          = "Zerline",
        Family        = "BeleagueredCastle",
        DeckCount     = 2,
        CardDealCount = 1,
        Redeals       = 0
    },
    Stock          = {
        Position = { x = 3, y = 0 },
        Initial = ops.Initial.face_down(64)
    },
    Waste          = {
        Position = { x = 4, y = 0 }
    },
    FreeCell       = {
        Position = { x = 5, y = 0 },
        Layout = "Row",
        Rule = { Base = rules.Base.Any, Build = rules.Build.Any(), Move = rules.Move.Top(), Limit = 4 }
    },
    Foundation     = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i % 2 + 3.5, y = i // 2 + 1 },
                Rule = { Base = rules.Base.King, Build = rules.Build.UpInSuit(true), Move = rules.Move.Top() }
            }
        end
    },
    Tableau        = {
        Size = 8,
        Pile = function(i)
            local rule = { Base = function(_, card, _) return rules.Base.Ranks(card, { "Queen" }) end, Build = rules.Build.DownByRank(), Move = rules.Move.Top() }
            if i < 4 then
                return {
                    Position = { x = 0, y = i + 1 },
                    Initial = ops.Initial.face_up(5),
                    Layout = "Row",
                    Rule = rule
                }
            else
                return {
                    Position = { x = 6, y = i - 3 },
                    Initial = ops.Initial.face_up(5),
                    Layout = "Row",
                    Rule = rule
                }
            end
        end
    },
    on_deal        = ops.Deal.stock_to_waste,
    check_playable = function(game, targetPile, targetIndex, drop, numCards)
        if targetPile.Type == "FreeCell" then
            local srcPile = game:find_pile(drop)
            if srcPile.Type == "Tableau" then
                return game:can_play(targetPile, targetIndex, drop, numCards)
            end
            return false
        end

        return game:can_play(targetPile, targetIndex, drop, numCards)
    end
}

------

local zerline_3_decks          = Sol.copy(zerline)
zerline_3_decks.Info.Name      = "Zerline (3 Decks)"
zerline_3_decks.Info.DeckCount = 3
zerline_3_decks.Stock.Initial  = ops.Initial.face_down(116)
zerline_3_decks.FreeCell       = {
    Position = { x = 5, y = 0 },
    Layout = "Row",
    Rule = { Base = rules.Base.Any, Build = rules.Build.Any(), Move = rules.Move.Top(), Limit = 6 }
}
zerline_3_decks.Foundation     = {
    Size = 12,
    Pile = function(i)
        return {
            Position = { x = i % 3 + 3, y = i // 3 + 1 },
            Rule = { Base = rules.Base.King, Build = rules.Build.UpInSuit(true), Move = rules.Move.Top() }
        }
    end
}
zerline_3_decks.Tableau        = {
    Size = 8,
    Pile = function(i)
        local rule = { Base = function(_, card, _) return rules.Base.Ranks(card, { "Queen" }) end, Build = rules.Build.DownByRank(), Move = rules.Move.Top() }
        if i < 4 then
            return {
                Position = { x = 0, y = i + 1 },
                Initial = ops.Initial.face_up(5),
                Layout = "Row",
                Rule = rule
            }
        else
            return {
                Position = { x = 7, y = i - 3 },
                Initial = ops.Initial.face_up(5),
                Layout = "Row",
                Rule = rule
            }
        end
    end
}

------

------------

Sol.register_game(beleaguered_castle)
Sol.register_game(castle_mount)
Sol.register_game(castle_of_indolence)
Sol.register_game(chequers)
Sol.register_game(chessboard)
Sol.register_game(citadel)
Sol.register_game(exiled_kings)
Sol.register_game(fortress)
Sol.register_game(lasker)
Sol.register_game(lightweight)
Sol.register_game(morehead)
Sol.register_game(penelopes_web)
Sol.register_game(selective_castle)
Sol.register_game(streets_and_alleys)
Sol.register_game(zerline)
Sol.register_game(zerline_3_decks)
