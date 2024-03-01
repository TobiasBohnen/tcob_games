// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

function Copy(obj) {
    switch (typeof(obj)) {
        case "table":
            local retValue = clone obj
            foreach(k, v in obj) {
                result[k] = Copy(v)
            }
            return retValue
        case "array":
            return obj.map(Copy)
        default:
            return obj
    }
}

RankValues <- {
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

Ranks <- ["Ace", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King"]

function GetRank(r, interval, wrap) {
    local rank = RankValues[r] - 1
    local target = rank + interval

    if (wrap) {
        target = target % 13
    }

    if (target == 0 || target > 13) {
        return ""
    }

    return Ranks[target]
}


function ShuffleTableau(game) {
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