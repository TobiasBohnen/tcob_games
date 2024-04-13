-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local fan                 = {
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
                Layout = "Row",
                Rule = Sol.Rules.king_downsuit_top
            }
        end
    },
    on_init    = function(game) Sol.Layout.fan(game, 5) end
}

------

local bear_river          = Sol.copy(fan)
bear_river.Info.Name      = "Bear River"
bear_river.Foundation     = {
    Size = 4,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i == 0 and 1 or 0),
            Rule = Sol.Rules.ff_upsuit_none_l13
        }
    end
}
bear_river.Tableau        = {
    Size = 18,
    Pile = function(i)
        local lastInRow = i % 6 == 5
        return {
            Initial = Sol.Initial.face_up(lastInRow and 2 or 3),
            Layout = "Row",
            Rule = { Base = lastInRow and Sol.Rules.Base.Any() or Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownInSuit(true), Move = Sol.Rules.Move.Top(), Limit = 3 }
        }
    end
}
bear_river.on_init        = function(game) Sol.Layout.fan(game, 6) end

------

local box_fan             = Sol.copy(fan)
box_fan.Info.Name         = "Box Fan"
box_fan.Tableau           = {
    Size = 16,
    Pile = {
        Initial = Sol.Initial.face_up(3),
        Layout = "Row",
        Rule = Sol.Rules.king_downac_top
    }
}
box_fan.on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation
box_fan.on_init           = function(game) Sol.Layout.fan(game, 4) end

------

local ceiling_fan         = Sol.copy(fan)
ceiling_fan.Info.Name     = "Ceiling Fan"
ceiling_fan.Tableau       = {
    Size = 18,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = Sol.Rules.king_downac_top
        }
    end
}

------

local clover_leaf         = {
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
            Layout = "Row",
            Rule = { Base = Sol.Rules.Base.Ranks({ "Ace", "King" }), Build = Sol.Rules.Build.UpOrDownInSuit(), Move = Sol.Rules.Move.Top() }
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

local alexander_the_great         = Sol.copy(clover_leaf)
alexander_the_great.Info.Name     = "Alexander the Great"
alexander_the_great.Tableau       = {
    Size = 12,
    Pile = {
        Initial = Sol.Initial.face_up(4),
        Layout = "Row",
        Rule = { Base = Sol.Rules.Base.Ranks({ "Ace", "King" }), Build = Sol.Rules.Build.UpOrDownInSuit(), Move = Sol.Rules.Move.Top() }
    }
}
alexander_the_great.on_init       = function(game) Sol.Layout.fan(game, 3) end

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
            Layout = "Row",
            Rule = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownInSuit(), Move = Sol.Rules.Move.Top() }
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

local quads                       = Sol.copy(fan)
quads.Info.Name                   = "Quads"
quads.Tableau                     = {
    Size = 13,
    Pile = {
        Initial = Sol.Initial.face_up(4),
        Layout = "Row",
        Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.Top(), Limit = 4 }
    }
}
quads.on_shuffle                  = function(game, card, pile)
    if pile.Type == "Tableau" and card.Rank == "Ace" then
        return game.PlaceTop(card, game.Foundation, true)
    end

    return false
end

------

local quads_plus                  = Sol.copy(fan)
quads_plus.Info.Name              = "Quads+"
quads_plus.Tableau                = {
    Size = 13,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 12 and 4 or 0),
            Layout = "Row",
            Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.Top(), Limit = 4 }
        }
    end
}
quads_plus.on_before_shuffle      = Sol.Ops.Shuffle.ace_to_foundation

------

local lucky_piles_pos             = {
    { 0, 1 }, { 2, 1 }, { 4, 1 }, { 6, 1 }, { 8, 1 },
    { 2, 2 }, { 4, 2 }, { 6, 2 },
    { 0, 3 }, { 2, 3 }, { 4, 3 }, { 6, 3 }, { 8, 3 } }

local lucky_piles                 = {
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
                Rule = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 13,
        Pile = function(i)
            return {
                Position = { x = lucky_piles_pos[i + 1][1], y = lucky_piles_pos[i + 1][2] },
                Initial = Sol.Initial.face_up(4),
                Layout = "Row",
                Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.UpOrDownInSuit(), Move = Sol.Rules.Move.Top() }
            }
        end
    }
}

------

