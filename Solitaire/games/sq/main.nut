// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

function Sol::copy(obj) {
    switch (typeof(obj)) {
        case "table":
            local retValue = clone obj
            foreach(k, v in obj) {
                retValue[k] = Sol.copy(v)
            }
            return retValue
        case "array":
            return obj.map(Sol.copy)
        default:
            return obj
    }
}

Sol.RankValues <- {
    Ace = 1,
    Two = 2,
    Three = 3,
    Four = 4,
    Five = 5,
    Six = 6,
    Seven = 7,
    Eight = 8,
    Nine = 9,
    Ten = 10,
    Jack = 11,
    Queen = 12,
    King = 13
}

Sol.Ranks <- ["Ace", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King"]

function Sol::get_rank(r, interval, wrap) {
    local target = Sol.RankValues[r] - 1 + interval

    if (wrap) {
        target = target % 13
    }

    if (target == 0 || target > 13) {
        return ""
    }

    return Sol.Ranks[target]
}


function Sol::shuffle_tableau(game) {
    local cards = []

    local tableau = game.Tableau
    foreach(tab in tableau) {
        foreach(card in tab.Cards) {
            cards.append(card)
        }
        tab.clear()
    }

    return game.shuffle_cards(cards)
}