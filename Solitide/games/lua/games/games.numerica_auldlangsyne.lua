-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local auld_lang_syne = {
    Info              = {
        Name      = "Auld Lang Syne",
        Family    = "Numerica",
        DeckCount = 1
    },
    Stock             = {
        Initial = Sol.Initial.face_down(48)
    },
    Foundation        = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_uprank_none }
    },
    Tableau           = {
        Size = 4,
        Pile = {
            Layout = "Column",
            Rule = Sol.Rules.none_none_top
        }
    },
    on_before_shuffle = Sol.Ops.Shuffle.ace_to_foundation,
    deal              = Sol.Ops.Deal.stock_to_tableau,
    on_init           = Sol.Layout.klondike
}


------

local old_fashioned        = Sol.copy(auld_lang_syne)
old_fashioned.Info.Name    = "Old Fashioned"
old_fashioned.Tableau.Size = 6


------

local tam_o_shanter             = Sol.copy(auld_lang_syne)
tam_o_shanter.Info.Name         = "Tam O'Shanter"
tam_o_shanter.Stock.Initial     = Sol.Initial.face_down(52)
tam_o_shanter.on_before_shuffle = nil


------

local acquaintance        = Sol.copy(auld_lang_syne)
acquaintance.Info.Name    = "Acquaintance"
acquaintance.Info.Redeals = 2
acquaintance.redeal       = function(game)
    local stock = game.Stock[1]
    if not stock.IsEmpty then return false end

    local tableau = game.Tableau

    local cards = {}
    for _, tab in ipairs(tableau) do
        local tabCards = tab.Cards
        for j = #tabCards, 1, -1 do
            cards[#cards + 1] = tabCards[j]
        end
        tab:clear_cards()
    end
    if #cards == 0 then return false end

    while #cards > 0 do
        game.PlaceTop(table.remove(cards, 1), stock, false)
    end

    stock:flip_down_cards()

    return true
end


------

local double_acquaintance           = Sol.copy(acquaintance)
double_acquaintance.Info.Name       = "Double Acquaintance"
double_acquaintance.Info.DeckCount  = 2
double_acquaintance.Stock.Initial   = Sol.Initial.face_down(96)
double_acquaintance.Foundation.Size = 8
double_acquaintance.Tableau.Size    = 8


------

local scuffle               = Sol.copy(acquaintance)
scuffle.Info.Name           = "Scuffle"
scuffle.Tableau.Pile.Layout = "Squared"


------

------------------------

Sol.register_game(auld_lang_syne)
Sol.register_game(acquaintance)
Sol.register_game(double_acquaintance)
Sol.register_game(old_fashioned)
Sol.register_game(scuffle)
Sol.register_game(tam_o_shanter)
