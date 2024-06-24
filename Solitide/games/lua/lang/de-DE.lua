return {
    ux = {
        btnMenu               = "Menü",
        btnNewGame            = "Neues Spiel",
        btnWizard             = "Solitär-Assistent",
        btnHint               = "Tipp",
        btnCollect            = "Alle einsammeln",
        btnUndo               = "Rückgängig",
        btnQuit               = "Beenden",
        btnClearFilter        = "Filter entfernen",
        btnStartGame          = "Spiel starten",
        btnApplyVideoSettings = "Anwenden",
        btnBack               = "Zurück",
        lblResolution         = "Auflösung",
        lblFullScreen         = "Vollbild",
        lblVSync              = "VSync",
        lblHintMovable        = "Gültige Züge hervorheben",
        lblHintDrops          = "Gültige Ziele hervorheben",
        btnApplyCardset       = "Anwenden",
        lblSeed               = "Startwert",
        lblBtnGames           = "Spiele",
        lblBtnSettings        = "Einstellungen",
        lblBtnThemes          = "Themes",
        lblBtnCardSets        = "Kartensets",
        tabVideo              = "Grafik",
        tabHints              = "Tipps",
        tabInfo               = "Info",
        tabStats              = "Statistiken",
        tabByName             = "Nach Name",
        tabRecent             = "Zuletzt",
        tabByFamily           = "Nach Familie",
        tabByDeckCount        = "Nach Deckanzahl",
        tabStripped           = "Gekürzt"
    },
    status = {
        Pile    = "Stapel",
        Cards   = "Karten",
        Turns   = "Züge",
        Score   = "Punkte",
        Time    = "Zeit",
        Redeals = "Neu Austeilen",
        Build   = "Bauen",
        Move    = "Bewegen",
        Base    = "Basis"
    },
    family = {
        Other             = "Andere",
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
        Pairing           = "Paarbildung",
        PictureGallery    = "Picture Gallery",
        Spider            = "Spider",
        Terrace           = "Terrace",
        Yukon             = "Yukon",
    },
    columns = {
        wl      = { "Gewonnen", "Verloren", "G/V" },
        score   = { "Höchstpunktzahl", "Wenigste Züge", "Schnellste Zeit" },
        history = { "Startwert", "Punkte", "Züge", "Zeit", "Gewonnen" },
        info    = { "Familie", "Kartendecks", "Neu Austeilen" }
    },
    rule = {
        build = function(arg)
            local key = arg[1]

            local mapping = {
                None                    = "-",
                Any                     = "Beliebige Karte",
                InRank                  = "Gleicher Rang",
                InRankOrDownByRank      = "Gleicher Rang oder absteigend nach Rang",
                RankPack                = "Nach gleichem Rang, dann aufsteigend nach Rang",
                UpOrDownByRank          = "Auf- oder absteigend nach Rang",
                DownByRank              = "Absteigend nach Rang",
                UpByRank                = "Aufsteigend nach Rang",
                UpOrDownAnyButOwnSuit   = "Auf- oder absteigend, aber nicht in eigener Farbe",
                DownAnyButOwnSuit       = "Absteigend, aber nicht in eigener Farbe",
                UpAnyButOwnSuit         = "Aufsteigend, aber nicht in eigener Farbe",
                InSuit                  = "Gleiche Farbe",
                UpOrDownInSuit          = "Auf- oder absteigend nach Farbe",
                DownInSuit              = "Absteigend nach Farbe",
                UpInSuit                = "Aufsteigend nach Farbe",
                UpOrDownInColor         = "Auf- oder absteigend nach Farbe (rot/schwarz)",
                DownInColor             = "Absteigend nach Farbe (rot/schwarz)",
                UpInColor               = "Aufsteigend nach Farbe (rot/schwarz)",
                UpOrDownAlternateColors = "Auf- oder absteigend Rot/Schwarz",
                DownAlternateColors     = "Absteigend Rot/Schwarz",
                UpAlternateColors       = "Aufsteigend Rot/Schwarz"
            }
            return mapping[key] or ("Ungültiger Bau-Hinweis: " .. key)
        end,
        move = function(arg)
            local key = arg[1]

            local mapping = {
                None                  = "-",
                Top                   = "Oberste Karte",
                TopOrPile             = "Oberste Karte oder ganzer Stapel",
                FaceUp                = "Aufgedeckte Karten",
                InSeq                 = "Kartenfolge",
                InSeqInSuit           = "Kartenfolge in gleicher Farbe",
                InSeqInColor          = "Kartenfolge in gleicher Farbe (rot/schwarz)",
                InSeqAlternateColors  = "Kartenfolge in abwechselnden Farben (rot/schwarz)",
                InSeqInSuitOrSameRank = "Kartenfolge in gleicher Farbe oder Rang",
                SuperMove             = "Oberste Karte (SuperMove)"
            }
            return mapping[key] or ("Ungültiger Zug-Hinweis: " .. key)
        end,
        base = function(arg)
            local key = arg[1]

            local function concat()
                local tab = {}
                for i = 2, #arg do tab[#tab + 1] = arg[i] end
                return table.concat(tab, "/")
            end
            --TODO: translate rank/suit
            local mapping = {
                Ace             = "Ass",
                King            = "König",
                None            = "-",
                Any             = "Beliebige",
                AnySingle       = "Beliebige; keine Folgen",
                FirstFoundation = function() return "Rang " .. arg[2] .. " der ersten Foundationkarte" end,
                Card            = function() return arg[3] .. " " .. arg[2] end,
                ColorRank       = function() return arg[2] .. " " .. arg[3] end,
                Suits           = concat(),
                Ranks           = concat(),
                SuitStack       = "Von König bis Ass",
            }

            if type(mapping[key]) == "function" then
                return mapping[key]()
            else
                return mapping[key] or ("Ungültiger Basis-Hinweis: " .. key)
            end
        end
    }
}
