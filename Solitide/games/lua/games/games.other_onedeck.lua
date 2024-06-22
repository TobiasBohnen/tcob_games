-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local board_patience = {
    Info = {
        Name      = "Board Patience",
        Family    = "Other",
        DeckCount = 1
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Initial  = Sol.Initial.face_up(i == 0 and 1 or 0),
                Rule     = Sol.Rules.ff_upsuit_top
            }
        end
    },
    Tableau = {
        Size = 10,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial  = Sol.Initial.face_up(i == 0 and 6 or 5),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownAlternateColors(true), Move = Sol.Rules.Move.Top() }
            }
        end
    }
}


------

local captive_queens = {
    Info = {
        Name      = "Captive Queens",
        Family    = "Other",
        DeckCount = 1,
        Redeals   = 2
    },
    Stock = {
        Position = { x = 0, y = 0.5 },
        Initial  = Sol.Initial.face_down(52)
    },
    Waste = {
        Position = { x = 0, y = 1.5 }
    },
    Foundation = {
        Size = 12,
        Pile = function(i)
            local rule = {}
            if i < 4 then
                rule = { Base = Sol.Rules.Base.Ranks({ "Five" }), Build = Sol.Rules.Build.DownInSuit(true), Move = Sol.Rules.Move.Top(), Limit = 6 }
            elseif i < 8 then
                rule = { Base = Sol.Rules.Base.Ranks({ "Queen" }), Build = Sol.Rules.Build.None(), Move = Sol.Rules.Move.None() }
            else
                rule = { Base = Sol.Rules.Base.Ranks({ "Six" }), Build = Sol.Rules.Build.UpInSuit(), Move = Sol.Rules.Move.Top(), Limit = 6 }
            end
            return {
                Position = { x = i % 4 + 1, y = i // 4 },
                Rule     = rule
            }
        end
    },
    deal = Sol.Ops.Deal.stock_to_waste,
    redeal = Sol.Ops.Redeal.waste_to_stock
}


------

local circle_eight_pos = {
    --[[ --]] { 1, 0 }, { 2, 0 }, { 3, 0 }, --[[ --]]
    { 0, 1 }, --[[ --]] --[[ --]] --[[ --]] { 4, 1 },
    --[[ --]] { 1, 2 }, { 2, 2 }, { 3, 2 }, --[[ --]]
}

local circle_eight = {
    Info = {
        Name      = "Circle Eight",
        Family    = "Other",
        DeckCount = 1,
        Redeals   = 1,
        Objective = "AllCardsToTableau"
    },
    Stock = {
        Position = { x = 1.5, y = 1 },
        Initial  = Sol.Initial.face_down(44)
    },
    Waste = {
        Position = { x = 2.5, y = 1 }
    },
    Tableau = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = circle_eight_pos[i + 1][1], y = circle_eight_pos[i + 1][2] },
                Initial  = Sol.Initial.face_up(1),
                Rule     = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpByRank(true), Move = Sol.Rules.Move.None() }
            }
        end
    },
    deal = Sol.Ops.Deal.stock_to_waste,
    redeal = Sol.Ops.Redeal.waste_to_stock
}


------

