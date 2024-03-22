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
    end
}

local build = {
    None                    = function(_, _)
        return {
            Hint = "No building.",
            Build = function(_, _)
                return false
            end
        }
    end,
    Any                     = function(_, _)
        return {
            Hint = "Any card.",
            Build = function(_, _)
                return true
            end
        }
    end,

    InRank                  = function(_, _)
        return {
            Hint = "Build by same rank.",
            Build = function(target, drop)
                return rules.in_rank(target, drop)
            end
        }
    end,

    InRankOrDownByRank      = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Build down by rank or by same rank.",
            Build = function(target, drop)
                return rules.in_rank(target, drop) or rules.build_down(target, drop, wrap, interval)
            end
        }
    end,

    RankPack                = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Build by same rank, then build up by rank.",
            Build = function(target, drop)
                if target.CardCount % 4 == 0 then
                    return rules.build_up(target, drop, wrap, interval)
                end
                return rules.in_rank(target, drop)
            end
        }
    end,

    UpOrDownByRank          = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Build up or down by rank.",
            Build = function(target, drop)
                return rules.build_up(target, drop, wrap, interval) or rules.build_down(target, drop, wrap, interval)
            end
        }
    end,
    DownByRank              = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Build down by rank.",
            Build = function(target, drop)
                return rules.build_down(target, drop, wrap, interval)
            end
        }
    end,
    UpByRank                = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Build up by rank.",
            Build = function(target, drop)
                return rules.build_up(target, drop, wrap, interval)
            end
        }
    end,

    UpOrDownAnyButOwnSuit   = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Build up or down by any suit but own.",
            Build = function(target, drop)
                return not rules.in_suit(target, drop) and (rules.build_up(target, drop, wrap, interval) or rules.build_down(target, drop, wrap, interval))
            end
        }
    end,
    DownAnyButOwnSuit       = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Build down by any suit but own.",
            Build = function(target, drop)
                return not rules.in_suit(target, drop) and rules.build_down(target, drop, wrap, interval)
            end
        }
    end,
    UpAnyButOwnSuit         = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Build up by any suit but own.",
            Build = function(target, drop)
                return not rules.in_suit(target, drop) and rules.build_up(target, drop, wrap, interval)
            end
        }
    end,

    UpOrDownInSuit          = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Build up or down by suit.",
            Build = function(target, drop)
                return rules.in_suit(target, drop) and (rules.build_up(target, drop, wrap, interval) or rules.build_down(target, drop, wrap, interval))
            end
        }
    end,
    DownInSuit              = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Build down by suit.",
            Build = function(target, drop)
                return rules.in_suit(target, drop) and rules.build_down(target, drop, wrap, interval)
            end
        }
    end,
    UpInSuit                = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Build up by suit.",
            Build = function(target, drop)
                return rules.in_suit(target, drop) and rules.build_up(target, drop, wrap, interval)
            end
        }
    end,

    UpOrDownInColor         = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Build up or down by color.",
            Build = function(target, drop)
                return rules.in_color(target, drop) and (rules.build_up(target, drop, wrap, interval) or rules.build_down(target, drop, wrap, interval))
            end
        }
    end,
    DownInColor             = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Build down by color.",
            Build = function(target, drop)
                return rules.in_color(target, drop) and rules.build_down(target, drop, wrap, interval)
            end
        }
    end,
    UpInColor               = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Build up by color.",
            Build = function(target, drop)
                return rules.in_color(target, drop) and rules.build_up(target, drop, wrap, interval)
            end
        }
    end,

    UpOrDownAlternateColors = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Build up or down by alternate color.",
            Build = function(target, drop)
                return rules.alternate_color(target, drop) and (rules.build_up(target, drop, wrap, interval) or rules.build_down(target, drop, wrap, interval))
            end
        }
    end,
    DownAlternateColors     = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Build down by alternate color.",
            Build = function(target, drop)
                return rules.alternate_color(target, drop) and rules.build_down(target, drop, wrap, interval)
            end
        }
    end,
    UpAlternateColors       = function(wrap, interval)
        interval = interval or 1
        wrap = wrap or false
        return {
            Hint = "Build up by alternate color.",
            Build = function(target, drop)
                return rules.alternate_color(target, drop) and rules.build_up(target, drop, wrap, interval)
            end
        }
    end,
}

