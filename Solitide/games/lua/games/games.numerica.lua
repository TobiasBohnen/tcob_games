-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local numerica = {
    Info        = {
        Name      = "Numerica",
        Family    = "Numerica",
        DeckCount = 1
    },
    Stock       = {
        Initial = Sol.Initial.face_down(51)
    },
    Waste       = {
        Initial = Sol.Initial.face_up(1)
    },
    Foundation  = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_uprank_top }
    },
    Tableau     = {
        Size = 4,
        Pile = {
            Layout = "Column",
            Rule   = Sol.Rules.any_any_top
        }
    },
    can_play    = function(game, targetPile, targetCardIndex, card, numCards)
        -- Tableau piles can only be build from the Waste
        if targetPile.Type == "Tableau" then
            local srcPile = game:find_pile(card)
            if srcPile.Type ~= "Waste" then return false end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    on_end_turn = function(game)
        local waste = game.Waste[1]

        if waste.IsEmpty then
            Sol.Ops.Deal.stock_to_waste(game)
        end
    end,
    on_init     = Sol.Layout.klondike
}


------

local numerica_2_decks           = Sol.copy(numerica)
numerica_2_decks.Info.Name       = "Numerica (2 Decks)"
numerica_2_decks.Info.DeckCount  = 2
numerica_2_decks.Stock.Initial   = Sol.Initial.face_down(103)
numerica_2_decks.Foundation.Size = 8
numerica_2_decks.Tableau.Size    = 6


------

local assembly         = Sol.copy(numerica)
assembly.Info.Name     = "Assembly"
assembly.Stock.Initial = Sol.Initial.face_down(47)
assembly.Tableau.Pile  = {
    Initial = Sol.Initial.face_up(1),
    Layout  = "Column",
    Rule    = Sol.Rules.any_downrank_top
}
assembly.can_play      = function(game, targetPile, targetCardIndex, card, numCards)
    -- empty Tableau piles can only be filled from the Waste
    if targetPile.Type == "Tableau" and targetPile.IsEmpty then
        local srcPile = game:find_pile(card)
        if srcPile.Type ~= "Waste" then return false end
    end

    return game:can_play(targetPile, targetCardIndex, card, numCards)
end
assembly.deal          = Sol.Ops.Deal.stock_to_waste


------
local amazons_ranks <const>      = { "Ace", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen" }
local amazons_rankvalues <const> = {
    Ace = 1,
    Seven = 2,
    Eight = 3,
    Nine = 4,
    Ten = 5,
    Jack = 6,
    Queen = 7
}

local amazons                    = {
    Info       = {
        Name      = "Amazons",
        Family    = "Numerica",
        DeckCount = 1,
        Redeals   = -1,
        DeckRanks = amazons_ranks
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(24)
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 1, y = 0 },
                Rule     = {
                    Base = Sol.Rules.Base.Ace(),
                    Build = {
                        Hint = "Up by rank",
                        Func = function(_, dst, src)
                            local target = amazons_rankvalues[dst.Rank] + 1
                            return amazons_ranks[target] == src.Rank
                        end
                    },
                    Move = Sol.Rules.Move.None(),
                    Limit = 13
                }
            }
        end
    },
    Tableau    = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 1, y = 1 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = "Column",
                Rule     = Sol.Rules.none_none_top
            }
        end
    },
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        -- seven to jack can only be played on the foundation pile above the tableau pile
        if targetPile.Type == "Foundation" and card.Rank ~= "Queen" and card.Rank ~= "Ace" then
            local srcPile = game:find_pile(card)
            if srcPile.Type == "Tableau" and srcPile.Index ~= targetPile.Index then
                return false
            end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    deal       = function(game)
        local from       = game.Stock[1]
        local to         = game.Tableau
        local foundation = game.Foundation

        if from.IsEmpty then return false end
        for idx, toPile in ipairs(to) do
            if from.IsEmpty then break end
            if foundation[idx].CardCount < 7 then
                from:move_cards(toPile, #from.Cards, 1, false)
                toPile:flip_up_top_card()
            end
        end
        return true
    end,
    redeal     = function(game)
        local stock = game.Stock[1]
        if not stock.IsEmpty then return false end

        local tableau = game.Tableau
        for idx = #tableau, 1, -1 do
            local tab = tableau[idx]
            if not tab.IsEmpty then
                local cards = tab.Cards
                for _ = 1, #cards do
                    tab:move_cards(stock, 1, 1, true)
                end
                stock:flip_down_cards()
            end
        end
        return true
    end
}


------


local toad = {
    Info       = {
        Name      = "Toad",
        Family    = "Numerica",
        DeckCount = 2,
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(104)
    },
    Reserve    = {
        Size = 13,
        Pile = function(i)
            return {
                Position = { x = i % 5, y = i // 5 + 1 }
            }
        end
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 4, y = 0 },
                Rule     = Sol.Rules.ace_upsuit_none
            }
        end
    },
    Tableau    = {
        Size = 5,
        Pile = function(i)
            return {
                Position = { x = i + 5.5, y = 1 },
                Layout   = "Column",
                Rule     = Sol.Rules.any_any_top
            }
        end
    },
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == "Tableau" and game:find_pile(card).Type == "Tableau" then
            return false
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    deal       = function(game)
        local reserve = game.Reserve
        for _, rev in ipairs(reserve) do
            if not rev.IsEmpty then return false end
        end

        return Sol.Ops.Deal.to_group(game.Stock[1], reserve)
    end
}


------

local amphibian           = Sol.copy(toad)
amphibian.Info.Name       = "Amphibian"
amphibian.Reserve.Size    = 4
amphibian.Foundation.Pile = function(i)
    return {
        Position = { x = i + 4, y = 0 },
        Rule     = Sol.Rules.ace_uprank_none
    }
end


------


local anno_domini = {
    Info       = {
        Name      = "Anno Domini",
        Family    = "Numerica",
        DeckCount = 1,
        Redeals   = 2
    },
    Stock      = {
        Initial = Sol.Initial.face_down(48)
    },
    Waste      = {},
    Foundation = {
        Size = 4,
        Pile = function(i)
            local base = tonumber(string.sub(Sol.Date, i + 1, i + 1)) + 1
            local ranks = { "Jack", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", }
            return {
                Rule = { Base = Sol.Rules.Base.Ranks({ ranks[base] }), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    Tableau    = {
        Size = 4,
        Pile = {
            Initial = Sol.Initial.face_up(1),
            Layout  = "Column",
            Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownAlternateColors(true), Move = Sol.Rules.Move.InSeq() }
        }
    },
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == "Foundation" then
            for _, fou in ipairs(game.Foundation) do
                if not fou.IsEmpty and fou.Cards[1].Suit == card.Suit then return false end
            end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    deal       = Sol.Ops.Deal.stock_to_waste,
    on_init    = Sol.Layout.klondike
}


------


------------------------

Sol.register_game(numerica)
Sol.register_game(amazons)
Sol.register_game(amphibian)
Sol.register_game(anno_domini)
Sol.register_game(assembly)
Sol.register_game(numerica_2_decks)
Sol.register_game(toad)
