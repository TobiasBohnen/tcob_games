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
    local body = ""
    if target == "To Waste" then
        body = "return Sol.Ops.Deal.to_group(game.Stock[1], game.Waste)"
    elseif target == "To Waste by Threes" then
        body = [[
local check = true
        for i = 1, 3 do
            check = check and Sol.Ops.Deal.to_group(game.Stock[1], game.Waste)
        end
        return check]]
    elseif target == "To Tableau" then
        body = "return Sol.Ops.Deal.stock_to_tableau(game)"
    end

    return [[
    deal = function(game)
        ]] .. body .. "\n" .. [[
    end,]] .. "\n"
end

local function get_options(obj)
    local hasReserve <const>    = obj.ReserveSize.value > 0
    local hasFreeCell <const>   = obj.FreeCellSize.value > 0
    local hasWaste <const>      = obj.WasteSize.value > 0
    local numStockCards <const> = obj.Decks.value * 52 - obj.ReserveCardCount.value - obj.FreeCellCardCount.value - obj.TableauCardCount.value
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
        Info       = {
            Name      = obj.Name.text,
            DeckCount = obj.Decks.value,
            Redeals   = obj.Redeals.value
        },
        Waste      = {
            IsPresent = hasWaste,
            Position  = posWaste,
            Size      = obj.WasteSize.value,
            Layout    = obj.WasteLayout.selected
        },
        Reserve    = {
            IsPresent   = hasReserve,
            Position    = posReserve,
            Size        = obj.ReserveSize.value,
            CardCount   = obj.ReserveCardCount.value,
            Orientation = obj.ReserveOrientation.selected,
            Layout      = obj.ReserveLayout.selected
        },
        FreeCell   = {
            IsPresent = hasFreeCell,
            Position  = posFreeCell,
            Size      = obj.FreeCellSize.value,
            CardCount = obj.FreeCellCardCount.value,
            Layout    = obj.FreeCellLayout.selected,
            Rule      = {
                Base = obj.FreeCellBase.selected,
                Build = obj.FreeCellBuild.selected,
                Move = obj.FreeCellMove.selected
            }
        },
        Foundation = {
            Position = posFoundation,
            Size     = obj.Decks.value * 4,
            Layout   = obj.FoundationLayout.selected,
            Rule     = {
                Base = obj.FoundationBase.selected,
                Build = obj.FoundationBuild.selected,
                Move = obj.FoundationMove.selected
            }
        },
        Tableau    = {
            IsPresent   = hasTableau,
            Position    = posTableau,
            Size        = obj.TableauSize.value,
            CardCount   = obj.TableauCardCount.value,
            Orientation = obj.TableauOrientation.selected,
            Layout      = obj.TableauLayout.selected,
            Rule        = {
                Base = obj.TableauBase.selected,
                Build = obj.TableauBuild.selected,
                Move = obj.TableauMove.selected
            }
        },
        Stock      = {
            IsPresent = hasStock,
            Position  = posStock,
            CardCount = numStockCards,
            Target    = obj.StockTarget.selected
        }
    }
end

