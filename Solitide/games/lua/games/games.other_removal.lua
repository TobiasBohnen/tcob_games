-- Copyright (c) 2025 Tobias Bohnen
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local function move_multiple(game)
    local foundation = game.Foundation[1]
    for _, tab in ipairs(game.Tableau) do
        if tab.CardCount > 1 then
            tab:move_cards(foundation, 1, tab.CardCount, false)
        end
    end

    Sol.Ops.Deal.to_group(game.Stock[1], game.Tableau, Sol.DealMode.IfEmpty)
end

------
local aces_up = {
    Info = {
        Name = "Aces Up",
        Family = "Other",
        DeckCount = 1,
        Objective = "AllCardsButFourToFoundation"
    },
    Stock = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(48)
    },
    Foundation = {
        Position = { x = 2, y = 0 },
        Rule = { Move = Sol.Rules.Move.None() }
    },
    Tableau = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 1, y = 1 },
                Initial = Sol.Initial.face_up(1),
                Layout = Sol.Pile.Layout.Column,
                Rule = {
                    Base = Sol.Rules.Base.Any(),
                    Build = Sol.Rules.Build.None(),
                    Move = Sol.Rules.Move.Top()
                }
            }
        end
    },
    can_play = function(game, targetPile, _, card, numCards)
        if targetPile.Type == Sol.Pile.Type.Foundation then
            if numCards ~= 1 then return false end
            if card.Rank == "Ace" then return false end
            if game:find_pile(card).Type ~= Sol.Pile.Type.Tableau then return false end

            local cardRank = Sol.RankValues[card.Rank]
            for _, tab in ipairs(game.Tableau) do
                if not tab.IsEmpty then
                    local tabCard = tab.Cards[tab.CardCount]
                    if tabCard.Suit == card.Suit and (tabCard.Rank == "Ace" or Sol.RankValues[tabCard.Rank] > cardRank) then
                        return true
                    end
                end
            end
        elseif targetPile.Type == Sol.Pile.Type.Tableau or targetPile.Type == Sol.Pile.Type.FreeCell then
            return targetPile.IsEmpty
        end

        return false
    end,
    get_status = function(game)
        if game.Foundation[1].CardCount == 48 then
            return Sol.GameStatus.Success
        end

        if game.Stock[1].IsEmpty then
            local suits = {}
            for _, tab in ipairs(game.Tableau) do
                if tab.IsEmpty or suits[tab.Cards[tab.CardCount].Suit] then
                    return Sol.GameStatus.Running
                end
                suits[tab.Cards[tab.CardCount].Suit] = true
            end
            return Sol.GameStatus.Failure
        end

        return Sol.GameStatus.Running
    end,
    deal = function(game)
        return Sol.Ops.Deal.to_group(game.Stock[1], game.Tableau, Sol.DealMode.Always)
    end
}

------
local aces_up_5 = Sol.copy(aces_up)
aces_up_5.Info.Name = "Aces Up 5"
aces_up_5.Stock.Initial = Sol.Initial.face_down(47)
aces_up_5.Tableau.Size = 5

------
local firing_squad = Sol.copy(aces_up)
firing_squad.Info.Name = "Firing Squad"
firing_squad.FreeCell = {
    Position = { x = 0, y = 1 },
    Layout = Sol.Pile.Layout.Column,
    Rule = {
        Base = Sol.Rules.Base.Any(),
        Build = Sol.Rules.Build.None(),
        Move = Sol.Rules.Move.Top()
    }
}

------
local fortunes = Sol.copy(aces_up)
fortunes.Info.Name = "Fortunes"
fortunes.Tableau.Pile = function(i)
    return {
        Position = { x = i + 1, y = 1 },
        Initial = Sol.Initial.face_up(1),
        Layout = Sol.Pile.Layout.Column,
        Rule = {
            Base = Sol.Rules.Base.Any(),
            Build = Sol.Rules.Build.None(),
            Move = Sol.Rules.Move.FaceUp()
        }
    }
end

------
local russian_aces = Sol.copy(aces_up)
russian_aces.Info.Name = "Russian Aces"
russian_aces.deal = function(game)
    local mode = Sol.DealMode.Always
    for _, tab in ipairs(game.Tableau) do
        if tab.IsEmpty then
            mode = Sol.DealMode.IfEmpty
            break
        end
    end
    return Sol.Ops.Deal.to_group(game.Stock[1], game.Tableau, mode)
end