local corner_suit = {
    Info = {
        Name      = "Corner Suite",
        Family    = "Other",
        DeckCount = 1
    },
    Stock = {
        Position = { x = 1.5, y = 0 },
        Initial  = Sol.Initial.face_down(52)
    },
    Waste = {
        Position = { x = 2.5, y = 0 }
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = (i % 2) * 4, y = (i // 2) * 4 },
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau = {
        Size = 9,
        Pile = function(i)
            return {
                Position = { x = i % 3 + 1, y = i // 3 + 1 },
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    deal = Sol.Ops.Deal.stock_to_waste,
    redeal = Sol.Ops.Redeal.waste_to_stock,
    can_play = function(game, targetPile, targetCardIndex, card, numCards)
        -- empty tableau can only be filled from the waste
        if targetPile.Type == Sol.Pile.Type.Tableau and targetPile.IsEmpty then
            if game:find_pile(card).Type ~= Sol.Pile.Type.Waste then
                return false
            end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end
}


------

local diamond_mine = {
    Info = {
        Name      = "Diamond Mine",
        Family    = "Other",
        DeckCount = 1
    },
    Foundation = {
        Position = { x = 6, y = 0 },
        Rule     = { Base = Sol.Rules.Base.Suits({ "Diamonds" }), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.None() }
    },
    Tableau = {
        Size = 13,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial  = Sol.Initial.top_face_up(4),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.any_downrank_inseq
            }
        end
    },
    get_status = function(game)
        -- all diamonds in foundation; sort remaining suits
        local foundation = game.Foundation
        if foundation[1].CardCount == 13 then
            for _, tableau in ipairs(game.Tableau) do
                if tableau.IsEmpty then goto continue end
                if tableau.CardCount ~= 13 then return Sol.GameStatus.Running end

                local cards = tableau.Cards
                local targetSuit = cards[1].Suit
                for i = 1, 13 do
                    if cards[i].Rank ~= Sol.Ranks[14 - i] or cards[i].Suit ~= targetSuit then
                        return Sol.GameStatus.Running
                    end
                end

                ::continue::
            end
            return Sol.GameStatus.Success
        end
        --TODO check dead
        return Sol.GameStatus.Running
    end,
    can_play = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type ~= Sol.Pile.Type.Foundation and not targetPile.IsEmpty then
            local targetCard = targetPile.Cards[targetCardIndex]
            if targetCard.Suit == "Diamonds" or card.Suit == "Diamonds" then
                return false
            end
        elseif targetPile.Type == Sol.Pile.Type.Tableau and targetPile.IsEmpty then
            if card.Suit == "Diamonds" then
                return false
            end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end
}


------

local dimensions = {
    Info = {
        Name      = "Dimensions",
        Family    = "Other",
        DeckCount = 1
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial  = Sol.Initial.face_down(52)
    },
    Waste = {
        Position = { x = 0, y = 1 }
    },
    Foundation = {
        Size = 3,
        Pile = function(i)
            return {
                Position = { x = 1, y = i },
                Layout   = Sol.Pile.Layout.Row,
                Rule     = {
                    Base = Sol.Rules.Base.Any(),
                    Build = {
                        Hint = { "InSuit" }, --custom By same suit or same rank
                        Func = function(_, base, drop)
                            return base.Suit == drop.Suit or base.Rank == drop.Rank
                        end
                    },
                    Move = Sol.Rules.Move.None(),
                    Limit = -1
                }
            }
        end
    },
    deal = Sol.Ops.Deal.stock_to_waste
}


------

local double_dot = {
    Info = {
        Name      = "Double Dot",
        Family    = "Other",
        DeckCount = 1
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial  = Sol.Initial.face_down(40)
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Rule     = {
                    Base = i < 2 and Sol.Rules.Base.Ace() or Sol.Rules.Base.Ranks({ "Two" }),
                    Build = Sol.Rules.Build.UpInSuit(true, 2),
                    Move = Sol.Rules.Move.Top()
                }
            }
        end
    },
    Tableau = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(true, 2), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    on_before_setup = function(game, card)
        if card.Rank == "Ace" and card.Color == "Black" then
            return game.PlaceTop(card, game.Foundation, 1, 2, true)
        elseif card.Rank == "Two" and card.Color == "Red" then
            return game.PlaceTop(card, game.Foundation, 3, 2, true)
        end

        return false
    end,
    deal = Sol.Ops.Deal.stock_to_tableau
}


------

local four_by_four = {
    Info = {
        Name      = "Four by Four",
        Family    = "Other",
        DeckCount = 1
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial  = Sol.Initial.face_down(51)
    },
    Waste = { Position = { x = 1, y = 0 } },
    Foundation = {
        Position = { x = 4.5, y = 0 },
        Initial  = Sol.Initial.face_up(1),
        Rule     = { Base = Sol.Rules.Base.FirstFoundation(), Build = Sol.Rules.Build.RankPack(true), Move = Sol.Rules.Move.None() }
    },
    Tableau = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 1 },
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.any_updownrank_top
            }
        end
    },
    deal = Sol.Ops.Deal.stock_to_waste
}


------

local six_by_six = {
    Info       = {
        Name      = "Six by Six",
        Family    = "Other",
        DeckCount = 1,
    },
    Stock      = { Initial = Sol.Initial.face_down(16) },
    Foundation = {
        Size = 4,
        Pile = {
            Rule = { Base = Sol.Rules.Base.Ace(), Build = Sol.Rules.Build.UpInSuit(), Move = Sol.Rules.Move.Top() }
        }
    },
    Tableau    = {
        Size = 6,
        Pile = {
            Initial = Sol.Initial.face_up(6),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.InSeqInSuit() }
        }
    },
    on_setup   = function(game, card, pile)
        if pile.Type == Sol.Pile.Type.Tableau and card.Rank == "Ace" then
            return game.PlaceTop(card, game.Foundation, true)
        end

        return false
    end,
    deal       = function(game) return Sol.Ops.Deal.to_pile(game.Stock[1], game.Tableau[1], 1) end,
    on_init    = Sol.Layout.klondike
}


------

local lucky_thirteen_pos <const> = {
    { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 }, { 4, 1 },
    --[[ --]] { 1, 2 }, { 2, 2 }, { 3, 2 }, --[[ --]]
    { 0, 3 }, { 1, 3 }, { 2, 3 }, { 3, 3 }, { 4, 3 }
}

local lucky_thirteen = {
    Info = {
        Name      = "Lucky Thirteen",
        Family    = "Other",
        DeckCount = 1
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 0.5, y = 0 },
                Rule     = Sol.Rules.ace_uprank_top
            }
        end
    },
    Tableau = {
        Size = 13,
        Pile = function(i)
            return {
                Position = { x = lucky_thirteen_pos[i + 1][1], y = lucky_thirteen_pos[i + 1][2] },
                Initial  = Sol.Initial.face_up(4),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = Sol.Rules.none_downrank_top
            }
        end
    }
}