local scotch_patience             = Sol.copy(fan)
scotch_patience.Info.Name         = "Scotch Patience"
scotch_patience.Foundation        = {
    Size = 4,
    Pile = { Rule = Sol.Rules.ace_upac_top }
}
scotch_patience.Tableau           = {
    Size = 18,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = Sol.Rules.none_downrank_top
        }
    end
}

------

local shamrocks                   = Sol.copy(fan)
shamrocks.Info.Name               = "Shamrocks"
shamrocks.Tableau                 = {
    Size = 18,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownByRank(), Move = Sol.Rules.Move.Top(), Limit = 3 }
        }
    end
}

------

local shamrocks_2                 = Sol.copy(fan)
shamrocks_2.Info.Name             = "Shamrocks II"
shamrocks_2.Tableau               = {
    Size = 18,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownByRank(), Move = Sol.Rules.Move.Top(), Limit = 3 }
        }
    end
}
shamrocks_2.on_after_shuffle      = Sol.Ops.Shuffle.kings_to_bottom

------

local troika                      = Sol.copy(fan)
troika.Info.Name                  = "Troika"
troika.Tableau                    = {
    Size = 18,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 17 and 3 or 1),
            Layout = "Row",
            Rule = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.Top(), Limit = 3 }
        }
    end
}
troika.on_shuffle                 = function(game, card, pile)
    if pile.Type == "Tableau" and card.Rank == "Ace" then
        return game.PlaceTop(card, game.Foundation, true)
    end

    return false
end

------

local belvedere                   = {
    Info              = {
        Name      = "Belvedere",
        Family    = "Fan",
        DeckCount = 1
    },
    Stock             = {
        Initial = Sol.Initial.face_down(27)
    },
    Waste             = {
        Size = 3,
        Pile = {
            Layout = "Squared",
            Rule = Sol.Rules.none_none_top
        }
    },
    Foundation        = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_uprank_none }
    },
    Tableau           = {
        Size = 8,
        Pile = {
            Initial = Sol.Initial.face_up(3),
            Layout = "Row",
            Rule = Sol.Rules.none_downrank_top
        }
    },
    on_before_shuffle = function(game, card)
        if card.Rank == "Ace" then
            return game.PlaceTop(card, game.Foundation[1], true)
        end

        return false
    end,
    on_init           = function(game) Sol.Layout.fan(game, 4) end,
    do_deal           = function(game) return Sol.Ops.Deal.to_group(game.Stock[1], game.Waste, false) end
}

------

local bristol                     = Sol.copy(belvedere)
bristol.Info.Name                 = "Bristol"
bristol.Stock.Initial             = Sol.Initial.face_down(28)
bristol.on_before_shuffle         = nil
bristol.on_after_shuffle          = Sol.Ops.Shuffle.kings_to_bottom

------

local dover                       = Sol.copy(bristol)
dover.Info.Name                   = "Dover"
dover.Info.DeckCount              = 2
dover.Stock.Initial               = Sol.Initial.face_down(80)

------

local club                        = {
    Info           = {
        Name      = "Club",
        Family    = "Fan",
        DeckCount = 2
    },
    Stock          = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(71)
    },
    Waste          = { Position = { x = 1, y = 0 } },
    Foundation     = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 3, y = 0 },
                Rule = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau        = {
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
                Initial = { false, true, true },
                Layout = "Row",
                Rule = Sol.Rules.any_downac_top
            }
        end
    },
    check_playable = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == "Tableau" then
            -- first tableau row can't be played to row 2 and 3 tableau piles
            local srcPile = game:find_pile(card)
            if srcPile.Type == "Tableau" and game:get_pile_index(srcPile) < 4 and game:get_pile_index(targetPile) >= 4 then
                return false
            end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    do_deal        = Sol.Ops.Deal.stock_to_waste
}

------

