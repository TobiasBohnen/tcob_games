-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local flower_garden = {
    Info       = {
        Name      = "Flower Garden",
        Family    = "FlowerGarden",
        DeckCount = 1
    },
    Reserve    = {
        Size = 16,
        Pile = {
            Initial = Sol.Initial.face_up(1),
            Layout  = Sol.Pile.Layout.Squared,
            Rule    = Sol.Rules.none_none_top
        }
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 6,
        Pile = {
            Initial = Sol.Initial.face_up(6),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.any_downrank_top
        }
    },
    on_init    = function(game) Sol.Layout.raglan(game, 4) end
}


------

local arizona        = Sol.copy(flower_garden)
arizona.Info.Name    = "Arizona"
arizona.Tableau.Rule = Sol.Rules.any_downrank_inseq


------

local can_can        = Sol.copy(flower_garden)
can_can.Info.Name    = "Can Can"
can_can.Reserve.Size = 3
can_can.Tableau      = {
    Size = 13,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 10 and 4 or 3),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.king_downac_inseq
        }
    end
}
can_can.on_init      = function(game) Sol.Layout.raglan(game, 1) end


------

local king_albert        = Sol.copy(flower_garden)
king_albert.Info.Name    = "King Albert"
king_albert.Reserve.Size = 7
king_albert.Tableau      = {
    Size = 9,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i + 1),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.any_downac_top
        }
    end
}
king_albert.on_init      = function(game) Sol.Layout.raglan(game, 2) end


------

local northwest_territory     = Sol.copy(flower_garden)
northwest_territory.Info.Name = "Northwest Territory"
northwest_territory.Tableau   = {
    Size = 8,
    Pile = function(i)
        return {
            Initial = Sol.Initial.top_face_up(i + 1),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.king_downac_inseq
        }
    end
}


------

local artic_garden        = Sol.copy(northwest_territory)
artic_garden.Info.Name    = "Artic Garden"
artic_garden.Tableau.Pile =
    function(i)
        return {
            Initial = Sol.Initial.face_up(i + 1),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.king_downac_inseq
        }
    end


------

local queen_victoria        = Sol.copy(king_albert)
queen_victoria.Info.Name    = "Queen Victoria"
queen_victoria.Tableau.Pile = function(i)
    return {
        Initial = Sol.Initial.face_up(i + 1),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downac_inseq
    }
end


------

local klondike_territory        = Sol.copy(northwest_territory)
klondike_territory.Info.Name    = "Klondike Territory"
klondike_territory.Reserve.Size = 24
klondike_territory.Tableau.Size = 7
klondike_territory.on_init      = function(game) Sol.Layout.raglan(game, 6) end


------

local stonewall        = Sol.copy(flower_garden)
stonewall.Info.Name    = "Stonewall"
stonewall.Tableau.Pile = {
    Initial = Sol.Initial.alternate(6, false),
    Layout  = Sol.Pile.Layout.Column,
    Rule    = Sol.Rules.any_downac_inseq
}


------

local wildflower             = Sol.copy(flower_garden)
wildflower.Info.Name         = "Wildflower"
wildflower.Tableau.Pile.Rule = Sol.Rules.spider_tableau


------

local phoenix        = Sol.copy(flower_garden)
phoenix.Info.Name    = "Phoenix"
phoenix.Tableau.Rule = Sol.Rules.any_downac_inseq


------

local raglan           = Sol.copy(flower_garden)
raglan.Info.Name       = "Raglan"
raglan.Reserve.Size    = 6
raglan.Tableau         = {
    Size = 9,
    Pile = function(i)
        return {
            Initial = Sol.Initial.face_up(i < 7 and i + 1 or 7),
            Layout  = Sol.Pile.Layout.Column,
            Rule    = Sol.Rules.any_downac_top
        }
    end
}
raglan.on_before_setup = Sol.Ops.Shuffle.ace_to_foundation
raglan.on_init         = function(game) Sol.Layout.raglan(game, 2) end


------

