-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local beleaguered_castle       = {
    Info              = {
        Name      = "Beleaguered Castle",
        Family    = "BeleagueredCastle",
        DeckCount = 1
    },
    Foundation        = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_none }
    },
    Tableau           = {
        Size = 8,
        Pile = {
            Initial = Sol.Initial.face_up(6),
            Layout = "Row",
            Rule = Sol.Rules.any_downrank_top
        }
    },
    on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation,
    on_init           = Sol.Layout.beleaguered_castle
}

------

local castle_mount             = Sol.copy(beleaguered_castle)
castle_mount.Info.Name         = "Castle Mount"
castle_mount.Info.DeckCount    = 3
castle_mount.Foundation        = {
    Size = 12,
    Pile = { Rule = Sol.Rules.ace_upsuit_none }
}
castle_mount.Tableau           = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.face_up(12),
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.InSeqInSuit() }
    }
}
castle_mount.on_init           = Sol.Layout.canister

------

local citadel                  = Sol.copy(beleaguered_castle)
citadel.Info.Name              = "Citadel"
citadel.on_shuffle             = Sol.Ops.Shuffle.play_to_foundation

------

local exiled_kings             = Sol.copy(citadel)
exiled_kings.Info.Name         = "Exiled Kings"
exiled_kings.Tableau.Pile      = {
    Initial = Sol.Initial.face_up(6),
    Layout = "Row",
    Rule = Sol.Rules.king_downrank_top
}

------

local castle_of_indolence      = {
    Info       = {
        Name      = "Castle of Indolence",
        Family    = "BeleagueredCastle",
        DeckCount = 2
    },
    Reserve    = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 4 },
                Initial = Sol.Initial.face_up(13),
                Rule = Sol.Rules.none_none_top
            }
        end
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i % 2 + 3, y = i // 2 },
                Rule = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 8,
        Pile = function(i)
            if i < 4 then
                return {
                    Position = { x = 0, y = i },
                    Initial = Sol.Initial.face_up(7),
                    Layout = "Row",
                    Rule = Sol.Rules.any_downrank_top
                }
            else
                return {
                    Position = { x = 5, y = i - 4 },
                    Initial = Sol.Initial.face_up(6),
                    Layout = "Row",
                    Rule = Sol.Rules.any_downrank_top
                }
            end
        end
    }
}

------

local chequers                 = {
    Info        = {
        Name      = "Chequers",
        Family    = "BeleagueredCastle",
        DeckCount = 2
    },
    Reserve     = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(4),
        Rule = Sol.Rules.none_none_none
    },
    Foundation  = {
        Size = 8,
        Pile = function(i)
            if i < 4 then
                return {
                    Position = { x = i + 2, y = 0 },
                    Rule = Sol.Rules.ace_upsuit_top
                }
            else
                return {
                    Position = { x = i + 2, y = 0 },
                    Rule = Sol.Rules.king_downsuit_top
                }
            end
        end
    },
    Tableau     = {
        Size = 25,
        Pile = function(i)
            return {
                Position = { x = i % 5 * 2.5, y = i // 5 + 1 },
                Initial = Sol.Initial.face_up(4),
                Layout = "Row",
                Rule = Sol.Rules.any_updownsuit_top
            }
        end
    },
    on_end_turn = function(game)
        local reserve1 = game.Reserve[1]
        Sol.Ops.Deal.to_group(reserve1, game.Tableau, true)
        if not reserve1.IsEmpty then
            reserve1:flip_down_top_card()
        end
    end
}

------

local fortress                 = {
    Info       = {
        Name      = "Fortress",
        Family    = "BeleagueredCastle",
        DeckCount = 1
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 2 and 6 or 5),
                Layout = "Column",
                Rule = Sol.Rules.any_updownsuit_top
            }
        end
    },
    on_init    = Sol.Layout.canister
}

------

local lightweight              = Sol.copy(beleaguered_castle)
lightweight.Info.Name          = "Lightweight"
lightweight.Info.DeckCount     = 2
lightweight.Foundation         = {
    Size = 8,
    Pile = { Rule = Sol.Rules.ace_upsuit_none }
}
lightweight.Tableau            = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.face_up(8),
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.InSeq() }
    }
}
lightweight.on_init            = Sol.Layout.canister

------

local morehead                 = {
    Info = {
        Name      = "Morehead",
        Family    = "BeleagueredCastle",
        DeckCount = 1
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 7 and i + 1 or 8),
                Layout = "Column",
                Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownAnyButOwnSuit(), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    on_init = Sol.Layout.canister
}

------

local penelopes_web            = {
    Info       = {
        Name      = "Penelope's Web",
        Family    = "BeleagueredCastle",
        DeckCount = 1
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_none }
    },
    Tableau    = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 4 and 7 or 6),
                Layout = "Row",
                Rule = Sol.Rules.king_downrank_top
            }
        end
    },
    on_init    = Sol.Layout.beleaguered_castle
}

------

