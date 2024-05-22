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
    local str = ""
    if target == "To Waste" then
        str = "function (game) return Sol.Ops.Deal.to_group(game.Stock[1], game.Waste) end"
    elseif target == "To Waste by Threes" then
        str = [[
function(game)
        local check = true
        for i = 1, 3 do
            check = check and Sol.Ops.Deal.to_group(game.Stock[1], game.Waste)
        end
        return check
    end]]
    elseif target == "To Tableau" then
        str = "Sol.Ops.Deal.stock_to_tableau"
    end
    return "deal = " .. str .. ",\n"
end

local function get_layout(obj)
    local hasReserve <const>    = obj.ReserveSize.value > 0
    local hasFreeCell <const>   = obj.FreeCellSize.value > 0
    local hasWaste <const>      = obj.WasteSize.value > 0
    local numStockCards <const> = obj.spnDecks.value * 52 - obj.ReserveCardCount.value - obj.FreeCellCardCount.value - obj.TableauCardCount.value
    local hasStock <const>      = numStockCards > 0
    local hasTableau <const>    = obj.TableauSize.value > 0

    local posStock              = "{ x = 0, y = 0 }"

    local posWaste              = "{ x = 1, y = i }"
    local posReserve            = "{ x = 0, y = i + 1 }"

    local posFoundationXOff     = 0
    if hasStock then posFoundationXOff = posFoundationXOff + 2 end
    if hasWaste then posFoundationXOff = posFoundationXOff + 1 end
    local posTableauXOff = posFoundationXOff
    if hasReserve then posTableauXOff = posTableauXOff + 1 end

    local posFreeCell = "{ x = i + " .. posFoundationXOff .. ", y = 0 }"
    posFoundationXOff = posFoundationXOff + obj.FreeCellSize.value
    if hasFreeCell then posFoundationXOff = posFoundationXOff + 1 end

    local posFoundation = "{ x = i + " .. posFoundationXOff .. ", y = 0 }"

    local posTableau    = "{ x = i + " .. posTableauXOff .. ", y = 1 }"

    return {
        Waste       = posWaste,
        HasWaste    = hasWaste,

        Reserve     = posReserve,
        HasReserve  = hasReserve,

        FreeCell    = posFreeCell,
        HasFreeCell = hasFreeCell,

        Foundation  = posFoundation,

        Tableau     = posTableau,
        HasTableau  = hasTableau,

        Stock       = posStock,
        HasStock    = hasStock,
        StockCards  = numStockCards
    }
end


return function(obj)
    local log           = {}
    local layout        = get_layout(obj)

    local strInfo       = [[
    Info = {
        Name      = "Wizard_]] .. obj.txtName.text .. [[",
        Family    = "]] .. "Other" .. [[",
        DeckCount = ]] .. obj.spnDecks.value .. [[,
        Redeals   = ]] .. obj.Redeals.value .. [[,
    },]] .. "\n"

    local strWaste      = [[
    Waste = {
        Size     = ]] .. obj.WasteSize.value .. [[,
        Pile     = function(i)
            return {
                Position = ]] .. layout.Waste .. [[,
                Layout   = "]] .. obj.WasteLayout.selected .. [[",
            }
        end,
    },]] .. "\n"

    local strReserve    = [[
    Reserve = {
        Size = ]] .. obj.ReserveSize.value .. [[,
        Pile = function(i)
            return {
                Position = ]] .. layout.Reserve .. [[,
                Initial  = ]] .. get_initial(obj.ReserveOrientation.selected, obj.ReserveCardCount.value, obj.ReserveSize.value) .. [[,
                Layout   = "]] .. obj.ReserveLayout.selected .. [[",
            }
        end,
    },]] .. "\n"

    local strFreeCell   = [[
    FreeCell = {
        Size = ]] .. obj.FreeCellSize.value .. [[,
        Pile = function(i)
            return {
                Position = ]] .. layout.FreeCell .. [[,
                Initial  = ]] .. get_initial("Face Up", obj.FreeCellCardCount.value, obj.FreeCellSize.value) .. [[,
                Rule     = { ]] .. get_rule(obj.FreeCellBase.selected, obj.FreeCellBuild.selected, obj.FreeCellMove.selected) .. [[ },
                Layout   = "]] .. obj.FreeCellLayout.selected .. [[",
            }
        end,
    },]] .. "\n"

    local strFoundation = [[
    Foundation = {
        Size = ]] .. obj.spnDecks.value * 4 .. [[,
        Pile = function(i)
            return {
                Position = ]] .. layout.Foundation .. [[,
                Rule     = { ]] .. get_rule(obj.FoundationBase.selected, obj.FoundationBuild.selected, obj.FoundationMove.selected) .. [[ },
                Layout   = "]] .. obj.FoundationLayout.selected .. [[",
            }
        end,
    },]] .. "\n"

    local strTableau    = [[
    Tableau = {
        Size = ]] .. obj.TableauSize.value .. [[,
        Pile = function(i)
            return {
                Position = ]] .. layout.Tableau .. [[,
                Initial  = ]] .. get_initial(obj.TableauOrientation.selected, obj.TableauCardCount.value, obj.TableauSize.value) .. [[,
                Rule     = { ]] .. get_rule(obj.TableauBase.selected, obj.TableauBuild.selected, obj.TableauMove.selected) .. [[ },
                Layout   = "]] .. obj.TableauLayout.selected .. [[",
            }
        end,
    },]] .. "\n"

    local strStock      = [[
    Stock = {
        Position = ]] .. layout.Stock .. [[,
        Size = ]] .. (layout.HasStock and 1 or 0) .. [[,
        Pile = { Initial = ]] .. get_initial("Face Down", layout.StockCards, 1) .. [[ },
    },]] .. "\n"

    local strDeal       = ""
    local strRedeal     = "\n"
    if layout.HasStock then
        strDeal = get_deal(obj.StockTarget.selected)
        if layout.HasWaste then
            strRedeal = "redeal = Sol.Ops.Redeal.waste_to_stock,\n"
        end
    end

    local game = [[
local game = {
]] .. strInfo .. [[
]] .. strStock .. [[
]] .. strWaste .. [[
]] .. strReserve .. [[
]] .. strFreeCell .. [[
]] .. strFoundation .. [[
]] .. strTableau .. [[
    ]] .. strDeal .. [[
    ]] .. strRedeal .. [[
}

Sol.register_game(game)
]]

    -- error checking
    if obj.txtName.text == "" or string.find(obj.txtName.text, "[\\/:*?\"<>|]") ~= nil then
        log[#log + 1] = "ERROR: invalid name"
    end
    if layout.StockCards < 0 then
        log[#log + 1] = "ERROR: insufficient amount of cards"
    end
    if layout.HasStock and not layout.HasWaste and string.find(obj.StockTarget.selected, "Waste") then
        log[#log + 1] = "ERROR: invalid deal target (waste)"
    end
    if layout.HasStock and not layout.HasTableau and string.find(obj.StockTarget.selected, "Tableau") then
        log[#log + 1] = "ERROR: invalid deal target (tableau)"
    end
    if not layout.HasStock and layout.HasWaste then
        log[#log + 1] = "ERROR: waste without stock"
    end

    return game, log
end