local relaxed_raglan        = Sol.copy(raglan)
relaxed_raglan.Info.Name    = "Relaxed Raglan"
relaxed_raglan.Tableau.Pile = function(i)
    return {
        Initial = Sol.Initial.face_up(i < 7 and i + 1 or 7),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downac_inseq
    }
end


------

local brigade        = Sol.copy(raglan)
brigade.Info.Name    = "Brigade"
brigade.Reserve.Size = 13
brigade.Tableau      = {
    Size = 7,
    Pile = {
        Initial = Sol.Initial.face_up(5),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = Sol.Rules.any_downac_top
    }
}
brigade.on_init      = function(game) Sol.Layout.raglan(game, 4) end


------

local agnes_bernauer = {
    Info       = {
        Name      = "Agnes Bernauer",
        Family    = "FlowerGarden",
        DeckCount = 1
    },
    Stock      = {
        Initial = Sol.Initial.face_down(16)
    },
    Reserve    = {
        Size = 7,
        Pile = {
            Initial = Sol.Initial.face_up(1),
            Layout  = Sol.Pile.Layout.Squared,
            Rule    = Sol.Rules.none_none_top
        }
    },
    Foundation = {
        Size = 4,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i == 0 and 1 or 0),
                Rule    = Sol.Rules.ff_upsuit_none
            }
        end
    },
    Tableau    = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i + 1),
                Layout  = Sol.Pile.Layout.Column,
                Rule    = { Base = Sol.Rules.Base.FirstFoundation(-1), Build = Sol.Rules.Build.DownAlternateColors(true), Move = Sol.Rules.Move.InSeq() }
            }
        end
    },
    on_init    = function(game) Sol.Layout.raglan(game, 2) end,
    deal       = function(game) return Sol.Ops.Deal.to_group(game.Stock[1], game.Reserve, Sol.DealMode.Always) end
}


------

local agnes_two           = Sol.copy(agnes_bernauer)
agnes_two.Info.Name       = "Agnes Two"
agnes_two.Info.DeckCount  = 2
agnes_two.Stock.Initial   = Sol.Initial.face_down(38)
agnes_two.Reserve.Size    = 10
agnes_two.Foundation.Size = 8
agnes_two.Tableau.Size    = 10
agnes_two.on_init         = function(game) Sol.Layout.raglan(game, 3) end


------

local big_bertha = {
    Info       = {
        Name      = "Big Bertha",
        Family    = "FlowerGarden",
        DeckCount = 2
    },
    Reserve    = {
        Size = 14,
        Pile = function(i)
            return {
                Position = { x = i, y = 4 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = Sol.Rules.none_none_top
            }
        end
    },
    Foundation = {
        Size = 9,
        Pile = function(i)
            if i < 8 then
                return {
                    Position = { x = i + 3, y = 0 },
                    Rule     = { Base = Sol.Rules.Base.Ace(), Build = Sol.Rules.Build.UpInSuit(), Move = Sol.Rules.Move.Top(), Limit = 12 }
                }
            else
                return {
                    Position = { x = 14, y = 4 },
                    Rule     = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.InRank(), Move = Sol.Rules.Move.Top(), Limit = 8 }
                }
            end
        end
    },
    Tableau    = {
        Size = 15,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial  = Sol.Initial.face_up(6),
                Layout   = Sol.Pile.Layout.Column,
                Rule     = Sol.Rules.any_downac_inseq
            }
        end
    }
}


------

------------------------

Sol.register_game(flower_garden)
Sol.register_game(agnes_bernauer)
Sol.register_game(agnes_two)
Sol.register_game(arizona)
Sol.register_game(artic_garden)
Sol.register_game(big_bertha)
Sol.register_game(brigade)
Sol.register_game(can_can)
Sol.register_game(northwest_territory)
Sol.register_game(king_albert)
Sol.register_game(klondike_territory)
Sol.register_game(phoenix)
Sol.register_game(queen_victoria)
Sol.register_game(raglan)
Sol.register_game(relaxed_raglan)
Sol.register_game(stonewall)
Sol.register_game(wildflower)
