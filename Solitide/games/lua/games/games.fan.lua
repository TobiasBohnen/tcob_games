-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local fan = {
    Info       = {
        Name      = "Fan",
        Family    = "Fan",
        DeckCount = 1
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 18,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 17 and 3 or 1),
                Layout  = "Row",
                Rule    = Sol.Rules.king_downsuit_top
            }
        end
    },
    on_init    = function(game) Sol.Layout.fan(game, 5) end
}


------

local bear_river      = Sol.copy(fan)
bear_river.Info.Name  = "Bear River"
bear_river.Foundation = {
    Size = 4,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i == 0 and 1 or 0),
            Rule    = Sol.Rules.ff_upsuit_none_l13
        }
    end
}
bear_river.Tableau    = {
    Size = 18,
    Pile = function(i)
        local lastInRow = i % 6 == 5
        return {
            Initial = Sol.Initial.face_up(lastInRow and 2 or 3),
            Layout  = "Row",
            Rule    = { Base = lastInRow and Sol.Rules.Base.Any() or Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownInSuit(true), Move = Sol.Rules.Move.Top(), Limit = 3 }
        }
    end
}
bear_river.on_init    = function(game) Sol.Layout.fan(game, 6) end


------

local box_fan             = Sol.copy(fan)
box_fan.Info.Name         = "Box Fan"
box_fan.Tableau           = {
    Size = 16,
    Pile = {
        Initial = Sol.Initial.face_up(3),
        Layout  = "Row",
        Rule    = Sol.Rules.king_downac_top
    }
}
box_fan.on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation
box_fan.on_init           = function(game) Sol.Layout.fan(game, 4) end


------

local ceiling_fan     = Sol.copy(fan)
ceiling_fan.Info.Name = "Ceiling Fan"
ceiling_fan.Tableau   = {
    Size = 18,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 17 and 3 or 1),
            Layout  = "Row",
            Rule    = Sol.Rules.king_downac_top
        }
    end
}


------

local clover_leaf = {
    Info              = {
        Name      = "Clover Leaf",
        Family    = "Fan",
        DeckCount = 1
    },
    Foundation        = {
        Size = 4,
        Pile = function(i)
            if i < 2 then
                return { Rule = { Base = Sol.Rules.Base.CardColor("Black", "Ace"), Build = Sol.Rules.Build.UpInSuit(), Move = Sol.Rules.Move.Top() } }
            else
                return { Rule = { Base = Sol.Rules.Base.CardColor("Red", "King"), Build = Sol.Rules.Build.DownInSuit(), Move = Sol.Rules.Move.Top() } }
            end
        end
    },
    Tableau           = {
        Size = 16,
        Pile = {
            Initial = Sol.Initial.face_up(3),
            Layout  = "Row",
            Rule    = { Base = Sol.Rules.Base.Ranks({ "Ace", "King" }), Build = Sol.Rules.Build.UpOrDownInSuit(), Move = Sol.Rules.Move.Top() }
        }
    },
    on_before_shuffle = function(game, card)
        if card.Rank == "Ace" and card.Color == "Black" then
            return game.PlaceTop(card, game.Foundation, 1, 2, true)
        end
        if card.Rank == "King" and card.Color == "Red" then
            return game.PlaceTop(card, game.Foundation, 3, 2, true)
        end

        return false
    end,
    on_init           = function(game) Sol.Layout.fan(game, 4) end
}


------

local alexander_the_great     = Sol.copy(clover_leaf)
alexander_the_great.Info.Name = "Alexander the Great"
alexander_the_great.Tableau   = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.face_up(4),
        Layout  = "Row",
        Rule    = { Base = Sol.Rules.Base.Ranks({ "Ace", "King" }), Build = Sol.Rules.Build.UpOrDownInSuit(), Move = Sol.Rules.Move.Top() }
    }
}
alexander_the_great.on_init   = function(game) Sol.Layout.fan(game, 3) end


------

