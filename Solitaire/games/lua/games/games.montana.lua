-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local montana_base                = {
    redeal = function(game, ranks)
        local columns = #ranks + 1
        local tableau = game.Tableau
        local cards = {}

        for y = 1, 4 do
            local inOrder = true
            local suit
            for x = 1, columns do
                local tab = tableau[x + (y - 1) * columns]
                if not tab.IsEmpty then
                    if x == 1 then
                        suit = tab.Cards[1].Suit
                    elseif inOrder then
                        inOrder = tab.Cards[1].Suit == suit
                    end

                    inOrder = inOrder and (tab.Cards[1].Rank == ranks[x])
                    if not inOrder then
                        cards[#cards + 1] = tab.Cards[1]
                        tab:clear()
                    end
                end
            end
        end
        cards = game:shuffle_cards(cards)

        for y = 1, 4 do
            local wasEmpty = false
            for x = 1, columns do
                local tab = tableau[x + (y - 1) * columns]
                if tab.IsEmpty then
                    if wasEmpty then
                        game.PlaceTop(table.remove(cards), tab, true)
                    else
                        wasEmpty = true
                    end
                end
            end
        end

        return true
    end,

    check_playable = function(game, targetPile, drop, ranks, mode)
        local tableau = game.Tableau

        local check_left = function(leftTab)
            --check if left card is one rank higher
            if leftTab.IsEmpty then return false end
            local leftCard = leftTab.Cards[1]
            if leftCard.Suit == drop.Suit then
                for k, v in ipairs(ranks) do
                    if k == #ranks then return false end --don't wrap
                    if v == leftCard.Rank then return ranks[k + 1] == drop.Rank end
                end
            end
        end
        local check_right = function(rightTab)
            --check if right card is one rank lower
            if rightTab.IsEmpty then return false end
            local rightCard = rightTab.Cards[1]
            if rightCard.Suit == drop.Suit then
                for k, v in ipairs(ranks) do
                    if k == #ranks then return false end --don't wrap
                    if v == rightCard.Rank then return ranks[k - 1] == drop.Rank end
                end
            end
        end

        local columns = #ranks + 1
        if not targetPile.IsEmpty then return false end

        local checkLeft = string.find(mode, "l")
        local checkRight = string.find(mode, "r")

        for i, tab in ipairs(tableau) do
            if tab == targetPile then
                if i % columns == 1 then return drop.Rank == ranks[1] end --leftmost column

                local result = false

                if checkLeft then
                    result = check_left(tableau[i - 1])
                end
                if result then return true end

                if checkRight and i % columns ~= 0 then
                    result = check_right(tableau[i + 1])
                end
                if result then return true end
            end
        end

        return false
    end,

    check_state = function(game, ranks)
        local columns = #ranks + 1
        local tableau = game.Tableau

        for y = 1, 4 do
            local firstTab = tableau[1 + (y - 1) * columns]
            if firstTab.IsEmpty or firstTab.Cards[1].Rank ~= ranks[1] then return "Running" end
            local suit = firstTab.Cards[1].Suit

            for x = 2, #ranks do
                local tab = tableau[x + (y - 1) * columns]
                if tab.IsEmpty then return "Running" end
                local card = tab.Cards[1]
                if card.Rank ~= ranks[x] or card.Suit ~= suit then return "Running" end
            end
        end

        return "Success"
    end
}

------
local montana_ranks               = { table.unpack(Sol.Ranks, 2, 13) }

local montana                     = {
    Info           = {
        Name          = "Montana",
        Family        = "Montana",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 2,
        DisableHints  = true
    },
    Stock          = {},
    Tableau        = {
        Size   = 52,
        Create = {
            Position = {},
            Initial = ops.Initial.face_up(1),
            Layout = "Squared",
            Rule = rules.none_none_top
        }
    },
    on_shuffle     = function(_, card, _)
        return card.Rank == "Ace"
    end,
    on_created     = function(game) Sol.Layout.montana(game, 13) end,
    on_redeal      = function(game)
        return montana_base.redeal(game, montana_ranks)
    end,
    check_state    = function(game)
        return montana_base.check_state(game, montana_ranks)
    end,
    check_playable = function(game, targetPile, _, drop, _)
        return montana_base.check_playable(game, targetPile, drop, montana_ranks, "l")
    end
}

------

local double_montana              = Sol.copy(montana)
double_montana.Info.Name          = "Double Montana"
double_montana.Info.DeckCount     = 2
double_montana.Tableau.Size       = 104

------

local moonlight                   = Sol.copy(montana)
moonlight.Info.Name               = "Moonlight"
moonlight.check_playable          = function(game, targetPile, _, drop, _)
    return montana_base.check_playable(game, targetPile, drop, montana_ranks, "rl")
end

------
local blue_moon_ranks             = Sol.Ranks

local blue_moon_shuffle           = function(game, card, rows)
    if card.Rank == "Ace" then
        for i = 0, rows - 1 do
            if game.PlaceTop(card, game.Tableau, 1 + i * 14, 1, true) then
                return true
            end
        end
    end

    return false
end

local blue_moon                   = {
    Info           = {
        Name          = "Blue Moon",
        Family        = "Montana",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 2,
        DisableHints  = true
    },
    Stock          = {},
    Tableau        = {
        Size   = 56,
        Create = function(i)
            return {
                Initial = ops.Initial.face_up(i % 14 == 0 and 0 or 1),
                Layout = "Squared",
                Rule = rules.none_none_top
            }
        end
    },
    on_shuffle     = function(game, card, _)
        return blue_moon_shuffle(game, card, 4)
    end,
    check_playable = function(game, targetPile, _, drop, _)
        return montana_base.check_playable(game, targetPile, drop, blue_moon_ranks, "l")
    end,
    on_redeal      = function(game)
        return montana_base.redeal(game, blue_moon_ranks)
    end,
    check_state    = function(game)
        return montana_base.check_state(game, blue_moon_ranks)
    end,
    on_created     = function(game) Sol.Layout.montana(game, 14) end
}

------

local double_blue_moon            = Sol.copy(blue_moon)
double_blue_moon.Info.Name        = "Double Blue Moon"
double_blue_moon.Info.DeckCount   = 2
double_blue_moon.Tableau.Size     = 112
double_blue_moon.on_shuffle       = function(game, card, _)
    return blue_moon_shuffle(game, card, 8)
end

------

local red_moon                    = Sol.copy(blue_moon)
red_moon.Info.Name                = "Red Moon"
red_moon.Tableau.Create           = function(i)
    return {
        Initial = ops.Initial.face_up((i % 14 < 2) and 0 or 1),
        Layout = "Squared",
        Rule = rules.none_none_top
    }
end
red_moon.on_before_shuffle        = blue_moon.on_shuffle
red_moon.on_shuffle               = nil

------

local double_red_moon             = Sol.copy(red_moon)
double_red_moon.Info.Name         = "Double Red Moon"
double_red_moon.Info.DeckCount    = 2
double_red_moon.Tableau.Size      = 112
double_red_moon.on_before_shuffle = function(game, card, _)
    return blue_moon_shuffle(game, card, 8)
end

------

local galary                      = Sol.copy(blue_moon)
galary.Info.Name                  = "Galary"
galary.Tableau.Create             = function(i)
    return {
        Initial = ops.Initial.face_up((i % 14 == 0 or i % 14 == 1) and 0 or 1),
        Layout = "Squared",
        Rule = rules.none_none_top
    }
end
galary.on_before_shuffle          = function(game, card)
    return blue_moon.on_shuffle(game, card)
end
galary.on_shuffle                 = nil
galary.check_playable             = function(game, targetPile, _, drop, _)
    return montana_base.check_playable(game, targetPile, drop, blue_moon_ranks, "rl")
end

------
local paganini_ranks              = { "Ace", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King" }

local paganini                    = {
    Info           = {
        Name          = "Paganini",
        Family        = "Montana",
        DeckCount     = 1,
        DeckRanks     = paganini_ranks,
        CardDealCount = 0,
        Redeals       = 1,
        DisableHints  = true
    },
    Stock          = {},
    Tableau        = {
        Size   = 40,
        Create = function(i)
            return {
                Initial = ops.Initial.face_up(i % 10 == 0 and 0 or 1),
                Layout = "Squared",
                Rule = rules.none_none_top
            }
        end
    },
    on_shuffle     = function(game, card, _)
        if card.Rank == "Ace" then
            return game.PlaceTop(card, game.Tableau, 1, 1, true)
                or game.PlaceTop(card, game.Tableau, 11, 1, true)
                or game.PlaceTop(card, game.Tableau, 21, 1, true)
                or game.PlaceTop(card, game.Tableau, 31, 1, true)
        end

        return false
    end,
    check_playable = function(game, targetPile, _, drop, _)
        return montana_base.check_playable(game, targetPile, drop, paganini_ranks, "l")
    end,
    on_redeal      = function(game)
        return montana_base.redeal(game, paganini_ranks)
    end,
    check_state    = function(game)
        return montana_base.check_state(game, paganini_ranks)
    end,
    on_created     = function(game) Sol.Layout.montana(game, 10) end
}

------
local spoilt_ranks                = { "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King", "Ace" }

local spoilt                      = {
    Info           = {
        Name          = "Spoilt",
        Family        = "Montana",
        DeckCount     = 1,
        DeckRanks     = spoilt_ranks,
        CardDealCount = 1,
        Redeals       = 0
    },
    Stock          = {
        Position = { x = 0, y = 1 },
        Initial = ops.Initial.face_down(3)
    },
    Waste          = {
        Position = { x = 0, y = 2 },
        Initial = ops.Initial.face_up(1)
    },
    Tableau        = {
        Size   = 32,
        Create = function(i)
            return {
                Position = { x = i % 8 + 1, y = i // 8 },
                Initial = ops.Initial.face_down(i % 8 == 0 and 0 or 1),
                Layout = "Squared",
                Rule = rules.none_none_none
            }
        end
    },
    on_change      = function(game)
        local sevenCount = 0
        for i, tableau in ipairs(game.Tableau) do
            if tableau.CardCount == 2 then
                tableau:move_cards(game.Waste[1], 1, 1, false)
                game.Waste[1]:flip_up_top_card()
                return
            elseif (i - 1) % 8 == 0 and not tableau.IsEmpty then
                sevenCount = sevenCount + 1
            end
        end

        if sevenCount == 4 then
            for _, tableau in ipairs(game.Tableau) do
                tableau:flip_up_top_card()
            end
        else
            if not game.Stock[1].IsEmpty and game.Waste[1].IsEmpty then
                ops.Deal.stock_to_waste(game)
            end
        end
    end,
    check_playable = function(game, targetPile, _, drop, _)
        local tableau = game.Tableau

        --get suits per row
        local rowEmpty = { true, true, true, true }
        local suitRows = {}
        for y = 1, 4 do
            for x = 1, 8 do
                local tab = tableau[x + (y - 1) * 8]
                if not tab.IsEmpty and tab.Cards[1].IsFaceUp then
                    rowEmpty[y] = false
                    suitRows[tab.Cards[1].Suit] = y
                    break
                end
            end
        end

        --check rank and suit
        for i, tab in ipairs(tableau) do
            if tab == targetPile then
                local x = (i - 1) % 8 + 1
                if drop.Rank == spoilt_ranks[x] then                          --rank fits
                    local y = (i - 1) // 8 + 1
                    if suitRows[drop.Suit] == y then return true end          --suit fits row
                    return suitRows[drop.Suit] == nil and rowEmpty[y] == true --suit not used elsewhere, and row is empty
                end
            end
        end

        return false
    end,

    check_state    = function(game)
        if game.Stock[1].IsEmpty and game.Waste[1].IsEmpty then
            --check rank and suit
            local tableau = game.Tableau
            for i, tab in ipairs(tableau) do
                local x = (i - 1) % 8 + 1
                local y = (i - 1) // 8 + 1
                local card = tab.Cards[1]
                if card.Rank ~= spoilt_ranks[x] or card.Suit ~= tableau[y * 7].Cards[1].Suit then
                    return "Failure"
                end
            end
            return "Success"
        end

        return "Running"
    end,
}

------------------------

Sol.register_game(montana)
Sol.register_game(double_montana)
Sol.register_game(blue_moon)
Sol.register_game(double_blue_moon)
Sol.register_game(galary)
Sol.register_game(paganini)
Sol.register_game(moonlight)
Sol.register_game(red_moon)
Sol.register_game(double_red_moon)
Sol.register_game(spoilt)