local rittenhouse              = {
    Info              = {
        Name      = "Rittenhouse",
        Family    = "BeleagueredCastle",
        DeckCount = 2
    },
    Foundation        = {
        Size = 8,
        Pile = function(i)
            if i < 4 then
                return {
                    Position = { x = i, y = 0 },
                    Rule = Sol.Rules.ace_uprank_none
                }
            else
                return {
                    Position = { x = i + 1, y = 0 },
                    Rule = Sol.Rules.king_downrank_none
                }
            end
        end
    },
    Tableau           = {
        Size = 9,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial = Sol.Initial.face_up(i < 6 and 11 or 10),
                Layout = "Column",
                Rule = Sol.Rules.any_updownrank_top
            }
        end
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
    on_shuffle        = function(game, card, pile)
        local idx = game:get_pile_index(pile)
        local foundation = game.Foundation

        if idx ~= 5 then
            if idx > 5 then idx = idx - 1 end
            return game:play_card(foundation[idx], card)
        else
            for _, v in ipairs(foundation) do
                if game:play_card(v, card) then return true end
            end
        end

        return false
    end,
    on_drop           = function(game, pile)
        local foundation = game.Foundation
        local tableau = game.Tableau

        -- check droppile first
        local dropIdx = game:get_pile_index(pile)
        while true do
            local check = false
            if dropIdx ~= 5 then
                check = pile:play_card(foundation[dropIdx > 5 and dropIdx - 1 or dropIdx], game) or check
            else
                for _, fou in ipairs(foundation) do
                    check = tableau[5]:play_card(fou, game) or check
                end
            end
            if not check then break end
        end

        -- check if tableau piles can be moved
        while true do
            local check = false
            for idx, tab in ipairs(tableau) do
                if idx ~= 5 then
                    check = tab:play_card(foundation[idx > 5 and idx - 1 or idx], game) or check
                end
            end
            for _, fou in ipairs(foundation) do
                check = tableau[5]:play_card(fou, game) or check
            end
            if not check then break end
        end
    end,
    can_play          = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == "Foundation" then
            local dstPileIdx = game:get_pile_index(targetPile)
            local srcPileIdx = game:get_pile_index(game:find_pile(card))

            if srcPileIdx > 5 then
                if dstPileIdx ~= srcPileIdx - 1 then return false end
            elseif srcPileIdx < 5 then
                if dstPileIdx ~= srcPileIdx then return false end
            end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
}

------

local selective_castle         = {
    Info       = {
        Name      = "Selective Castle",
        Family    = "BeleagueredCastle",
        DeckCount = 1
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ff_upsuit_none_l13 }
    },
    Tableau    = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 4 and 7 or 6),
                Layout = "Row",
                Rule = Sol.Rules.any_downrank_top
            }
        end
    },
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        local foundation1 = game.Foundation[1]
        if foundation1.IsEmpty and targetPile == foundation1 then -- allow any card on first foundation
            return true
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    on_init    = Sol.Layout.beleaguered_castle
}

------

local streets_and_alleys       = {
    Info       = {
        Name      = "Streets and Alleys",
        Family    = "BeleagueredCastle",
        DeckCount = 1
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_none }
    },
    Tableau    = {
        Size = 8,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 4 and 7 or 6),
                Layout = "Row",
                Rule = Sol.Rules.any_downrank_top
            }
        end
    },
    on_init    = Sol.Layout.beleaguered_castle
}

------

local zerline                  = {
    Info       = {
        Name      = "Zerline",
        Family    = "BeleagueredCastle",
        DeckCount = 2
    },
    Stock      = {
        Position = { x = 3, y = 0 },
        Initial = Sol.Initial.face_down(64)
    },
    Waste      = {
        Position = { x = 4, y = 0 }
    },
    FreeCell   = {
        Position = { x = 5, y = 0 },
        Layout = "Row",
        Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.Any(), Move = Sol.Rules.Move.Top(), Limit = 4 }
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i % 2 + 3.5, y = i // 2 + 1 },
                Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    Tableau    = {
        Size = 8,
        Pile = function(i)
            local rule = { Base = Sol.Rules.Base.Ranks({ "Queen" }), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.Top() }
            if i < 4 then
                return {
                    Position = { x = 0, y = i + 1 },
                    Initial = Sol.Initial.face_up(5),
                    Layout = "Row",
                    Rule = rule
                }
            else
                return {
                    Position = { x = 6, y = i - 3 },
                    Initial = Sol.Initial.face_up(5),
                    Layout = "Row",
                    Rule = rule
                }
            end
        end
    },
    deal       = Sol.Ops.Deal.stock_to_waste,
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == "FreeCell" then
            local srcPile = game:find_pile(card)
            if srcPile.Type == "Tableau" then
                return game:can_play(targetPile, targetCardIndex, card, numCards)
            end
            return false
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end
}

------

local zerline_3_decks          = Sol.copy(zerline)
zerline_3_decks.Info.Name      = "Zerline (3 Decks)"
zerline_3_decks.Info.DeckCount = 3
zerline_3_decks.Stock.Initial  = Sol.Initial.face_down(116)
zerline_3_decks.FreeCell       = {
    Position = { x = 5, y = 0 },
    Layout = "Row",
    Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.Any(), Move = Sol.Rules.Move.Top(), Limit = 6 }
}
zerline_3_decks.Foundation     = {
    Size = 12,
    Pile = function(i)
        return {
            Position = { x = i % 3 + 3, y = i // 3 + 1 },
            Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.Top() }
        }
    end
}
zerline_3_decks.Tableau        = {
    Size = 8,
    Pile = function(i)
        local rule = { Base = Sol.Rules.Base.Ranks({ "Queen" }), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.Top() }
        if i < 4 then
            return {
                Position = { x = 0, y = i + 1 },
                Initial = Sol.Initial.face_up(5),
                Layout = "Row",
                Rule = rule
            }
        else
            return {
                Position = { x = 7, y = i - 3 },
                Initial = Sol.Initial.face_up(5),
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
Sol.register_game(citadel)
Sol.register_game(exiled_kings)
Sol.register_game(fortress)
Sol.register_game(lightweight)
Sol.register_game(morehead)
Sol.register_game(penelopes_web)
Sol.register_game(rittenhouse)
Sol.register_game(selective_castle)
Sol.register_game(streets_and_alleys)
Sol.register_game(zerline)
Sol.register_game(zerline_3_decks)