local house_in_the_wood           = Sol.copy(fan)
house_in_the_wood.Info.Name       = "House in the Wood"
house_in_the_wood.Info.DeckCount  = 2
house_in_the_wood.Foundation.Size = 8
house_in_the_wood.Tableau         = {
    Size = 35,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 34 and 3 or 2),
            Layout  = "Row",
            Rule    = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownInSuit(), Move = Sol.Rules.Move.Top() }
        }
    end
}
house_in_the_wood.on_init         = function(game) Sol.Layout.fan(game, 6) end


------

local house_on_the_hill           = Sol.copy(house_in_the_wood)
house_on_the_hill.Info.Name       = "House on the Hill"
house_on_the_hill.Foundation.Pile = function(i)
    if (i < 4) then
        return {
            Rule = Sol.Rules.ace_upsuit_top
        }
    else
        return {
            Rule = Sol.Rules.king_downsuit_top
        }
    end
end


------

local quads      = Sol.copy(fan)
quads.Info.Name  = "Quads"
quads.Tableau    = {
    Size = 13,
    Pile = {
        Initial = Sol.Initial.face_up(4),
        Layout  = "Row",
        Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.Top(), Limit = 4 }
    }
}
quads.on_shuffle = function(game, card, pile)
    if pile.Type == "Tableau" and card.Rank == "Ace" then
        return game.PlaceTop(card, game.Foundation, true)
    end

    return false
end


------

local quads_plus             = Sol.copy(fan)
quads_plus.Info.Name         = "Quads+"
quads_plus.Tableau           = {
    Size = 13,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 12 and 4 or 0),
            Layout  = "Row",
            Rule    = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.Top(), Limit = 4 }
        }
    end
}
quads_plus.on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation


------

local lucky_piles_pos <const> = {
    { 0, 1 }, { 2, 1 }, { 4, 1 }, { 6, 1 }, { 8, 1 },
    --[[ --]] { 2, 2 }, { 4, 2 }, { 6, 2 }, --[[ --]]
    { 0, 3 }, { 2, 3 }, { 4, 3 }, { 6, 3 }, { 8, 3 }
}

local lucky_piles             = {
    Info       = {
        Name      = "Lucky Piles",
        Family    = "Fan",
        DeckCount = 1
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = (i + 0.5) * 2, y = 0 },
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 13,
        Pile = function(i)
            return {
                Position = { x = lucky_piles_pos[i + 1][1], y = lucky_piles_pos[i + 1][2] },
                Initial  = Sol.Initial.face_up(4),
                Layout   = "Row",
                Rule     = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.UpOrDownInSuit(), Move = Sol.Rules.Move.Top() }
            }
        end
    }
}


------

local scotch_patience      = Sol.copy(fan)
scotch_patience.Info.Name  = "Scotch Patience"
scotch_patience.Foundation = {
    Size = 4,
    Pile = { Rule = Sol.Rules.ace_upac_top }
}
scotch_patience.Tableau    = {
    Size = 18,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 17 and 3 or 1),
            Layout  = "Row",
            Rule    = Sol.Rules.none_downrank_top
        }
    end
}


------

local shamrocks     = Sol.copy(fan)
shamrocks.Info.Name = "Shamrocks"
shamrocks.Tableau   = {
    Size = 18,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 17 and 3 or 1),
            Layout  = "Row",
            Rule    = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownByRank(), Move = Sol.Rules.Move.Top(), Limit = 3 }
        }
    end
}


------

local shamrocks_2            = Sol.copy(fan)
shamrocks_2.Info.Name        = "Shamrocks II"
shamrocks_2.Tableau          = {
    Size = 18,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 17 and 3 or 1),
            Layout  = "Row",
            Rule    = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownByRank(), Move = Sol.Rules.Move.Top(), Limit = 3 }
        }
    end
}
shamrocks_2.on_after_shuffle = Sol.Ops.Shuffle.kings_to_bottom


------

local troika      = Sol.copy(fan)
troika.Info.Name  = "Troika"
troika.Tableau    = {
    Size = 18,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 17 and 3 or 1),
            Layout  = "Row",
            Rule    = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.Top(), Limit = 3 }
        }
    end
}
troika.on_shuffle = function(game, card, pile)
    if pile.Type == "Tableau" and card.Rank == "Ace" then
        return game.PlaceTop(card, game.Foundation, true)
    end

    return false
end


------

