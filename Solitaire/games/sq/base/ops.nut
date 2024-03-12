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

local redeal = {
    waste_to_stock = @(game) game.Waste[0].redeal(game.Stock[0])
}

local deal = {
    stock_to_waste = @(game) game.Stock[0].deal(game.Waste[0], game.CardDealCount),
    stock_to_waste_by_redeals_left = @(game) game.Stock[0].deal(game.Waste[0], game.RedealsLeft + 1)
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