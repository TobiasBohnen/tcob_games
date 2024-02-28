// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

local shuffle_tab = {
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

local redeal_tab = {
    waste_to_stock = @(game) game.Waste[0].redeal(game.Stock[0])
}

local deal_tab = {
    stock_to_waste = @(game) game.Stock[0].deal(game.Waste[0], game.CardDealCount),
    stock_to_waste_by_redeals_left = @(game) game.Stock[0].deal(game.Waste[0], game.RedealsLeft + 1)
}

return {
    shuffle = shuffle_tab,
    redeal = redeal_tab,
    deal = deal_tab
}