local crescent                    = {
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
                Rule = i < 4 and Sol.Rules.ace_upsuit_top or Sol.Rules.king_downsuit_top
            }
        end
    },
    Tableau           = {
        Size = 16,
        Pile = function(i)
            return {
                Position = { x = (i % 4) * 3, y = i // 4 + 1 },
                Initial = Sol.Initial.face_up(6),
                Layout = "Row",
                Rule = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownInSuit(true), Move = Sol.Rules.Move.Top() }
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
    do_redeal         = function(game)
        for _, tableau in ipairs(game.Tableau) do
            if tableau.CardCount > 1 then
                tableau:move_card(1, tableau.CardCount)
            end
        end
        return true
    end
}

------

local fascination_fan             = {
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
                Rule = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau = {
        Size = 18,
        Pile = function(i)
            return {
                Position = { x = (i % 5) * 2, y = i // 5 + 1 },
                Initial = Sol.Initial.top_face_up(i == 17 and 1 or 3),
                Layout = "Row",
                Rule = Sol.Rules.none_downac_top
            }
        end
    },
    do_redeal = function(game)
        local cards = Sol.shuffle_tableau(game)
        if #cards == 0 then return false end

        for _, tableau in ipairs(game.Tableau) do
            for _ = 1, 3 do
                if #cards == 0 then break end
                local card = table.remove(cards)
                game.PlaceTop(card, tableau, false)
            end
            tableau:flip_cards({ false, false, true })
        end

        return true
    end
}

------

local free_fan                    = {
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
                Rule = Sol.Rules.any_none_top
            }
        end
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = 10, y = i },
                Rule = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau    = {
        Size = 18,
        Pile = function(i)
            return {
                Position = { x = (i % 5) * 2, y = i // 5 },
                Initial = Sol.Initial.face_up(i < 17 and 3 or 1),
                Layout = "Row",
                Rule = Sol.Rules.king_downsuit_top
            }
        end
    }
}

------

local new_york                    = {
    Info           = {
        Name      = "New York",
        Family    = "Fan",
        DeckCount = 2
    },
    Reserve        = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.top_face_up(95),
        Rule = Sol.Rules.none_none_top
    },
    FreeCell       = {
        Size = 3,
        Pile = function(i)
            return {
                Position = { x = 0, y = i + 1 },
                Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.Any(), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    Foundation     = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Initial = Sol.Initial.face_up(i == 0 and 1 or 0),
                Rule = Sol.Rules.ff_upsuit_none_l13
            }
        end
    },
    Tableau        = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 1 },
                Initial = Sol.Initial.face_up(1),
                Layout = "Column",
                Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownAlternateColors(true), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    check_playable = function(game, targetPile, targetCardIndex, card, numCards)
        if targetPile.Type == "FreeCell" then -- freecells only accept reserve cards
            if game:find_pile(card).Type ~= "Reserve" then return false end
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end
}

------

local gotham                      = Sol.copy(new_york)
gotham.Info.Name                  = "Gotham"
gotham.Reserve.Initial            = Sol.Initial.top_face_up(79)
gotham.Tableau.Pile               = function(i)
    return {
        Position = { x = i + 2, y = 1 },
        Initial = Sol.Initial.face_up(3),
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.DownByRank(true), Move = Sol.Rules.Move.InSeq() }
    }
end
gotham.check_playable             = function(game, targetPile, targetCardIndex, card, numCards)
    local srcPile = game:find_pile(card)

    if targetPile.Type == "FreeCell" then -- freecells only accept reserve cards
        if srcPile.Type ~= "Reserve" then return false end
    end

    if targetPile.Type == "Tableau" and targetPile.IsEmpty then -- empty piles can only be filled from reserve or freecell
        if srcPile.Type ~= "Reserve" and srcPile.Type ~= "FreeCell" then return false end
    end

    return game:can_play(targetPile, targetCardIndex, card, numCards)
end

------

------------------------

Sol.register_game(fan)
Sol.register_game(alexander_the_great)
Sol.register_game(bear_river)
Sol.register_game(belvedere)
Sol.register_game(box_fan)
Sol.register_game(bristol)
Sol.register_game(ceiling_fan)
Sol.register_game(clover_leaf)
Sol.register_game(club)
Sol.register_game(crescent)
Sol.register_game(dover)
Sol.register_game(fascination_fan)
Sol.register_game(free_fan)
Sol.register_game(gotham)
Sol.register_game(house_in_the_wood)
Sol.register_game(house_on_the_hill)
Sol.register_game(new_york)
Sol.register_game(quads)
Sol.register_game(quads_plus)
Sol.register_game(lucky_piles)
Sol.register_game(scotch_patience)
Sol.register_game(shamrocks)
Sol.register_game(shamrocks_2)
Sol.register_game(troika)
