-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local function get_piles(game)
    local tableau    = game.Tableau
    local foundation = game.Foundation
    local stock      = game.Stock
    local waste      = game.Waste
    local freecell   = game.FreeCell
    local reserve    = game.Reserve
    return {
        Stock = stock,
        HasStock = #stock > 0,
        Waste = waste,
        WasteSize = #waste,
        HasWaste = #waste > 0,
        Tableau = tableau,
        TableauSize = #tableau,
        Foundation = foundation,
        FoundationSize = #foundation,
        FreeCell = freecell,
        FreeCellSize = #freecell,
        Reserve = reserve,
        ReserveSize = #reserve,
    }
end

return {
    -- Stock            -> bottom right
    -- Foundation       -> right
    -- Tableau          -> two rows
    bakers_dozen = function(game)
        local piles = get_piles(game)

        local tabColumns = piles.TableauSize // 2 + piles.TableauSize % 2
        local fouColumns = piles.FoundationSize // 4

        if piles.HasStock then
            piles.Stock[1].Position = { x = tabColumns, y = 4 }
        end

        for i = 0, piles.FoundationSize - 1 do
            piles.Foundation[i + 1].Position = { x = i % fouColumns + tabColumns, y = i // fouColumns }
        end

        for i = 0, piles.TableauSize - 1 do
            piles.Tableau[i + 1].Position = { x = i % tabColumns, y = i // tabColumns * 2.5 }
        end
    end,
    -- Foundation       -> center
    -- Tableau          -> left and right
    beleaguered_castle = function(game)
        local piles = get_piles(game)

        local middle = piles.TableauSize / 2
        for i = 0, piles.FoundationSize - 1 do
            piles.Foundation[i + 1].Position = { x = middle, y = i }
        end

        for i = 0, piles.TableauSize - 1 do
            if i < piles.TableauSize / 2 then
                piles.Tableau[i + 1].Position = { x = 0, y = i }
            else
                piles.Tableau[i + 1].Position = { x = middle + 1, y = i - middle }
            end
        end
    end,
    -- Stock, Waste and Foundation  -> bottom right
    -- Tableau                      -> top
    big_harp = function(game)
        local piles = get_piles(game)

        local bottomRow = 4

        if piles.HasStock then
            piles.Stock[1].Position = { x = piles.FoundationSize + 1, y = bottomRow }
        end
        if piles.HasWaste then
            piles.Waste[1].Position = { x = piles.FoundationSize, y = bottomRow }
        end

        for i = 0, piles.FoundationSize - 1 do
            piles.Foundation[i + 1].Position = { x = i, y = bottomRow }
        end

        local tableauOffsetX = (piles.FoundationSize + 2 - piles.TableauSize) / 2
        tableauOffsetX = math.max(0, tableauOffsetX)

        for i = 0, piles.TableauSize - 1 do
            piles.Tableau[i + 1].Position = { x = i + tableauOffsetX, y = 0 }
        end
    end,
    -- Stock, Waste, single Reserve/FreeCell    -> top left
    -- Foundation                               -> top right
    -- Tableau                                  -> second row
    canfield = function(game)
        local piles = get_piles(game)

        local offsetX = 3
        if piles.HasStock then
            piles.Stock[1].Position = { x = 0, y = 0 }
        end
        if piles.HasWaste then
            piles.Waste[1].Position = { x = 1, y = 0 }
        else
            offsetX = 2
        end
        if piles.ReserveSize > 0 then
            piles.Reserve[1].Position = { x = 0, y = 1 }
        elseif piles.FreeCellSize > 0 then
            piles.FreeCell[1].Position = { x = 0, y = 1 }
        end

        for i = 0, piles.FoundationSize - 1 do
            piles.Foundation[i + 1].Position = { x = i + offsetX, y = 0 }
        end

        for i = 0, piles.TableauSize - 1 do
            piles.Tableau[i + 1].Position = { x = i + offsetX, y = 1 }
        end
    end,
    -- Foundation       -> top right
    -- Tableau          -> second row
    canister = function(game)
        local piles             = get_piles(game)

        local foundationOffsetX = (piles.TableauSize - piles.FoundationSize) / 2
        foundationOffsetX       = math.max(0, foundationOffsetX)

        for i = 0, piles.FoundationSize - 1 do
            piles.Foundation[i + 1].Position = { x = i + foundationOffsetX, y = 0 }
        end

        local tableauOffsetX = (piles.FoundationSize - piles.TableauSize) / 2
        tableauOffsetX = math.max(0, tableauOffsetX)

        for i = 0, piles.TableauSize - 1 do
            piles.Tableau[i + 1].Position = { x = i + tableauOffsetX, y = 1 }
        end
    end,
    -- Stock            -> top left
    -- Foundation       -> top right
    -- Tableau          -> second row
    capricieuse = function(game)
        local piles = get_piles(game)

        if piles.HasStock then
            piles.Stock[1].Position = { x = 0, y = 0 }
        end

        for i = 0, piles.FoundationSize - 1 do
            piles.Foundation[i + 1].Position = { x = i + 2, y = 0 }
        end

        for i = 0, piles.TableauSize - 1 do
            piles.Tableau[i + 1].Position = { x = i, y = 1 }
        end
    end,
    -- Foundation       -> top
    -- FreeCells        -> second row
    -- Tableau          -> third row
    double_free_cell = function(game)
        local piles             = get_piles(game)

        local foundationOffsetX = (piles.TableauSize - piles.FoundationSize) / 2
        foundationOffsetX       = math.max(0, foundationOffsetX)


        for i = 0, piles.FoundationSize - 1 do
            piles.Foundation[i + 1].Position = { x = foundationOffsetX + i, y = 0 }
        end

        local fcOffsetX = (piles.TableauSize - piles.FreeCellSize) / 2
        fcOffsetX = math.max(0, fcOffsetX)

        for i = 0, piles.FreeCellSize - 1 do
            piles.FreeCell[i + 1].Position = { x = fcOffsetX + i, y = 1 }
        end

        for i = 0, piles.TableauSize - 1 do
            piles.Tableau[i + 1].Position = { x = i, y = 2 }
        end
    end,
    -- Foundation       -> right or top
    -- Tableau          -> in columns
    -- Stock and Waste  -> bottom
    fan = function(game, columns)
        local piles = get_piles(game)
        local maxY = 0

        if piles.FoundationSize <= 4 then
            --foundation right
            for i = 0, piles.FoundationSize - 1 do
                piles.Foundation[i + 1].Position = { x = columns * 2, y = i }
            end

            for i = 0, piles.TableauSize - 1 do
                maxY = math.max(maxY, i // columns)
                piles.Tableau[i + 1].Position = { x = (i % columns) * 2, y = i // columns }
            end
        else
            --foundation top
            local foundationOffsetX = math.max(0, ((columns * 2) - piles.FoundationSize) / 2)

            for i = 0, piles.FoundationSize - 1 do
                piles.Foundation[i + 1].Position = { x = foundationOffsetX + i, y = 0 }
            end

            for i = 0, piles.TableauSize - 1 do
                maxY = math.max(maxY, i // columns + 1)
                piles.Tableau[i + 1].Position = { x = (i % columns) * 2, y = i // columns + 1 }
            end
        end

        if piles.HasStock then
            local stockOffsetX = math.max(0, ((columns * 2) - (piles.WasteSize + 1)) / 2)
            local stockOffsetY = maxY + 1
            piles.Stock[1].Position = { x = stockOffsetX, y = stockOffsetY }

            for i = 1, piles.WasteSize do
                piles.Waste[i].Position = { x = stockOffsetX + i, y = stockOffsetY }
            end
        end
    end,
    -- FreeCell         -> top
    -- Foundation       -> middle
    -- Tableau          -> left and right
    fastness = function(game)
        local piles  = get_piles(game)

        local middle = piles.TableauSize / 2
        for i = 0, piles.FreeCellSize - 1 do
            piles.FreeCell[i + 1].Position = { x = i + (middle + 1) / 2, y = 0 }
        end

        for i = 0, piles.FoundationSize - 1 do
            piles.Foundation[i + 1].Position = { x = middle, y = i + 1 }
        end

        for i = 0, piles.TableauSize - 1 do
            if i < piles.TableauSize / 2 then
                piles.Tableau[i + 1].Position = { x = 0, y = i + 1 }
            else
                piles.Tableau[i + 1].Position = { x = middle + 1, y = i - middle + 1 }
            end
        end
    end,
    -- FreeCells        -> bottom
    -- Foundation       -> right
    -- Tableau          -> top
    flipper = function(game)
        local piles = get_piles(game)

        for i = 0, piles.FreeCellSize - 1 do
            piles.FreeCell[i + 1].Position = { x = i + (piles.TableauSize - piles.FreeCellSize) / 2, y = 3 }
        end

        for i = 0, piles.FoundationSize - 1 do
            piles.Foundation[i + 1].Position = { x = math.max(piles.FreeCellSize, piles.TableauSize), y = i }
        end

        for i = 0, piles.TableauSize - 1 do
            piles.Tableau[i + 1].Position = { x = i, y = 0 }
        end
    end,
    -- Stock and Waste  -> bottom right
    -- Foundation       -> top
    -- Tableau          -> second row
    forty_thieves = function(game)
        local piles = get_piles(game)

        local foundationOffsetX = math.max(0, (piles.TableauSize - piles.FoundationSize) / 2)

        for i = 0, piles.FoundationSize - 1 do
            piles.Foundation[i + 1].Position = { x = i + foundationOffsetX, y = 0 }
        end

        local tableauOffsetX = (piles.FoundationSize - piles.TableauSize) / 2
        tableauOffsetX = math.max(0, tableauOffsetX)

        for i = 0, piles.TableauSize - 1 do
            piles.Tableau[i + 1].Position = { x = i + tableauOffsetX, y = 1 }
        end

        local bottomRow = 3
        if piles.HasStock then
            piles.Stock[1].Position = { x = foundationOffsetX + piles.FoundationSize - 1, y = bottomRow }
        end
        if piles.HasWaste then
            piles.Waste[1].Position = { x = foundationOffsetX + piles.FoundationSize - 2, y = bottomRow }
        end
    end,
    -- FreeCells        -> top left
    -- Foundation       -> top right
    -- Tableau          -> second row
    free_cell = function(game)
        local piles = get_piles(game)

        for i = 0, piles.FreeCellSize - 1 do
            piles.FreeCell[i + 1].Position = { x = i, y = 0 }
        end

        local foundationOffsetX = (piles.TableauSize - piles.FreeCellSize - piles.FoundationSize) / 2 + 1
        foundationOffsetX = math.max(1, foundationOffsetX)

        for i = 0, piles.FoundationSize - 1 do
            piles.Foundation[i + 1].Position = { x = i + piles.FreeCellSize + foundationOffsetX, y = 0 }
        end

        local tableauOffsetX = (piles.FreeCellSize + piles.FoundationSize + 1 - piles.TableauSize) / 2
        tableauOffsetX = math.max(0, tableauOffsetX)

        for i = 0, piles.TableauSize - 1 do
            piles.Tableau[i + 1].Position = { x = i + tableauOffsetX, y = 1 }
        end
    end,
    -- single Stock and Foundation  -> bottom center
    -- Tableau          -> top
    golf = function(game)
        local piles = get_piles(game)

        if piles.HasStock then
            piles.Stock[1].Position = { x = piles.TableauSize / 2 - 1, y = 2 }
        end
        if piles.FoundationSize > 0 then
            piles.Foundation[1].Position = { x = piles.TableauSize / 2, y = 2 }
        end

        for i = 0, piles.TableauSize - 1 do
            piles.Tableau[i + 1].Position = { x = i, y = 0 }
        end
    end,
    -- Stock            -> bottom right
    -- Foundation       -> right
    -- Tableau          -> top
    gypsy = function(game)
        local piles = get_piles(game)

        if piles.HasStock then
            piles.Stock[1].Position = { x = piles.TableauSize + 0.5, y = 4 }
        end

        for i = 0, piles.FoundationSize - 1 do
            piles.Foundation[i + 1].Position = { x = i % 2 + piles.TableauSize, y = i // 2 }
        end

        for i = 0, piles.TableauSize - 1 do
            piles.Tableau[i + 1].Position = { x = i, y = 0 }
        end
    end,
    -- Stock and Waste  -> top left
    -- Foundation       -> top right
    -- Tableau          -> second row
    klondike = function(game)
        local piles = get_piles(game)

        local offX  = 3
        if piles.HasStock then
            piles.Stock[1].Position = { x = 0, y = 0 }
        end

        if piles.HasWaste then
            piles.Waste[1].Position = { x = 1, y = 0 }
        else
            offX = offX - 1
        end

        local foundationOffsetX = piles.TableauSize - piles.FoundationSize - offX
        foundationOffsetX = math.max(0, foundationOffsetX)

        for i = 0, piles.FoundationSize - 1 do
            piles.Foundation[i + 1].Position = { x = i + offX + foundationOffsetX, y = 0 }
        end

        local tableauOffsetX = (piles.FoundationSize + offX + foundationOffsetX - piles.TableauSize) / 2
        tableauOffsetX = math.max(0, tableauOffsetX)

        for i = 0, piles.TableauSize - 1 do
            piles.Tableau[i + 1].Position = { x = i + tableauOffsetX, y = 1 }
        end
    end,
    -- Stock            -> bottom center
    -- Tableau          -> in columns
    montana = function(game, columns)
        local piles = get_piles(game)

        if piles.HasStock then
            piles.Stock[1].Position = { x = (columns - 1) / 2, y = piles.TableauSize // columns }
        end

        for i = 0, piles.TableauSize - 1 do
            piles.Tableau[i + 1].Position = { x = i % columns, y = i // columns }
        end
    end,
    -- Stock            -> top left
    -- Reserve          -> right in columns
    -- Foundation       -> top
    -- Tableau          -> second row
    raglan = function(game, columns)
        local piles = get_piles(game)

        if piles.HasStock then
            piles.Stock[1].Position = { x = 0, y = 0 }
        end

        for i = 0, piles.ReserveSize - 1 do
            piles.Reserve[i + 1].Position = { x = i % columns + piles.TableauSize, y = i // columns + 1 }
        end

        local foundationOffsetX = (piles.TableauSize - piles.FoundationSize) / 2
        foundationOffsetX = math.max(piles.HasStock and 2 or 0, foundationOffsetX)

        for i = 0, piles.FoundationSize - 1 do
            piles.Foundation[i + 1].Position = { x = foundationOffsetX + i, y = 0 }
        end

        for i = 0, piles.TableauSize - 1 do
            piles.Tableau[i + 1].Position = { x = i, y = 1 }
        end
    end,
    -- Foundation       -> right
    -- Tableau          -> top
    yukon = function(game)
        local piles = get_piles(game)

        for i = 0, piles.FoundationSize - 1 do
            piles.Foundation[i + 1].Position = { x = piles.TableauSize + i // 4, y = i % 4 }
        end

        for i = 0, piles.TableauSize - 1 do
            piles.Tableau[i + 1].Position = { x = i, y = 0 }
        end
    end
}
