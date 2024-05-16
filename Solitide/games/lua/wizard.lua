-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local function get_initial(orient, cards, piles)
    local count = "0"
    if piles > 0 then
        local breakPile = cards % piles
        if breakPile == 0 then
            count = tostring(cards // piles)
        else
            local cardCount = cards // piles
            count = "i < " .. breakPile .. " and " .. tostring(cardCount + 1) .. " or " .. tostring(cardCount)
        end
    end

    local str = "Sol.Initial."
    if orient == "Face Down" then
        str = str .. "face_down(" .. count .. ")"
    elseif orient == "Face Up" then
        str = str .. "face_up(" .. count .. ")"
    elseif orient == "Top Card Face Up" then
        str = str .. "top_face_up(" .. count .. ")"
    elseif orient == "Alternate - First Face Up" then
        str = str .. "alternate(" .. count .. ", true)"
    elseif orient == "Alternate - First Face Down" then
        str = str .. "alternate(" .. count .. ", false)"
    end
    return str
end

local function get_rule(base, build, move)
    return "Base = Sol.Rules.Base." .. base .. "(), Build = Sol.Rules.Build." .. build .. "(), Move = Sol.Rules.Move." .. move .. "()"
end

local function get_deal(target)
    local str = "deal = Sol.Ops.Deal."
    if target == "To Waste" then
        str = str .. "stock_to_waste"
    elseif target == "To Waste by Threes" then
        str = str .. "stock_to_waste_by_3"
    elseif target == "To Tableau" then
        str = str .. "stock_to_tableau"
    end
    return str .. ",\n"
end

local function check_layout(layout, log, hasReserve, hasFreeCell, hasWaste, hasStock)
    local function contains(tab, val)
        for _, value in ipairs(tab) do if value == val then return true end end
        return false
    end

    local noReserve = { "bakers_dozen", "beleaguered_castle", "big_harp", "canister", "capricieuse", "double_free_cell", "fastness", "flipper", "forty_thieves", "free_cell", "golf", "gypsy", "klondike", "yukon" }
    if contains(noReserve, layout) then if hasReserve then log[#log + 1] = "ERROR: layout does not support reserve piles" end end
    local noFreeCell = { "bakers_dozen", "beleaguered_castle", "big_harp", "canister", "capricieuse", "flipper", "golf", "gypsy", "klondike", "yukon" }
    if contains(noFreeCell, layout) then if hasFreeCell then log[#log + 1] = "ERROR: layout does not support freecell piles" end end
    local noWaste = { "bakers_dozen", "beleaguered_castle", "canister", "capricieuse", "double_free_cell", "fastness", "flipper", "free_cell", "gypsy", "yukon" }
    if contains(noWaste, layout) then if hasWaste then log[#log + 1] = "ERROR: layout does not support waste piles" end end
    local noStock = { "beleaguered_castle", "canister", "double_free_cell", "fastness", "flipper", "yukon" }
    if contains(noStock, layout) then if hasStock then log[#log + 1] = "ERROR: layout does not support stock piles" end end
end

return function(obj)
    local log = {}

    local numCards = obj.spnDecks.value * 52

    local strInfo = [[
    Info = {
        Name      = "Wizard_]] .. obj.txtName.text .. [[",
        Family    = "]] .. "Other" .. [[",
        DeckCount = ]] .. obj.spnDecks.value .. [[,
        Redeals   = ]] .. obj.Redeals.value .. [[,
    }]]

    local strWaste = [[
    Waste = {
        Size  = ]] .. obj.WasteSize.value .. [[,
        Pile = function(i)
            return {
                Layout  = "]] .. obj.WasteLayout.selected .. [[",
            }
        end,
    }]]

    local strReserve = [[
    Reserve = {
        Size = ]] .. obj.ReserveSize.value .. [[,
        Pile = function(i)
            return {
                Initial = ]] .. get_initial(obj.ReserveOrientation.selected, obj.ReserveCardCount.value, obj.ReserveSize.value) .. [[,
                Layout  = "]] .. obj.ReserveLayout.selected .. [[",
            }
        end,
    }]]
    numCards = numCards - obj.ReserveCardCount.value

    local strFreeCell = [[
    FreeCell = {
        Size = ]] .. obj.FreeCellSize.value .. [[,
        Pile = function(i)
            return {
                Initial = ]] .. get_initial("Face Up", obj.FreeCellCardCount.value, obj.FreeCellSize.value) .. [[,
                Rule    = { ]] .. get_rule(obj.FreeCellBase.selected, obj.FreeCellBuild.selected, obj.FreeCellMove.selected) .. [[ },
                Layout  = "]] .. obj.FreeCellLayout.selected .. [[",
            }
        end,
    }]]
    numCards = numCards - obj.FreeCellCardCount.value

    local strFoundation = [[
    Foundation = {
        Size = ]] .. obj.spnDecks.value * 4 .. [[,
        Pile = function(i)
            return {
                Rule    = { ]] .. get_rule(obj.FoundationBase.selected, obj.FoundationBuild.selected, obj.FoundationMove.selected) .. [[ },
                Layout  = "]] .. obj.FoundationLayout.selected .. [[",
            }
        end,
    }]]

    local strTableau = [[
    Tableau = {
        Size = ]] .. obj.TableauSize.value .. [[,
        Pile = function(i)
            return {
                Initial = ]] .. get_initial(obj.TableauOrientation.selected, obj.TableauCardCount.value, obj.TableauSize.value) .. [[,
                Rule    = { ]] .. get_rule(obj.TableauBase.selected, obj.TableauBuild.selected, obj.TableauMove.selected) .. [[ },
                Layout  = "]] .. obj.TableauLayout.selected .. [[",
            }
        end,
    }]]
    numCards = numCards - obj.TableauCardCount.value

    local stockSize = numCards > 0 and 1 or 0
    local strStock = [[
    Stock = {
        Size = ]] .. stockSize .. [[,
        Pile = { Initial = ]] .. get_initial("Face Down", numCards, 1) .. [[ },
    }]]

    local strDeal = ""
    local strRedeal = "\n"
    if numCards > 0 then
        strDeal = get_deal(obj.StockTarget.selected)
        if obj.WasteSize.value > 0 then
            strRedeal = "redeal = Sol.Ops.Redeal.waste_to_stock,\n"
        end
    end

    local game = [[
local game = {
]] .. strInfo .. [[,
]] .. strStock .. [[,
]] .. strWaste .. [[,
]] .. strReserve .. [[,
]] .. strFreeCell .. [[,
]] .. strFoundation .. [[,
]] .. strTableau .. [[,
    on_init = Sol.Layout.]] .. obj.cybLayout.selected .. [[,
    ]] .. strDeal .. [[
    ]] .. strRedeal .. [[
}

Sol.register_game(game)
]]

    -- error checking
    if obj.txtName.text == "" or string.find(obj.txtName.text, "[\\/:*?\"<>|]") ~= nil then
        log[#log + 1] = "ERROR: invalid name"
    end
    if numCards < 0 then
        log[#log + 1] = "ERROR: insufficient amount of cards"
    end
    if numCards > 0 and obj.WasteSize.value == 0 and string.find(obj.StockTarget.selected, Sol.Pile.Type.Waste) then
        log[#log + 1] = "ERROR: stock deals to waste, but no waste available"
    end
    if numCards > 0 and obj.TableauSize.value == 0 and string.find(obj.StockTarget.selected, Sol.Pile.Type.Tableau) then
        log[#log + 1] = "ERROR: stock deals to tableau, but no tableau available"
    end

    local hasReserve = obj.ReserveSize.value > 0
    local hasFreeCell = obj.FreeCellSize.value > 0
    local hasWaste = obj.WasteSize.value > 0
    local hasStock = numCards > 0
    check_layout(obj.cybLayout.selected, log, hasReserve, hasFreeCell, hasWaste, hasStock)

    return game, log
end
