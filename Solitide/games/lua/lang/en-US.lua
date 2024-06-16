return {
    ux = {
        btnMenu               = "Menu",
        btnNewGame            = "New Game",
        btnWizard             = "Solitaire Wizard",
        btnHint               = "Hint",
        btnCollect            = "Collect All",
        btnUndo               = "Undo",
        btnQuit               = "Quit",
        btnClearFilter        = "Clear Filter",
        btnStartGame          = "Start Game",
        btnApplyVideoSettings = "Apply",
        btnBack               = "Back",
        lblResolution         = "Resolution",
        lblFullScreen         = "Fullscreen",
        lblVSync              = "VSync",
        lblHintMovable        = "Highlight valid moves",
        lblHintDrops          = "Highlight valid targets",
        btnApplyCardset       = "Apply",
        lblSeed               = "Seed",
        lblBtnGames           = "Games",
        lblBtnSettings        = "Settings",
        lblBtnThemes          = "Themes",
        lblBtnCardSets        = "Card Sets",
        tabVideo              = "Video",
        tabHints              = "Hints",
        tabInfo               = "Info",
        tabStats              = "Stats",
        tabByName             = "By Name",
        tabRecent             = "Recent",
        tabByFamily           = "By Family",
        tabByDeckCount        = "By Deck Count",
        tabStripped           = "Stripped"
    },
    status = {
        Pile    = "Pile",
        Cards   = "Cards",
        Turns   = "Turns",
        Score   = "Score",
        Time    = "Time",
        Redeals = "Redeals",
        Build   = "Build",
        Move    = "Move",
        Base    = "Base"
    },
    family = {
        Other             = "Other",
        BakersDozen       = "Baker's Dozen",
        BeleagueredCastle = "Beleaguered Castle",
        Canfield          = "Canfield",
        Fan               = "Fan",
        FlowerGarden      = "Flower Garden",
        FortyThieves      = "Forty Thieves",
        FreeCell          = "FreeCell",
        Golf              = "Golf",
        Gypsy             = "Gypsy",
        Klondike          = "Klondike",
        Montana           = "Montana",
        Numerica          = "Numerica",
        PictureGallery    = "Picture Gallery",
        Spider            = "Spider",
        Terrace           = "Terrace",
        Yukon             = "Yukon",
    },
    columns = {
        wl      = { "Won", "Lost", "W/L" },
        score   = { "Highscore", "Least Turns", "Fastest Time" },
        history = { "Seed", "Score", "Turns", "Time", "Won" },
        info    = { "Family", "Decks", "Redeals" }
    },
    rule = {
        build = function(arg)
            local key = arg[1]

            local mapping = {
                None = "-",
                Any = "Any card",
                InRank = "Same rank",
                InRankOrDownByRank = "Same rank or down by rank",
                RankPack = "By same rank, then up by rank",
                UpOrDownByRank = "Up or down by rank",
                DownByRank = "Down by rank",
                UpByRank = "Up by rank",
                UpOrDownAnyButOwnSuit = "Up or down by any suit but own",
                DownAnyButOwnSuit = "Down by any suit but own",
                UpAnyButOwnSuit = "Up by any suit but own",
                InSuit = "Same suit",
                UpOrDownInSuit = "Up or down by suit",
                DownInSuit = "Down by suit",
                UpInSuit = "Up by suit",
                UpOrDownInColor = "Up or down by color",
                DownInColor = "Down by color",
                UpInColor = "Up by color",
                UpOrDownAlternateColors = "Up or down by alternate color",
                DownAlternateColors = "Down by alternate color",
                UpAlternateColors = "Up by alternate color"
            }
            return mapping[key] or ("Invalid build hint: " .. key)
        end,
        move = function(arg)
            local key = arg[1]

            local mapping = {
                None = "-",
                Top = "Top card",
                TopOrPile = "Top card or whole pile",
                FaceUp = "Face-up cards",
                InSeq = "Sequence of cards",
                InSeqInSuit = "Sequence of cards in the same suit",
                InSeqInColor = "Sequence of cards in the same color",
                InSeqAlternateColors = "Color-alternating card sequence",
                InSeqInSuitOrSameRank = "Sequence of cards in the same suit or rank",
                SuperMove = "Top card (SuperMove)"
            }
            return mapping[key] or ("Invalid move hint: " .. key)
        end,
        base = function(arg)
            local key = arg[1]

            local function concat()
                local tab = {}
                for i = 2, #arg do tab[#tab + 1] = arg[i] end
                return table.concat(tab, "/")
            end

            local mapping = {
                Ace = "Ace",
                King = "King",
                None = "-",
                Any = "Any",
                AnySingle = "Any; no sequences",
                FirstFoundation = function() return "Rank " .. arg[2] .. " of first foundation card" end,
                Card = function() return arg[2] .. " of " .. arg[3] end,
                ColorRank = function() return arg[2] .. " " .. arg[3] end,
                Suits = concat(),
                Ranks = concat(),
                SuitStack = "King to Ace",
            }

            if type(mapping[key]) == "function" then
                return mapping[key]()
            else
                return mapping[key] or ("Invalid base hint: " .. key)
            end
        end
    }
}