------

local phalanx = {
    Info       = {
        Name      = "Phalanx",
        Family    = "Other",
        DeckCount = 1
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial  = Sol.Initial.face_down(37)
    },
    Waste      = {
        Position = { x = 1, y = 0 }
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            local pos <const> = { { 1.5, 1.5 }, { 6.5, 1.5 }, { 1, 2.5 }, { 7, 2.5 } }
            return {
                Position = { x = pos[i + 1][1], y = pos[i + 1][2] },
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    FreeCell   = {
        Size = 15,
        Pile = function(i)
            local function get_py_cells(n) return (n * (n + 1)) / 2 end
            local function get_py_row(n) return (-1 + math.sqrt((8 * n) + 1)) / 2 end
            local lastRowSize <const>   = 15 - get_py_cells(get_py_row(15) - 1)
            local currentRow <const>    = math.ceil(get_py_row(i + 1))
            local currentColumn <const> = i - (get_py_cells(currentRow - 1))

            return {
                Position = {
                    x = (lastRowSize - currentRow) / 2 + currentColumn + 2,
                    y = currentRow - 1
                },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = Sol.Rules.any_none_top
            }
        end
    },
    deal       = Sol.Ops.Deal.stock_to_waste
}


------

local simplex = {
    Info        = {
        Name      = "Simplex",
        Family    = "Other",
        DeckCount = 1
    },
    Stock       = {
        Position = { x = 0, y = 0 },
        Initial  = Sol.Initial.face_down(43)
    },
    Waste       = {
        Position = { x = 1, y = 0 }
    },
    Foundation  = {
        Position = { x = 2, y = 0 },
        Rule     = Sol.Rules.none_none_none
    },
    Tableau     = {
        Size = 9,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.InSeq() }
            }
        end
    },
    on_end_turn = function(game)
        local tableau = game.Tableau
        for _, tab in ipairs(tableau) do
            if tab.CardCount == 4 then
                tab:move_cards(game.Foundation[1], 1, 4, false)
            end
        end
    end,
    deal        = Sol.Ops.Deal.stock_to_waste
}


------

local turncoats = {
    Info = {
        Name      = "Turncoats",
        Family    = "Other",
        DeckCount = 1
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial  = Sol.Initial.face_down(40)
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau = {
        Size = 12,
        Pile = function(i)
            return {
                Position = { x = i % 6, y = i // 6 + 1 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownAlternateColors(), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    on_end_turn = function(game)
        Sol.Ops.Deal.to_group(game.Stock[1], game.Tableau, Sol.DealMode.IfEmpty)
    end
}


------

local trusty_twelve = {
    Info = {
        Name      = "Trusty Twelve",
        Family    = "Other",
        DeckCount = 1,
        Objective = "AllCardsToTableau"
    },
    Reserve = {
        Position = { x = 0, y = 0 },
        Initial  = Sol.Initial.face_down(40)
    },
    Tableau = {
        Size = 12,
        Pile = function(i)
            return {
                Position = { x = i % 4 + 1, y = i // 4 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    on_end_turn = function(game)
        local reserve = game.Reserve[1]
        Sol.Ops.Deal.to_group(reserve, game.Tableau, Sol.DealMode.IfEmpty)
        reserve:flip_down_cards()
    end,
    can_play = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == Sol.Pile.Type.Tableau and game:find_pile(card).CardCount > 1 then
            return false
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end
}


------

local up_and_up = {
    Info = {
        Name      = "Up and Up",
        Family    = "Other",
        DeckCount = 1,
        Objective = "AllCardsToTableau"
    },
    Reserve = {
        Position = { x = 0, y = 0 },
        Initial  = Sol.Initial.face_up(42)
    },
    Tableau = {
        Size = 10,
        Pile = function(i)
            return {
                Position = { x = i % 5 + 1, y = i // 5 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.UpByRank(true), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    on_end_turn = function(game)
        local reserve = game.Reserve[1]
        Sol.Ops.Deal.to_group(reserve, game.Tableau, Sol.DealMode.IfEmpty)
    end,
    can_play = function(game, targetPile, targetCardIndex, card, numCards)
        local srcPile = game:find_pile(card)
        if targetPile.Type == Sol.Pile.Type.Tableau and srcPile.Type == Sol.Pile.Type.Tableau and srcPile.CardCount > 1 then
            return false
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end
}

------

------------------------

Sol.register_game(board_patience)
Sol.register_game(captive_queens)
Sol.register_game(circle_eight)
Sol.register_game(corner_suit)
Sol.register_game(diamond_mine)
Sol.register_game(dimensions)
Sol.register_game(double_dot)
Sol.register_game(four_by_four)
Sol.register_game(lucky_thirteen)
Sol.register_game(phalanx)
Sol.register_game(simplex)
Sol.register_game(six_by_six)
Sol.register_game(turncoats)
Sol.register_game(trusty_twelve)
Sol.register_game(up_and_up)
