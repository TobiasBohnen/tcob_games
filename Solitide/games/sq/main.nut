// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

Sol.Rules <- require("rules")
Sol.Ops <- require("ops")

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
Sol.Suits <- ["Hearts", "Diamonds", "Spades", "Clubs"]
Sol.SuitColors <- {
    Hearts = "Red",
    Diamonds = "Red",
    Spades = "Black",
    Clubs = "Black"
}

function Sol::get_rank(r, interval, wrap) {
    local target = Sol.RankValues[r] - 1 + interval

    if (wrap) {
        target = target % 13
    }

    if (target < 0 || target > 13) {
        return ""
    }

    return Sol.Ranks[target]
}

function Sol::shuffle_piles(game) {
    local cards = []

    foreach(pile in pileGroups) {
        foreach(card in pile.Cards) {
            cards.append(card)
        }
        pile.clear_cards()
    }

    return game.shuffle_cards(cards)
}

Sol.Initial <- {
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

enum deal_mode {
    IfEmpty,
    IfNotEmpty,
    Always
}

Sol.Pile <- {
    Type = {
        Stock = "Stock",
        Waste = "Waste",
        FreeCell = "FreeCell",
        Reserve = "Reserve",
        Foundation = "Foundation",
        Tableau = "Tableau"
    },
    Layout = {
        Squared = "Squared",
        Column = "Column",
        Row = "Row",
        Fan = "Fan"
    }
}
Sol.GameStatus <- {
    Running = "Running",
    Failure = "Failure",
    Success = "Success"
}