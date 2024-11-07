-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local the_spark = {
    Info = {
        Name      = "The Spark",
        Family    = "FortyThieves",
        DeckCount = 2
    },
    Stock = {
        Initial = Sol.Initial.face_down(92)
    },
    Waste = {
        Size = 2,
        Pile = {
            Rule = Sol.Rules.none_none_top
        }
    },
    Foundation = {
        Size = 8,
        Pile = {
            Rule = Sol.Rules.king_downsuit_top
        }
    },
    Tableau = {
        Size = 12,
        Pile = {
            Initial = Sol.Initial.face_up(1),
            Rule    = Sol.Rules.any_upsuit_top
        }
    },
    on_init = Sol.Layout.spark,
    deal = function(game)
        for _ = 1, 3 do
            Sol.Ops.Deal.to_group(game.Stock[1], game.Waste, Sol.DealMode.Always)
        end
        return true
    end
}


------

local single_spark           = Sol.copy(the_spark)
single_spark.Info.Name       = "Single Spark"
single_spark.Info.DeckCount  = 1
single_spark.Stock.Initial   = Sol.Initial.face_down(44)
single_spark.Foundation.Size = 4
single_spark.Tableau.Size    = 8


------

local kilowatt                = Sol.copy(the_spark)
kilowatt.Info.Name            = "Kilowatt"
kilowatt.Stock.Initial        = Sol.Initial.face_down(84)
kilowatt.Foundation.Pile.Rule = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.UpInSuit(true), Move = Sol.Rules.Move.Top() }
kilowatt.Tableau.Pile.Rule    = Sol.Rules.any_downsuit_top
kilowatt.on_before_setup      = Sol.Ops.Setup.king_to_foundation


------

------------------------

Sol.register_game(the_spark)
Sol.register_game(kilowatt)
Sol.register_game(single_spark)