------
local aces_square = {
    Info = {
        Name = "Aces Square",
        Family = "Other",
        DeckCount = 1,
        Objective = "AllCardsButFourToFoundation"
    },
    Stock = {
        Position = { x = 4.5, y = 2 },
        Initial = Sol.Initial.face_down(36)
    },
    Foundation = {
        Position = { x = 4.5, y = 0 },
        Rule = { Move = Sol.Rules.Move.None() }
    },
    Tableau = {
        Size = 16,
        Pile = function(i)
            return {
                Position = { x = i % 4, y = i // 4 },
                Initial = Sol.Initial.face_up(1),
                Layout = Sol.Pile.Layout.Column,
                Rule = {
                    Base = Sol.Rules.Base.None(),
                    Build = Sol.Rules.Build.None(),
                    Move = Sol.Rules.Move.Top()
                }
            }
        end
    },
    can_play = function(game, targetPile, _, card, numCards)
        if card.Rank == "Ace" or targetPile.IsEmpty or numCards > 1 then
            return false
        end
        if targetPile.Type == Sol.Pile.Type.Tableau then
            local targetCard = targetPile.Cards[1]
            if targetCard.Rank == "Ace" or targetCard.Suit ~= card.Suit then
                return false
            end
            local tableau = game.Tableau
            local srcPile = game:find_pile(card)
            local dstIndex, srcIndex = -1, -1
            for i, tab in ipairs(tableau) do
                if tab == targetPile then dstIndex = i end
                if tab == srcPile then srcIndex = i end
            end
            if dstIndex == -1 or srcIndex == -1 then return false end
            return dstIndex // 4 == srcIndex // 4 or dstIndex % 4 == srcIndex % 4
        end
        return false
    end,
    get_status = function(game)
        if game.Foundation[1].CardCount == 48 then
            return Sol.GameStatus.Success
        end
        return Sol.GameStatus.Running
    end,
    on_end_turn = move_multiple
}

------
local cover = {
    Info = {
        Name = "Cover",
        Family = "Other",
        DeckCount = 1,
        Objective = "AllCardsButFourToFoundation"
    },
    Stock = {
        Initial = Sol.Initial.face_down(48)
    },
    Foundation = {
        Rule = { Move = Sol.Rules.Move.None() }
    },
    Tableau = {
        Size = 4,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(1),
                Layout = Sol.Pile.Layout.Column,
                Rule = {
                    Base = Sol.Rules.Base.None(),
                    Build = {
                        Hint = { "InSuit" },
                        Func = function(_, cbase, drop) return cbase.Suit == drop.Suit end
                    },
                    Move = Sol.Rules.Move.Top()
                }
            }
        end
    },
    get_status = function(game)
        if game.Foundation[1].CardCount == 48 then
            return Sol.GameStatus.Success
        end
        local suits = {}
        for _, tab in ipairs(game.Tableau) do
            if suits[tab.Cards[tab.CardCount].Suit] then
                return Sol.GameStatus.Running
            end
            suits[tab.Cards[tab.CardCount].Suit] = true
        end
        return Sol.GameStatus.Failure
    end,
    on_end_turn = move_multiple,
    on_init = function(game) Sol.Layout.klondike(game) end
}

------
local deck = Sol.copy(cover)
deck.Info.Name = "Deck"
deck.deal = function(game) Sol.Ops.Deal.to_group(game.Stock[1], game.Tableau, Sol.DealMode.Always) end
deck.get_status = function(game)
    if game.Foundation[1].CardCount == 48 then return Sol.GameStatus.Success end
    if game.Stock[1].IsEmpty then
        local suits = {}
        for _, tab in ipairs(game.Tableau) do
            if not tab.IsEmpty then
                if suits[tab.Cards[tab.CardCount].Suit] then return Sol.GameStatus.Running end
                suits[tab.Cards[tab.CardCount].Suit] = true
            end
        end
        return Sol.GameStatus.Failure
    end
    return Sol.GameStatus.Running
end
deck.on_drop = function(game, pile)
    pile:move_cards(game.Foundation[1], pile.CardCount - 2, 2, false)
end
deck.on_end_turn = nil

------

---------------------------

Sol.register_game(aces_up)
Sol.register_game(aces_up_5)
Sol.register_game(aces_square)
Sol.register_game(cover)
Sol.register_game(deck)
Sol.register_game(firing_squad)
Sol.register_game(fortunes)
Sol.register_game(russian_aces)
