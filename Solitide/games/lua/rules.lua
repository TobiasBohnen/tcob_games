-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local rules = {
    in_suit = function(card0, card1)
        return card0.Suit == card1.Suit
    end,

    in_color = function(card0, card1)
        return Sol.SuitColors[card0.Suit] == Sol.SuitColors[card1.Suit]
    end,
    alternate_color = function(card0, card1)
        return Sol.SuitColors[card0.Suit] ~= Sol.SuitColors[card1.Suit]
    end,

    in_rank = function(card0, card1)
        return card0.Rank == card1.Rank
    end,

    build_up = function(card0, card1, wrap, interval)
        return Sol.get_rank(card0.Rank, interval, wrap) == card1.Rank
    end,
    build_down = function(card0, card1, wrap, interval)
        return Sol.get_rank(card0.Rank, -interval, wrap) == card1.Rank
    end,

    build_up_or_down = function(card0, card1, wrap, interval)
        return Sol.get_rank(card0.Rank, interval, wrap) == card1.Rank or Sol.get_rank(card0.Rank, -interval, wrap) == card1.Rank
    end
}

local build = {
    None                    = function(_, _)
        return {
            Hint = "None",
            Func = function(_, _, _)
                return false
            end
        }
    end,
    Any                     = function(_, _)
        return {
            Hint = "Any card",
            Func = function(_, _, _)
                return true
            end
        }
    end,

    InRank                  = function(_, _)
        return {
            Hint = "By same rank",
            Func = function(_, target, card)
                return rules.in_rank(target, card)
            end
        }
    end,

    InRankOrDownByRank      = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Down by rank or by same rank",
            Func = function(_, target, card)
                return rules.in_rank(target, card) or rules.build_down(target, card, wrap, interval)
            end
        }
    end,

    RankPack                = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "By same rank, then up by rank",
            Func = function(game, target, card)
                local count = game:find_pile(target).CardCount
                if count > 0 and count % (game.DeckCount * 4) == 0 then
                    return rules.build_up(target, card, wrap, interval)
                end
                return rules.in_rank(target, card)
            end
        }
    end,

    UpOrDownByRank          = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Up or down by rank",
            Func = function(_, target, card)
                return rules.build_up_or_down(target, card, wrap, interval)
            end
        }
    end,
    DownByRank              = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Down by rank",
            Func = function(_, target, card)
                return rules.build_down(target, card, wrap, interval)
            end
        }
    end,
    UpByRank                = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Up by rank",
            Func = function(_, target, card)
                return rules.build_up(target, card, wrap, interval)
            end
        }
    end,

    UpOrDownAnyButOwnSuit   = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Up or down by any suit but own",
            Func = function(_, target, card)
                return not rules.in_suit(target, card) and rules.build_up_or_down(target, card, wrap, interval)
            end
        }
    end,
    DownAnyButOwnSuit       = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Down by any suit but own",
            Func = function(_, target, card)
                return not rules.in_suit(target, card) and rules.build_down(target, card, wrap, interval)
            end
        }
    end,
    UpAnyButOwnSuit         = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Up by any suit but own",
            Func = function(_, target, card)
                return not rules.in_suit(target, card) and rules.build_up(target, card, wrap, interval)
            end
        }
    end,

    UpOrDownInSuit          = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Up or down by suit",
            Func = function(_, target, card)
                return rules.in_suit(target, card) and rules.build_up_or_down(target, card, wrap, interval)
            end
        }
    end,
    DownInSuit              = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Down by suit",
            Func = function(_, target, card)
                return rules.in_suit(target, card) and rules.build_down(target, card, wrap, interval)
            end
        }
    end,
    UpInSuit                = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Up by suit",
            Func = function(_, target, card)
                return rules.in_suit(target, card) and rules.build_up(target, card, wrap, interval)
            end
        }
    end,

    UpOrDownInColor         = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Up or down by color",
            Func = function(_, target, card)
                return rules.in_color(target, card) and rules.build_up_or_down(target, card, wrap, interval)
            end
        }
    end,
    DownInColor             = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Down by color",
            Func = function(_, target, card)
                return rules.in_color(target, card) and rules.build_down(target, card, wrap, interval)
            end
        }
    end,
    UpInColor               = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Up by color",
            Func = function(_, target, card)
                return rules.in_color(target, card) and rules.build_up(target, card, wrap, interval)
            end
        }
    end,

    UpOrDownAlternateColors = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Up or down by alternate color",
            Func = function(_, target, card)
                return rules.alternate_color(target, card) and rules.build_up_or_down(target, card, wrap, interval)
            end
        }
    end,
    DownAlternateColors     = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Down by alternate color",
            Func = function(_, target, card)
                return rules.alternate_color(target, card) and rules.build_down(target, card, wrap, interval)
            end
        }
    end,
    UpAlternateColors       = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Up by alternate color",
            Func = function(_, target, card)
                return rules.alternate_color(target, card) and rules.build_up(target, card, wrap, interval)
            end
        }
    end,
}

