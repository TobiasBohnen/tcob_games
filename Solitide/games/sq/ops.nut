// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

local shuffle = {
    // before shuffle
    ace_to_foundation = function(game, card) {
        if (card.Rank == "Ace") {
            return game.PlaceTop(card, game.Foundation, true)
        }

        return false
    },
    king_to_foundation = function(game, card) {
        if (card.Rank == "King") {
            return game.PlaceTop(card, game.Foundation, true)
        }

        return false
    },
    // after shuffle
    kings_to_bottom = function(game) {
        foreach(v in game.Tableau) {
            v.shift_rank_to_bottom("King")
        }
    }
}

local function redeal_func(from, to) {
    if (to.IsEmpty && !from.IsEmpty) {
        from.move_cards(to, 0, from.Cards.len(), true)
        to.flip_down_cards()
        return true
    }

    return false
}

local redeal = {
    to_pile = redeal_func,
    waste_to_stock = @(game) redeal_func(game.Waste[0], game.Stock[0])
}

local function deal_func(from, to, count) {
    if (from.IsEmpty) {
        return false
    }
    for (local i = 0; i < count; i++) {
        from.move_cards(to, from.Cards.len() - 1, 1, false)
    }
    to.flip_up_cards()
    return true
}

local function deal_group_func(from, to, emptyMode) {
    if (from.IsEmpty) {
        return false
    }
    foreach(toPile in to) {
        if (from.IsEmpty) {
            break
        }
        local check = true
        if (emptyMode == deal_mode.IfEmpty) {
            check = toPile.IsEmpty
        } else if (emptyMode == deal_mode.IfNotEmpty) {
            check = !toPile.IsEmpty
        }

        if (check) {
            from.move_cards(toPile, from.Cards.len() - 1, 1, false)
            toPile.flip_up_top_card()
        }
    }
    return true
}

local deal = {
    to_pile = deal_func,
    to_group = deal_group_func,
    stock_to_waste = @(game) deal_func(game.Stock[0], game.Waste[0], 1),
    stock_to_waste_by_3 = @(game) deal_func(game.Stock[0], game.Waste[0], 3),
    stock_to_waste_by_redeals_left = @(game) deal_func(game.Stock[0], game.Waste[0], game.RedealsLeft + 1),
    waste_or_stock_to_empty_tableau = @(game) deal_group_func(game.Waste[0], game.Tableau, deal_mode.IfEmpty) || deal_group_func(game.Stock[0], game.Tableau, deal_mode.IfEmpty)
}

function get_py_cells(n) {
    return (n * (n + 1)) / 2
}

function get_py_row(n) {
    return (-1 + math.sqrt((8 * n) + 1)) / 2
}

local pyramid = {
    pile = function(size, baseSize, offset, i) {
        local dummyCells = get_py_cells(baseSize - 1)
        local dummyCellRows = get_py_row(dummyCells)
        local totalRows = get_py_row(size + dummyCells)
        local lastRowStart = get_py_cells(totalRows - 1) - dummyCells
        local lastRowSize = size - lastRowStart

        local idx = i + 1
        local currentRow = math.ceil(get_py_row(dummyCells + idx)) - dummyCellRows
        local currentRowSize = currentRow + baseSize - 1
        local currentColumn = i - (get_py_cells(currentRow - 1 + dummyCellRows) - dummyCells)

        return {
            HasMarker = idx < baseSize,
            Layout = Sol.Pile.Layout.Column,
            Initial = idx <= lastRowStart ? Sol.Initial.face_down(1) : Sol.Initial.face_up(1),
            Position = {
                x = (lastRowSize - currentRowSize) / 2 + currentColumn + offset.x,
                y = (currentRow * 0.5) + offset.y
            }
        }
    },

    flip = function(size, baseSize, tableau) {
        local dummyCells = get_py_cells(baseSize - 1)
        local totalRows = get_py_row(size + dummyCells)
        local last = get_py_cells(totalRows - 1) - dummyCells

        for (local tabIdx = 1; tabIdx <= last; tabIdx++) {
            local pile = tableau[tabIdx]
            if (pile.IsEmpty) {
                continue
            }

            local rowSize = math.ceil(get_py_row(dummyCells + tabIdx))
            if (tableau[tabIdx + rowSize + 0].IsEmpty && tableau[tabIdx + rowSize + 1].IsEmpty) {
                pile.flip_up_top_card()
            }
        }
    },

    face_up_pile = function(size, baseSize, offset, i) {
        local dummyCells = get_py_cells(baseSize - 1)
        local totalRows = get_py_row(size + dummyCells)
        local last = get_py_cells(totalRows - 1) - dummyCells

        local pile = Sol.Ops.Pyramid.pile(size, baseSize, offset, i)
        // all cards visible
        pile.Initial = Sol.Initial.face_up(1)
        // only bottom row playable
        local move = Sol.Rules.Move.Top()
        move.IsPlayable = i >= last

        pile.Rule = {
            Base = Sol.Rules.Base.None(),
            Build = Sol.Rules.Build.None(),
            Move = move
        }

        return pile
    },

    face_up_flip = function(size, baseSize, tableau) {
        local dummyCells = get_py_cells(baseSize - 1)
        local totalRows = get_py_row(size + dummyCells)
        local last = get_py_cells(totalRows - 1) - dummyCells

        for (local tabIdx = 1; tabIdx <= last; tabIdx++) {
            local pile = tableau[tabIdx]
            if (pile.IsEmpty) {
                continue
            }

            local rowSize = math.ceil((-1 + math.sqrt((8 * tabIdx) + 1)) / 2)
            pile.IsPlayable = tableau[tabIdx + rowSize + 0].IsEmpty && tableau[tabIdx + rowSize + 1].IsEmpty
        }
    }
}

return {
    Shuffle = shuffle,
    Redeal = redeal,
    Deal = deal,
    Pyramid = pyramid
}