local function check_error(options)
    local log = {}
    -- error checking
    if options.Info.Name == "" or string.find(options.Info.Name, "[\\/:*?\"<>|]") ~= nil then
        log[#log + 1] = "Invalid name: The name is either empty or contains invalid characters."
    end
    if options.Stock.CardCount < 0 then
        log[#log + 1] = "Insufficient amount of cards."
    end
    if options.Stock.IsPresent and not options.Waste.IsPresent and string.find(options.Stock.Target, "Waste") then
        log[#log + 1] = "Invalid deal target: Stock is present but attempting to deal to waste which is not present."
    end
    if options.Stock.IsPresent and not options.Tableau.IsPresent and string.find(options.Stock.Target, "Tableau") then
        log[#log + 1] = "Invalid deal target: Stock is present but attempting to deal to tableau which is not present."
    end
    if not options.Stock.IsPresent and options.Waste.IsPresent then
        log[#log + 1] = "Waste is present without stock."
    end
    if not options.Tableau.IsPresent and options.Tableau.CardCount > 0 then
        log[#log + 1] = "Tableau is not present, but there are " .. options.Tableau.CardCount .. " tableau cards."
    end
    if not options.Reserve.IsPresent and options.Reserve.CardCount > 0 then
        log[#log + 1] = "Reserve is not present, but there are " .. options.Reserve.CardCount .. " reserve cards."
    end
    if not options.FreeCell.IsPresent and options.FreeCell.CardCount > 0 then
        log[#log + 1] = "FreeCell is not present, but there are " .. options.FreeCell.CardCount .. " freecell cards."
    end
    return log
end

return function(obj)
    local options <const> = get_options(obj)
    local name <const>    = "(Wizard) " .. options.Info.Name;

    local strInfo         = [[
    Info = {
        Name      = "]] .. name .. [[",
        Family    = "]] .. "Other" .. [[",
        DeckCount = ]] .. options.Info.DeckCount .. [[,
        Redeals   = ]] .. options.Info.Redeals .. [[,
    },]] .. "\n"

    local strWaste        = [[
    Waste = {
        Size = ]] .. options.Waste.Size .. [[,
        Pile = function(i)
            return {
                Position = ]] .. options.Waste.Position .. [[,
                Layout   = Sol.Pile.Layout.]] .. options.Waste.Layout .. [[,
            }
        end,
    },]] .. "\n"

    local strReserve      = [[
    Reserve = {
        Size = ]] .. options.Reserve.Size .. [[,
        Pile = function(i)
            return {
                Position = ]] .. options.Reserve.Position .. [[,
                Initial  = ]] .. get_initial(options.Reserve.Orientation, options.Reserve.CardCount, options.Reserve.Size) .. [[,
                Layout   = Sol.Pile.Layout.]] .. options.Reserve.Layout .. [[,
            }
        end,
    },]] .. "\n"

    local strFreeCell     = [[
    FreeCell = {
        Size = ]] .. options.FreeCell.Size .. [[,
        Pile = function(i)
            return {
                Position = ]] .. options.FreeCell.Position .. [[,
                Initial  = ]] .. get_initial("Face Up", options.FreeCell.CardCount, options.FreeCell.Size) .. [[,
                Rule     = { ]] .. get_rule(options.FreeCell.Rule.Base, options.FreeCell.Rule.Build, options.FreeCell.Rule.Move) .. [[ },
                Layout   = Sol.Pile.Layout.]] .. options.FreeCell.Layout .. [[,
            }
        end,
    },]] .. "\n"

    local strFoundation   = [[
    Foundation = {
        Size = ]] .. options.Foundation.Size .. [[,
        Pile = function(i)
            return {
                Position = ]] .. options.Foundation.Position .. [[,
                Rule     = { ]] .. get_rule(options.Foundation.Rule.Base, options.Foundation.Rule.Build, options.Foundation.Rule.Move) .. [[ },
                Layout   = Sol.Pile.Layout.]] .. options.Foundation.Layout .. [[,
            }
        end,
    },]] .. "\n"

    local strTableau      = [[
    Tableau = {
        Size = ]] .. options.Tableau.Size .. [[,
        Pile = function(i)
            return {
                Position = ]] .. options.Tableau.Position .. [[,
                Initial  = ]] .. get_initial(options.Tableau.Orientation, options.Tableau.CardCount, options.Tableau.Size) .. [[,
                Rule     = { ]] .. get_rule(options.Tableau.Rule.Base, options.Tableau.Rule.Build, options.Tableau.Rule.Move) .. [[ },
                Layout   = Sol.Pile.Layout.]] .. options.Tableau.Layout .. [[,
            }
        end,
    },]] .. "\n"

    local strStock        = [[
    Stock = {
        Size = ]] .. (options.Stock.IsPresent and 1 or 0) .. [[,
        Pile = {
            Position = ]] .. options.Stock.Position .. [[,
            Initial  = ]] .. get_initial("Face Down", options.Stock.CardCount, 1) .. [[,
        },
    },]] .. "\n"

    local strDeal         = ""
    local strRedeal       = "\n"
    if options.Stock.IsPresent then
        strDeal = get_deal(options.Stock.Target)
        if options.Waste.IsPresent then
            strRedeal = "    redeal = Sol.Ops.Redeal.waste_to_stock,\n"
        end
    end

    local game = [[
local game = {
]] .. strInfo .. strStock .. strWaste .. strReserve .. strFreeCell .. strFoundation .. strTableau .. strDeal .. strRedeal .. [[
}

Sol.register_game(game)
]]

    return game, check_error(options), name
end