local move = {
    None = function()
        return {
            Hint = "None",
            IsPlayable = false,
            IsSequence = false
        }
    end,
    Top = function()
        return {
            Hint = "Top card",
            IsSequence = false,
            Func = function(_, target, idx)
                return idx == target.CardCount
            end
        }
    end,
    TopOrPile = function()
        return {
            Hint = "Top card or whole pile",
            Func = function(_, target, idx)
                return idx == target.CardCount or idx == 1
            end
        }
    end,
    FaceUp = function()
        return {
            Hint = "Face-up cards",
            Func = function(_, target, idx)
                return target.Cards[idx].IsFaceUp
            end
        }
    end,
    InSeq = function()
        return {
            Hint = "Sequence of cards",
            Func = function(game, target, idx)
                local cards = target.Cards
                if cards[idx].IsFaceDown then return false end
                for i = idx, #cards - 1 do
                    if not game:can_play(target, i, cards[i + 1], 1) then return false end
                end
                return true
            end
        }
    end,
    InSeqInSuit = function() -- in sequence and in suit
        return {
            Hint = "Sequence of cards in the same suit",
            Func = function(game, target, idx)
                local cards = target.Cards
                if cards[idx].IsFaceDown then return false end

                local targetSuit = cards[idx].Suit
                for i = idx, #cards - 1 do
                    if not game:can_play(target, i, cards[i + 1], 1)
                        or cards[i + 1].Suit ~= targetSuit
                    then
                        return false
                    end
                end
                return true
            end
        }
    end,
    InSeqAlternateColors = function() -- in sequence and alternate colors
        return {
            Hint = "Color-alternating card sequence",
            Func = function(game, target, idx)
                local cards = target.Cards
                if cards[idx].IsFaceDown then return false end

                for i = idx, #cards - 1 do
                    local targetColor = Sol.SuitColors[cards[i].Suit]
                    if not game:can_play(target, i, cards[i + 1], 1)
                        or Sol.SuitColors[cards[i + 1].Suit] == targetColor
                    then
                        return false
                    end
                end
                return true
            end
        }
    end,
    InSeqInSuitOrSameRank = function() -- in sequence and (in suit or same rank)
        return {
            Hint = "Sequence of cards in the same suit or rank",
            Func = function(game, target, idx)
                local cards = target.Cards
                if cards[idx].IsFaceDown then return false end
                local targetSuit = cards[idx].Suit

                local result = true
                for i = idx, #cards - 1 do
                    if not game:can_play(target, i, cards[i + 1], 1)
                        or cards[i + 1].Suit ~= targetSuit
                    then
                        result = false
                        break
                    end
                end
                if result then return true end

                local targetRank = cards[#cards].Rank

                for i = idx, #cards - 1 do
                    if cards[i].Rank ~= targetRank then
                        return false
                    end
                end
                return true
            end
        }
    end,
    SuperMove = function()
        return {
            Hint = "Top card (SuperMove)",
            Func = function(game, target, idx)
                local cards = target.Cards
                if cards[idx].IsFaceDown then return false end

                local freeCell = game.FreeCell
                if #freeCell == 0 then return idx == target.CardCount end

                local movableCards = 0
                for _, fc in ipairs(freeCell) do
                    if fc.IsEmpty then movableCards = movableCards + 1 end
                end
                if idx + movableCards < #cards then
                    return false
                end

                for i = idx, #cards - 1 do
                    if not game:can_play(target, i, cards[i + 1], 1) then return false end
                end
                return true
            end
        }
    end,
}

local base = {
    Ace = function()
        return {
            Hint = "Ace",
            Func = function(_, card, _)
                return card.Rank == "Ace"
            end
        }
    end,
    King = function()
        return {
            Hint = "King",
            Func = function(_, card, _)
                return card.Rank == "King"
            end
        }
    end,
    None = function()
        return {
            Hint = "None",
            Func = function(_, _, _)
                return false
            end
        }
    end,
    Any = function()
        return {
            Hint = "Any",
            Func = function(_, _, _)
                return true
            end
        }
    end,
    AnySingle = function()
        return {
            Hint = "Any; no sequences",
            Func = function(_, _, numCards)
                return numCards == 1
            end
        }
    end,
    FirstFoundation = function(interval)
        interval = interval or 0
        local intervalStr = ""
        if interval ~= 0 then
            intervalStr = tostring(interval)
            if interval > 0 then intervalStr = "+" .. intervalStr end
        end
        return {
            Hint = "Rank" .. intervalStr .. " of first foundation card",
            Func = function(game, card, _)
                local pile = game.Foundation[1]
                if pile.IsEmpty then return false end
                local rank = pile.Cards[1].Rank
                return card.Rank == Sol.get_rank(rank, interval, true)
            end
        }
    end,
    Card = function(suit, rank)
        return {
            Hint = rank .. " of " .. suit,
            Func = function(_, card, _)
                return card.Rank == rank and card.Suit == suit
            end
        }
    end,
    CardColor = function(color, rank)
        return {
            Hint = color .. " " .. rank,
            Func = function(_, card, _)
                return card.Rank == rank and Sol.SuitColors[card.Suit] == color
            end
        }
    end,
    Suits = function(suits)
        return {
            Hint = table.concat(suits, "/"),
            Func = function(_, card, _)
                for _, value in ipairs(suits) do
                    if value == card.Suit then
                        return true
                    end
                end
                return false
            end
        }
    end,
    Ranks = function(ranks)
        return {
            Hint = table.concat(ranks, "/"),
            Func = function(_, card, _)
                for _, value in ipairs(ranks) do
                    if value == card.Rank then
                        return true
                    end
                end
                return false
            end
        }
    end
}

return {
    Build = build,
    Move = move,
    Base = base,

    any_downac_top = { Base = base.Any(), Build = build.DownAlternateColors(), Move = move.Top() },
    any_downac_inseq = { Base = base.Any(), Build = build.DownAlternateColors(), Move = move.InSeq() },
    any_downac_faceup = { Base = base.Any(), Build = build.DownAlternateColors(), Move = move.FaceUp() },
    any_downac_sm = { Base = base.Any(), Build = build.DownAlternateColors(), Move = move.SuperMove() },
    any_downrank_top = { Base = base.Any(), Build = build.DownByRank(), Move = move.Top() },
    any_downrank_inseq = { Base = base.Any(), Build = build.DownByRank(), Move = move.InSeq() },
    any_downsuit_top = { Base = base.Any(), Build = build.DownInSuit(), Move = move.Top() },
    any_downsuit_inseq = { Base = base.Any(), Build = build.DownInSuit(), Move = move.InSeq() },
    any_downsuit_faceup = { Base = base.Any(), Build = build.DownInSuit(), Move = move.FaceUp() },
    any_downabos_top = { Base = base.Any(), Build = build.DownAnyButOwnSuit(), Move = move.Top() },
    any_updownrank_top = { Base = base.Any(), Build = build.UpOrDownByRank(), Move = move.Top() },
    any_updownsuit_top = { Base = base.Any(), Build = build.UpOrDownInSuit(), Move = move.Top() },
    any_updownac_top = { Base = base.Any(), Build = build.UpOrDownAlternateColors(), Move = move.Top() },
    any_none_top = { Base = base.Any(), Build = build.None(), Move = move.Top() },
    ace_upac_top = { Base = base.Ace(), Build = build.UpAlternateColors(), Move = move.Top() },
    ace_uprank_none = { Base = base.Ace(), Build = build.UpByRank(), Move = move.None() },
    ace_upsuit_top = { Base = base.Ace(), Build = build.UpInSuit(), Move = move.Top() },
    ace_upsuit_none = { Base = base.Ace(), Build = build.UpInSuit(), Move = move.None() },
    king_downac_top = { Base = base.King(), Build = build.DownAlternateColors(), Move = move.Top() },
    king_downac_faceup = { Base = base.King(), Build = build.DownAlternateColors(), Move = move.FaceUp() },
    king_downac_inseq = { Base = base.King(), Build = build.DownAlternateColors(), Move = move.InSeq() },
    king_downrank_top = { Base = base.King(), Build = build.DownByRank(), Move = move.Top() },
    king_downrank_inseq = { Base = base.King(), Build = build.DownByRank(), Move = move.InSeq() },
    king_downrank_none = { Base = base.King(), Build = build.DownByRank(), Move = move.None() },
    king_downsuit_top = { Base = base.King(), Build = build.DownInSuit(), Move = move.Top() },
    king_downsuit_none = { Base = base.King(), Build = build.DownInSuit(), Move = move.None() },
    king_downsuit_inseq = { Base = base.King(), Build = build.DownInSuit(), Move = move.InSeq() },
    king_downsuit_faceup = { Base = base.King(), Build = build.DownInSuit(), Move = move.FaceUp() },
    none_downac_top = { Base = base.None(), Build = build.DownAlternateColors(), Move = move.Top() },
    none_downrank_top = { Base = base.None(), Build = build.DownByRank(), Move = move.Top() },
    none_downsuit_top = { Base = base.None(), Build = build.DownInSuit(), Move = move.Top() },
    none_none_top = { Base = base.None(), Build = build.None(), Move = move.Top() },
    none_none_none = { Base = base.None(), Build = build.None(), Move = move.None() },
    ff_upsuit_top_l13 = { Base = base.FirstFoundation(), Build = build.UpInSuit(true), Move = move.Top(), Limit = 13 },
    ff_upsuit_none_l13 = { Base = base.FirstFoundation(), Build = build.UpInSuit(true), Move = move.None(), Limit = 13 },
    spider = { Base = base.Any(), Build = build.DownByRank(), Move = move.InSeqInSuit() },
}