local club = {
    Info       = {
        Name      = "Club",
        Family    = "Fan",
        DeckCount = 2
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(71)
    },
    Waste      = { Position = { x = 1, y = 0 } },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 11,
        Pile = function(i)
            local pos = {}
            if i < 3 then
                pos = { x = i * 2 + 3, y = 1 }
            else
                pos = { x = (i % 4) * 2 + 2, y = (i + 1) // 4 + 1 }
            end
            return {
                Position = pos,
                Initial  = { false, true, true },
                Layout   = "Row",
                Rule     = Sol.Rules.any_downac_top
            }
        end
    },
    can_play   = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == "Tableau" then
            -- first tableau row can't be played to row 2 and 3 tableau piles
            local srcPile = game:find_pile(card)
            if srcPile.Type == "Tableau" and srcPile.Index < 4 and targetPile.Index >= 4 then
                return false
            end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    deal       = Sol.Ops.Deal.stock_to_waste
}


------

local crescent = {
    Info              = {
        Name      = "Crescent",
        Family    = "Fan",
        DeckCount = 2,
        Redeals   = 3
    },
    Stock             = { Position = { x = 0, y = 0 } },
    Foundation        = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Rule     = i < 4 and Sol.Rules.ace_upsuit_top or Sol.Rules.king_downsuit_top
            }
        end
    },
    Tableau           = {
        Size = 16,
        Pile = function(i)
            return {
                Position = { x = (i % 4) * 3, y = i // 4 + 1 },
                Initial  = Sol.Initial.face_up(6),
                Layout   = "Row",
                Rule     = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownInSuit(true), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    on_before_shuffle = function(game, card)
        if card.Rank == "Ace" then
            return game.PlaceTop(card, game.Foundation, 1, 4, true)
        end
        if card.Rank == "King" then
            return game.PlaceTop(card, game.Foundation, 5, 4, true)
        end

        return false
    end,
    redeal            = function(game)
        for _, tableau in ipairs(game.Tableau) do
            if tableau.CardCount > 1 then
                tableau:shift_card(1, tableau.CardCount)
            end
        end
        return true
    end
}


------

local fascination_fan = {
    Info = {
        Name      = "Fascination Fan",
        Family    = "Fan",
        DeckCount = 1,
        Redeals   = 6
    },
    Stock = { Position = { x = 0, y = 0 } },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau = {
        Size = 18,
        Pile = function(i)
            return {
                Position = { x = (i % 5) * 2, y = i // 5 + 1 },
                Initial  = Sol.Initial.top_face_up(i == 17 and 1 or 3),
                Layout   = "Row",
                Rule     = Sol.Rules.none_downac_top
            }
        end
    },
    redeal = function(game)
        local tableau = game.Tableau
        local cards = Sol.shuffle_piles(game, { tableau })
        if #cards == 0 then return false end

        for _, tab in ipairs(tableau) do
            for _ = 1, 3 do
                if #cards == 0 then break end
                game.PlaceTop(table.remove(cards), tab, false)
            end
            tab:flip_cards({ false, false, true })
        end

        return true
    end
}


------

local forest_glade = {
    Info = {
        Name      = "Forest Glade",
        Family    = "Fan",
        DeckCount = 2,
        Redeals   = 2
    },
    Stock = {
        Position = { x = 0, y = 5 },
        Initial = Sol.Initial.face_down(56)
    },
    Foundation = {
        Size = 16,
        Pile = function(i)
            if i < 8 then
                return {
                    Position = { x = i % 2, y = i // 2 },
                    Rule     = { Base = Sol.Rules.Base.Ace(), Build = Sol.Rules.Build.UpInSuit(false, 2), Move = Sol.Rules.Move.Top() }
                }
            else
                return {
                    Position = { x = (i - 8) % 2 + 8, y = (i - 8) // 2 },
                    Rule     = { Base = Sol.Rules.Base.Ranks({ "Two" }), Build = Sol.Rules.Build.UpInSuit(false, 2), Move = Sol.Rules.Move.Top() }
                }
            end
        end
    },
    Tableau = {
        Size = 16,
        Pile = function(i)
            local pos = {}
            if i >= 2 and i <= 13 then
                pos.x = ((i + 1) % 3) * 2 + 2
                pos.y = (i + 1) // 3
            elseif i < 2 then
                pos.x = i * 2 + 3
                pos.y = 0
            else
                pos.x = (i - 14) * 2 + 3
                pos.y = 5
            end
            return {
                Position = pos,
                Initial  = Sol.Initial.face_up(3),
                Layout   = "Row",
                Rule     = Sol.Rules.king_downsuit_top
            }
        end
    },
    deal = function(game)
        local tableau = game.Tableau
        local stock = game.Stock[1]

        -- fill empty tableau piles from stock
        for _, tab in ipairs(tableau) do
            if stock.IsEmpty then break end
            if tab.IsEmpty then
                stock:move_cards(tab, stock.CardCount - 2, 3, false)
                tab:flip_up_cards()
                return true
            end
        end

        return false
    end,
    redeal = function(game)
        local tableau = game.Tableau
        local stock = game.Stock[1]

        -- don't redeal if tableau has a empty pile
        for _, tab in ipairs(tableau) do
            if tab.IsEmpty then
                return false
            end
        end

        -- shuffle
        local cards = Sol.shuffle_piles(game, { tableau, game.Stock })
        if #cards == 0 then return false end

        -- redeal
        for _, tab in ipairs(tableau) do
            for _ = 1, 3 do
                if #cards == 0 then break end
                game.PlaceTop(table.remove(cards), tab, false)
            end
            tab:flip_up_cards()
        end

        while #cards ~= 0 do
            game.PlaceTop(table.remove(cards), stock, false)
        end
        stock:flip_down_cards()

        return true
    end
}


------

local free_fan = {
    Info       = {
        Name      = "Free Fan",
        Family    = "Fan",
        DeckCount = 1
    },
    FreeCell   = {
        Size = 2,
        Pile = function(i)
            return {
                Position = { x = ((i + 18) % 5) * 2, y = (i + 18) // 5 },
                Rule     = Sol.Rules.any_none_top
            }
        end
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = 10, y = i },
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 18,
        Pile = function(i)
            return {
                Position = { x = (i % 5) * 2, y = i // 5 },
                Initial  = Sol.Initial.face_up(i < 17 and 3 or 1),
                Layout   = "Row",
                Rule     = Sol.Rules.king_downsuit_top
            }
        end
    }
}


------

local intelligence = {
    Info             = {
        Name      = "Intelligence",
        Family    = "Fan",
        DeckCount = 2,
        Redeals   = 2
    },
    Stock            = {
        Initial = Sol.Initial.face_down(50)
    },
    Foundation       = {
        Size = 8,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau          = {
        Size = 18,
        Pile = {
            Initial = Sol.Initial.face_up(3),
            Layout  = "Row",
            Rule    = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownInSuit(), Move = Sol.Rules.Move.Top() }
        }
    },
    on_shuffle       = function(game, card, pile)
        if pile.Type == "Tableau" and card.Rank == "Ace" then
            return game.PlaceTop(card, game.Foundation, true)
        end

        return false
    end,
    on_after_shuffle = function(game)
        -- refill Tableau from Stock back to three cards
        local stock = game.Stock[1]
        local stockCards = stock.Cards
        local idx = #stockCards
        for _, tab in ipairs(game.Tableau) do
            while tab.CardCount < 3 do
                if stockCards[idx].Rank == "Ace" then
                    for _, fou in ipairs(game.Foundation) do
                        if fou.IsEmpty then
                            stock:move_cards(fou, idx, 1, false)
                            fou:flip_up_cards()
                            break
                        end
                    end
                else
                    stock:move_cards(tab, idx, 1, false)
                end

                idx = idx - 1
            end
            tab:flip_up_cards()
        end

        return false
    end,
    redeal           = function(game)
        local tableau = game.Tableau
        local stock = game.Stock[1]
        local foundation = game.Foundation

        -- shuffle
        local cards = Sol.shuffle_piles(game, { tableau, game.Stock })
        if #cards == 0 then return false end

        -- redeal
        for _, tab in ipairs(tableau) do
            local i = 1
            while i <= 3 do
                if #cards == 0 then break end

                local card = table.remove(cards)
                if card.Rank == "Ace" then
                    game.PlaceTop(card, foundation, true)
                else
                    game.PlaceTop(card, tab, false)
                    i = i + 1
                end
            end
            tab:flip_up_cards()
        end

        while #cards ~= 0 do
            game.PlaceTop(table.remove(cards), stock, false)
        end
        stock:flip_down_cards()

        return true
    end,
    on_end_turn      = function(game)
        local tableau = game.Tableau
        local stock = game.Stock[1]

        -- fill empty tableau piles from stock
        for _, tab in ipairs(tableau) do
            if stock.IsEmpty then break end
            if tab.IsEmpty then
                stock:move_cards(tab, stock.CardCount - 2, 3, false)
                tab:flip_up_cards()
                return true
            end
        end
    end,
    on_init          = function(game) Sol.Layout.fan(game, 4) end,
}


------

local intelligence_plus         = Sol.copy(intelligence)
intelligence_plus.Info.Name     = "Intelligence +"
intelligence_plus.Stock.Initial = Sol.Initial.face_down(47)
intelligence_plus.Tableau.Size  = 19


------

local la_belle_lucie = {
    Info       = {
        Name      = "La Belle Lucie",
        Family    = "Fan",
        DeckCount = 1,
        Redeals   = 2
    },
    Stock      = {},
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 18,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i == 17 and 1 or 3),
                Layout  = "Row",
                Rule    = Sol.Rules.none_downsuit_top
            }
        end
    },
    redeal     = function(game)
        local tableau = game.Tableau

        -- shuffle
        local cards = Sol.shuffle_piles(game, { tableau, game.Stock })
        if #cards == 0 then return false end

        -- redeal
        for _, tab in ipairs(tableau) do
            local i = 1
            while i <= 3 do
                if #cards == 0 then break end

                local card = table.remove(cards)
                game.PlaceTop(card, tab, false)
                i = i + 1
            end
            tab:flip_up_cards()
        end

        return true
    end,
    on_init    = function(game) Sol.Layout.fan(game, 4) end,
}


------

local roaming_proils = {
    Info = {
        Name      = "Roaming Proils",
        Family    = "Fan",
        DeckCount = 1
    },
    FreeCell = {
        Initial = Sol.Initial.face_up(1),
        Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.None(), Move = Sol.Rules.Move.Top() }
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau = {
        Size = 17,
        Pile = {
            Initial = Sol.Initial.top_face_up(3),
            Layout  = "Row",
            Rule    = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.Top() }
        }
    },
    can_play = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == "Tableau" and targetCardIndex >= 3 then
            -- only three consecutive cards of the same rank
            local cards = targetPile.Cards
            if cards[targetCardIndex].IsFaceUp and cards[targetCardIndex].Rank == card.Rank
                and cards[targetCardIndex - 1].IsFaceUp and cards[targetCardIndex - 1].Rank == card.Rank
                and cards[targetCardIndex - 2].IsFaceUp and cards[targetCardIndex - 2].Rank == card.Rank
            then
                return false
            end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    on_init = function(game) Sol.Layout.fan(game, 5) end
}


------

local open_proils = Sol.copy(roaming_proils)
open_proils.Info.Name = "Open Proils"
open_proils.Tableau.Pile.Initial = Sol.Initial.face_up(3)


------

------------------------

Sol.register_game(fan)
Sol.register_game(alexander_the_great)
Sol.register_game(bear_river)
Sol.register_game(box_fan)
Sol.register_game(ceiling_fan)
Sol.register_game(clover_leaf)
Sol.register_game(club)
Sol.register_game(crescent)
Sol.register_game(fascination_fan)
Sol.register_game(forest_glade)
Sol.register_game(free_fan)
Sol.register_game(house_in_the_wood)
Sol.register_game(house_on_the_hill)
Sol.register_game(intelligence)
Sol.register_game(intelligence_plus)
Sol.register_game(la_belle_lucie)
Sol.register_game(quads)
Sol.register_game(quads_plus)
Sol.register_game(lucky_piles)
Sol.register_game(open_proils)
Sol.register_game(roaming_proils)
Sol.register_game(scotch_patience)
Sol.register_game(shamrocks)
Sol.register_game(shamrocks_2)
Sol.register_game(troika)