local move = {
    None = function()
        return {
            IsPlayable = false,
            IsSequence = false
        }
    end,
    Top = function()
        return {
            IsSequence = false,
            Move = function(_, target, idx)
                return idx == target.CardCount
            end
        }
    end,
    TopOrPile = function()
        return {
            Move = function(_, target, idx)
                return idx == target.CardCount or idx == 1
            end
        }
    end,
    FaceUp = function()
        return {
            Move = function(_, target, idx)
                return target.Cards[idx].IsFaceUp
            end
        }
    end,
    InSeq = function()
        return {
            Move = function(game, target, idx)
                local cards = target.Cards
                if cards[idx].IsFaceDown then return false end
                for i = idx, #cards - 1, 1 do
                    if not game:can_play(target, i, cards[i + 1], 1) then return false end
                end
                return true
            end
        }
    end,
    InSeqInSuit = function()
        return {
            Move = function(game, target, idx)
                local cards = target.Cards
                if cards[idx].IsFaceDown then return false end

                local targetSuit = cards[idx].Suit
                for i = idx, #cards - 1, 1 do
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
    InSeqInSuitOrSameRank = function()
        return {
            Move = function(game, target, idx)
                local cards = target.Cards
                if cards[idx].IsFaceDown then return false end
                local targetSuit = cards[idx].Suit

                local result = true
                for i = idx, #cards - 1, 1 do
                    if not game:can_play(target, i, cards[i + 1], 1)
                        or cards[i + 1].Suit ~= targetSuit
                    then
                        result = false
                        break
                    end
                end
                if result then return true end

                local targetRank = cards[#cards].Rank

                for i = idx, #cards - 1, 1 do
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
            Move = function(game, target, idx)
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

                for i = idx, #cards - 1, 1 do
                    if not game:can_play(target, i, cards[i + 1], 1) then return false end
                end
                return true
            end
        }
    end,
}

local base = {
    --direct
    Ace = function(_, card, _)
        return card.Rank == "Ace"
    end,
    King = function(_, card, _)
        return card.Rank == "King"
    end,
    None = function(_, _, _)
        return false
    end,
    Any = function(_, _, _)
        return true
    end,
    AnySingle = function(_, _, numCards)
        return numCards == 1
    end,
    --indirect
    FirstFoundation = function(game, card, interval)
        local pile = game.Foundation[1]
        if pile.IsEmpty then
            return false
        end
        local rank = pile.Cards[1].Rank
        return card.Rank == Sol.get_rank(rank, interval or 0, true)
    end,
    Card = function(card, suit, rank)
        return card.Rank == rank and card.Suit == suit
    end,
    CardColor = function(card, color, rank)
        return card.Rank == rank and Sol.SuitColors[card.Suit] == color
    end,
    Suits = function(card, suits)
        for _, value in ipairs(suits) do
            if value == card.Suit then
                return true
            end
        end
        return false
    end,
    Ranks = function(card, ranks)
        for _, value in ipairs(ranks) do
            if value == card.Rank then
                return true
            end
        end
        return false
    end
}

return {
    Build = build,
    Move = move,
    Base = base,

    any_downac_top = { Base = base.Any, Build = build.DownAlternateColors(), Move = move.Top() },
    any_downac_inseq = { Base = base.Any, Build = build.DownAlternateColors(), Move = move.InSeq() },
    any_downac_faceup = { Base = base.Any, Build = build.DownAlternateColors(), Move = move.FaceUp() },
    any_downac_sm = { Base = base.Any, Build = build.DownAlternateColors(), Move = move.SuperMove() },
    any_downrank_top = { Base = base.Any, Build = build.DownByRank(), Move = move.Top() },
    any_downsuit_top = { Base = base.Any, Build = build.DownInSuit(), Move = move.Top() },
    any_downsuit_inseq = { Base = base.Any, Build = build.DownInSuit(), Move = move.InSeq() },
    any_downsuit_faceup = { Base = base.Any, Build = build.DownInSuit(), Move = move.FaceUp() },
    any_updownsuit_top = { Base = base.Any, Build = build.UpOrDownInSuit(), Move = move.Top() },
    any_updownac_top = { Base = base.Any, Build = build.UpOrDownAlternateColors(), Move = move.Top() },
    any_none_top = { Base = base.Any, Build = build.None(), Move = move.Top() },
    ace_upac_top = { Base = base.Ace, Build = build.UpAlternateColors(), Move = move.Top() },
    ace_upsuit_top = { Base = base.Ace, Build = build.UpInSuit(), Move = move.Top() },
    ace_upsuit_none = { Base = base.Ace, Build = build.UpInSuit(), Move = move.None() },
    king_downac_top = { Base = base.King, Build = build.DownAlternateColors(), Move = move.Top() },
    king_downac_faceup = { Base = base.King, Build = build.DownAlternateColors(), Move = move.FaceUp() },
    king_downac_inseq = { Base = base.King, Build = build.DownAlternateColors(), Move = move.InSeq() },
    king_downrank_top = { Base = base.King, Build = build.DownByRank(), Move = move.Top() },
    king_downsuit_top = { Base = base.King, Build = build.DownInSuit(), Move = move.Top() },
    king_downsuit_inseq = { Base = base.King, Build = build.DownInSuit(), Move = move.InSeq() },
    king_downsuit_faceup = { Base = base.King, Build = build.DownInSuit(), Move = move.FaceUp() },
    none_downrank_top = { Base = base.None, Build = build.DownByRank(), Move = move.Top() },
    none_downsuit_top = { Base = base.None, Build = build.DownInSuit(), Move = move.Top() },
    none_none_top = { Base = base.None, Build = build.None(), Move = move.Top() },
    none_none_none = { Base = base.None, Build = build.None(), Move = move.None() },
    ff_upsuit_top = { Base = function(game, card, _) return base.FirstFoundation(game, card) end, Build = build.UpInSuit(true), Move = move.Top() },
    ff_upsuit_none = { Base = function(game, card, _) return base.FirstFoundation(game, card) end, Build = build.UpInSuit(true), Move = move.None() }
}
