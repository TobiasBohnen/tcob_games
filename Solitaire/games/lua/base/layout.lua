-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https:--opensource.org/licenses/MIT

return {
    -- Stock            -> bottom right
    -- Foundation       -> right
    -- Tableau          -> two rows
    bakers_dozen = function(game)
        local tabSize    = #game.Tableau
        local fouSize    = #game.Foundation

        local tabColumns = tabSize // 2 + tabSize % 2
        local fouColumns = fouSize // 4

        if #game.Stock > 0 then
            game.Stock[1].Position = { x = tabColumns, y = 4 }
        end

        for i = 0, fouSize - 1 do
            game.Foundation[i + 1].Position = { x = i % fouColumns + tabColumns, y = i // fouColumns }
        end

        for i = 0, tabSize - 1 do
            game.Tableau[i + 1].Position = { x = i % tabColumns, y = i // tabColumns * 2.5 }
        end
    end,
    -- Foundation       -> center
    -- Tableau          -> left and right
    beleaguered_castle = function(game)
        local tabSize = #game.Tableau
        local fouSize = #game.Foundation

        local middle = tabSize / 2
        for i = 0, fouSize - 1 do
            game.Foundation[i + 1].Position = { x = middle, y = i }
        end

        for i = 0, tabSize - 1 do
            if i < tabSize / 2 then
                game.Tableau[i + 1].Position = { x = 0, y = i }
            else
                game.Tableau[i + 1].Position = { x = middle + 1, y = i - middle }
            end
        end
    end,
    -- Stock, Waste and Foundation  -> bottom right
    -- Tableau                      -> top
    big_harp = function(game)
        local tabSize   = #game.Tableau
        local fouSize   = #game.Foundation

        local bottomRow = 4

        if #game.Stock > 0 then
            game.Stock[1].Position = { x = fouSize + 1, y = bottomRow }
        end
        if #game.Waste > 0 then
            game.Waste[1].Position = { x = fouSize, y = bottomRow }
        end

        for i = 0, fouSize - 1 do
            game.Foundation[i + 1].Position = { x = i, y = bottomRow }
        end

        local tabOffsetX = (fouSize + 2 - tabSize) / 2
        tabOffsetX = math.max(0, tabOffsetX)

        for i = 0, tabSize - 1 do
            game.Tableau[i + 1].Position = { x = i + tabOffsetX, y = 0 }
        end
    end,
    -- Stock, Waste and Reserve     -> top left
    -- Foundation                   -> top right
    -- Tableau                      -> second row
    canfield = function(game)
        local tabSize = #game.Tableau
        local fouSize = #game.Foundation

        if #game.Stock > 0 then
            game.Stock[1].Position = { x = 0, y = 0 }
        end
        if #game.Waste > 0 then
            game.Waste[1].Position = { x = 1, y = 0 }
        end
        if #game.Reserve > 0 then
            game.Reserve[1].Position = { x = 0, y = 1 }
        end

        for i = 0, fouSize - 1 do
            game.Foundation[i + 1].Position = { x = i + 3, y = 0 }
        end

        for i = 0, tabSize - 1 do
            game.Tableau[i + 1].Position = { x = i + 3, y = 1 }
        end
    end,
    -- Foundation       -> top right
    -- Tableau          -> second row
    canister = function(game)
        local tabSize = #game.Tableau
        local fouSize = #game.Foundation

        local fouOffsetX = (tabSize - fouSize) / 2
        fouOffsetX = math.max(0, fouOffsetX)

        for i = 0, fouSize - 1 do
            game.Foundation[i + 1].Position = { x = i + fouOffsetX, y = 0 }
        end

        local tabOffsetX = (fouSize - tabSize) / 2
        tabOffsetX = math.max(0, tabOffsetX)

        for i = 0, tabSize - 1 do
            game.Tableau[i + 1].Position = { x = i + tabOffsetX, y = 1 }
        end
    end,
    -- Stock            -> top left
    -- Foundation       -> top right
    -- Tableau          -> second row
    capricieuse = function(game)
        local tabSize = #game.Tableau
        local fouSize = #game.Foundation

        if #game.Stock > 0 then
            game.Stock[1].Position = { x = 0, y = 0 }
        end

        for i = 0, fouSize - 1 do
            game.Foundation[i + 1].Position = { x = i + 2, y = 0 }
        end

        for i = 0, tabSize - 1 do
            game.Tableau[i + 1].Position = { x = i, y = 1 }
        end
    end,
    -- Foundation       -> top
    -- FreeCells        -> second row
    -- Tableau          -> third row
    double_free_cell = function(game)
        local fcSize = #game.FreeCell
        local tabSize = #game.Tableau
        local fouSize = #game.Foundation

        local fouOffsetX = (tabSize - fouSize) / 2
        fouOffsetX = math.max(0, fouOffsetX)

        for i = 0, fouSize - 1 do
            game.Foundation[i + 1].Position = { x = fouOffsetX + i, y = 0 }
        end

        local fcOffsetX = (tabSize - fcSize) / 2
        fcOffsetX = math.max(0, fcOffsetX)

        for i = 0, fcSize - 1 do
            game.FreeCell[i + 1].Position = { x = fcOffsetX + i, y = 1 }
        end

        for i = 0, tabSize - 1 do
            game.Tableau[i + 1].Position = { x = i, y = 2 }
        end
    end,
    -- Foundation       -> left
    -- Tableau          -> in columns
    fan = function(game, columns)
        local tabSize = #game.Tableau
        local fouSize = #game.Foundation

        local fouOffsetX = (columns - fouSize) / 2
        fouOffsetX = math.max(0, fouOffsetX)

        for i = 0, fouSize - 1 do
            game.Foundation[i + 1].Position = { x = columns * 2, y = i }
        end

        for i = 0, tabSize - 1 do
            game.Tableau[i + 1].Position = { x = (i % columns) * 2, y = i // columns }
        end
    end,
    -- FreeCell         -> top
    -- Foundation       -> middle
    -- Tableau          -> left and right
    fastness = function(game)
        local fcSize = #game.FreeCell
        local tabSize = #game.Tableau
        local fouSize = #game.Foundation

        local middle = tabSize / 2

        for i = 0, fcSize - 1 do
            game.FreeCell[i + 1].Position = { x = i + (middle + 1) / 2, y = 0 }
        end

        for i = 0, fouSize - 1 do
            game.Foundation[i + 1].Position = { x = middle, y = i + 1 }
        end

        for i = 0, tabSize - 1 do
            if i < tabSize / 2 then
                game.Tableau[i + 1].Position = { x = 0, y = i + 1 }
            else
                game.Tableau[i + 1].Position = { x = middle + 1, y = i - middle + 1 }
            end
        end
    end,
    -- FreeCells        -> bottom
    -- Foundation       -> right
    -- Tableau          -> top
    flipper = function(game)
        local fcSize = #game.FreeCell
        local tabSize = #game.Tableau
        local fouSize = #game.Foundation

        for i = 0, fcSize - 1 do
            game.FreeCell[i + 1].Position = { x = i, y = 3 }
        end

        for i = 0, fouSize - 1 do
            game.Foundation[i + 1].Position = { x = fcSize, y = i }
        end

        for i = 0, tabSize - 1 do
            game.Tableau[i + 1].Position = { x = i, y = 0 }
        end
    end,
    -- Stock and Waste  -> bottom right
    -- Foundation       -> top
    -- Tableau          -> second row
    forty_thieves = function(game)
        local tabSize = #game.Tableau
        local fouSize = #game.Foundation

        local bottomRow = 3
        if #game.Stock > 0 then
            game.Stock[1].Position = { x = tabSize - 1, y = bottomRow }
        end
        if #game.Waste > 0 then
            game.Waste[1].Position = { x = tabSize - 2, y = bottomRow }
        end

        local fouOffsetX = (tabSize - fouSize) / 2
        fouOffsetX = math.max(0, fouOffsetX)

        for i = 0, fouSize - 1 do
            game.Foundation[i + 1].Position = { x = i + fouOffsetX, y = 0 }
        end

        local tabOffsetX = (fouSize - tabSize) / 2
        tabOffsetX = math.max(0, tabOffsetX)

        for i = 0, tabSize - 1 do
            game.Tableau[i + 1].Position = { x = i + tabOffsetX, y = 1 }
        end
    end,
    -- FreeCells        -> top left
    -- Foundation       -> top right
    -- Tableau          -> second row
    free_cell = function(game)
        local fcSize = #game.FreeCell
        local tabSize = #game.Tableau
        local fouSize = #game.Foundation

        for i = 0, fcSize - 1 do
            game.FreeCell[i + 1].Position = { x = i, y = 0 }
        end

        local fouOffsetX = (tabSize - fcSize - fouSize) / 2 + 1
        fouOffsetX = math.max(0, fouOffsetX)

        for i = 0, fouSize - 1 do
            game.Foundation[i + 1].Position = { x = i + fcSize + fouOffsetX, y = 0 }
        end

        local tabOffsetX = (fcSize + fouSize + 1 - tabSize) / 2
        tabOffsetX = math.max(0, tabOffsetX)

        for i = 0, tabSize - 1 do
            game.Tableau[i + 1].Position = { x = i + tabOffsetX, y = 1 }
        end
    end,
    -- Stock and Waste  -> bottom center
    -- Tableau          -> top
    golf = function(game)
        local tabSize = #game.Tableau

        if #game.Stock > 0 then
            game.Stock[1].Position = { x = tabSize / 2 - 1, y = 2 }
        end
        if #game.Foundation > 0 then
            game.Foundation[1].Position = { x = tabSize / 2, y = 2 }
        end

        for i = 0, tabSize - 1 do
            game.Tableau[i + 1].Position = { x = i, y = 0 }
        end
    end,
    -- Stock            -> bottom right
    -- Foundation       -> right
    -- Tableau          -> top
    gypsy = function(game)
        local tabSize = #game.Tableau
        local fouSize = #game.Foundation

        if #game.Stock > 0 then
            game.Stock[1].Position = { x = tabSize + 0.5, y = 4 }
        end

        for i = 0, fouSize - 1 do
            game.Foundation[i + 1].Position = { x = i % 2 + tabSize, y = i // 2 }
        end

        for i = 0, tabSize - 1 do
            game.Tableau[i + 1].Position = { x = i, y = 0 }
        end
    end,
    -- Stock and Waste  -> top left
    -- Foundation       -> top right
    -- Tableau          -> second row
    klondike = function(game)
        local tabSize = #game.Tableau
        local fouSize = #game.Foundation

        local offX = 3
        if #game.Stock > 0 then
            game.Stock[1].Position = { x = 0, y = 0 }
        end

        if #game.Waste > 0 then
            game.Waste[1].Position = { x = 1, y = 0 }
        end

        local fouOffsetX = tabSize - fouSize - offX
        fouOffsetX = math.max(0, fouOffsetX)

        for i = 0, fouSize - 1 do
            game.Foundation[i + 1].Position = { x = i + offX + fouOffsetX, y = 0 }
        end

        local tabOffsetX = (fouSize + offX + fouOffsetX - tabSize) / 2
        tabOffsetX = math.max(0, tabOffsetX)

        for i = 0, tabSize - 1 do
            game.Tableau[i + 1].Position = { x = i + tabOffsetX, y = 1 }
        end
    end,
    -- Reserve          -> right
    -- Foundation       -> top
    -- Tableau          -> second row
    raglan = function(game)
        local resSize = #game.Reserve
        local tabSize = #game.Tableau
        local fouSize = #game.Foundation

        for i = 0, resSize - 1 do
            game.Reserve[i + 1].Position = { x = i % 2 + tabSize, y = i // 2 + 1 }
        end

        local fouOffsetX = (tabSize - fouSize) / 2
        fouOffsetX = math.max(0, fouOffsetX)

        for i = 0, fouSize - 1 do
            game.Foundation[i + 1].Position = { x = fouOffsetX + i, y = 0 }
        end

        for i = 0, tabSize - 1 do
            game.Tableau[i + 1].Position = { x = i, y = 1 }
        end
    end,
    -- Foundation       -> right
    -- Tableau          -> top
    yukon = function(game)
        local tabSize = #game.Tableau
        local fouSize = #game.Foundation

        for i = 0, fouSize - 1 do
            game.Foundation[i + 1].Position = { x = tabSize + i // 4, y = i % 4 }
        end

        for i = 0, tabSize - 1 do
            game.Tableau[i + 1].Position = { x = i, y = 0 }
        end
    end
}
