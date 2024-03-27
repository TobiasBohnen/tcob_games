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
            v.move_rank_to_bottom("King")
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

local function deal_group_func(from, to, ifEmpty) {
    if (from.IsEmpty) {
        return false
    }
    foreach(toPile in to) {
        if (from.IsEmpty) {
            break
        }
        if (ifEmpty == toPile.IsEmpty) {
            from.move_cards(toPile, from.Cards.len() - 1, 1, false)
            toPile.flip_up_top_card()
        }
    }
    return true
}

local deal = {
    to_pile = deal_func,
    to_group = deal_group_func,
    stock_to_waste = @(game) deal_func(game.Stock[0], game.Waste[0], game.CardDealCount),
    stock_to_waste_by_redeals_left = @(game) deal_func(game.Stock[0], game.Waste[0], game.RedealsLeft + 1)
}

local initial = {
    top_face_up = function(size) {
        local retValue = []
        for (local i = 0; i < size - 1; i += 1) {
            retValue.append(false)
        }
        retValue.append(true)
        return retValue
    },
    face_up = function(size) {
        local retValue = []
        for (local i = 0; i < size; i += 1) {
            retValue.append(true)
        }
        return retValue
    },
    face_down = function(size) {
        local retValue = []
        for (local i = 0; i < size; i += 1) {
            retValue.append(false)
        }
        return retValue
    },
    alternate = function(size, first) {
        local retValue = []
        for (local i = 0; i < size; i += 1) {
            retValue.append(first)
            first = !first
        }
        return retValue
    }
}

return {
    Initial = initial,
    Shuffle = shuffle,
    Redeal = redeal,
    Deal = deal
}