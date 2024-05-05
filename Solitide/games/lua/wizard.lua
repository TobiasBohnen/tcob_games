-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local function get_initial(orient, count)
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
    str = str .. ",\n"
    return str
end

return function(obj)
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
        Size  = "]] .. obj.WasteSize.value .. [[",
        Pile  = { Layout = ]] .. obj.WasteLayout.selected .. [[ },
    }]]

    local strReserve = [[
    Reserve = {
        Size = ]] .. obj.ReserveSize.value .. [[,
        Pile = {
            Initial = ]] .. get_initial(obj.ReserveOrientation.selected, obj.ReserveCardCount.value) .. [[,
            Layout  = "]] .. obj.ReserveLayout.selected .. [[",
        },
    }]]
    numCards = numCards - obj.ReserveSize.value * obj.ReserveCardCount.value

    local strFreeCell = [[
    FreeCell = {
        Size = ]] .. obj.FreeCellSize.value .. [[,
        Pile = {
            Initial = ]] .. get_initial("Face Up", obj.FreeCellCardCount.value) .. [[,
            Rule    = { ]] .. get_rule(obj.FreeCellBase.selected, obj.FreeCellBuild.selected, obj.FreeCellMove.selected) .. [[ },
            Layout  = "]] .. obj.FreeCellLayout.selected .. [[",
        },
    }]]
    numCards = numCards - obj.FreeCellSize.value * obj.FreeCellCardCount.value

    local strFoundation = [[
    Foundation = {
        Size = ]] .. obj.spnDecks.value * 4 .. [[,
        Pile = {
            Rule    = { ]] .. get_rule(obj.FoundationBase.selected, obj.FoundationBuild.selected, obj.FoundationMove.selected) .. [[ },
            Layout  = "]] .. obj.FoundationLayout.selected .. [[",
        },
    }]]

    local strTableau = [[
    Tableau = {
        Size = ]] .. obj.TableauSize.value .. [[,
        Pile = {
            Initial = ]] .. get_initial(obj.TableauOrientation.selected, obj.TableauCardCount.value) .. [[,
            Rule    = { ]] .. get_rule(obj.TableauBase.selected, obj.TableauBuild.selected, obj.TableauMove.selected) .. [[ },
            Layout  = "]] .. obj.TableauLayout.selected .. [[",
        },
    }]]
    numCards = numCards - obj.TableauSize.value * obj.TableauCardCount.value

    if numCards < 0 then return "" end -- ERROR - used more cards than available
    local stockSize = numCards > 0 and 1 or 0
    local strStock = [[
    Stock = {
        Size = ]] .. stockSize .. [[,
        Pile = {
            Initial = ]] .. get_initial("Face Down", numCards) .. [[,
        },
    }]]

    local strDeal = ""
    local strRedeal = ""
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
    ]] .. strDeal .. strRedeal .. [[
}

Sol.register_game(game)
]]


    return game
end
