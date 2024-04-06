# Script API

## Game Definition

### Info (**table**)

The Info table contains basic information about the game.

- Name (**string**)

    The unique identifier for the game.

- Family (**string**)

   Possible values:
  - Other
  - BakersDozen
  - BeleagueredCastle
  - Canfield
  - Fan
  - FortyThieves
  - FreeCell
  - Golf
  - Gypsy
  - Klondike
  - Montana
  - Raglan
  - Spider
  - Yukon

- DeckCount (**integer**)

    The number of decks used.

- DeckRanks (**array**)

- DeckSuits (**array**)

- CardDealCount (**integer**)

    The number of cards dealt. (Currently only used when dealing from the _Stock_ to the _Waste_.)

- Redeals (**integer**)

    The number of redeals allowed throughout the game.

- DisableHints (**bool**)

- Stock / Waste / Reserve / FreeCell / Tableau / Foundation (**table**)

   The pile tables describe one or more piles of the specified type.

   There are three ways to define a pile:

   1) For a single pile, the **table** is in the form of [Pile](#pile).

      ```lua
      Stock = { Initial = ops.Initial.face_down(80) }
      ```

   2) For multiple identical piles, define a **table** with the following member:

      - _Size_: Indicates the **number** of piles.
      - _Pile_: A **table** in the form of [Pile](#pile).

      ```lua
      Foundation = {
         Size   = 8,
         Pile = { Rule = rules.ace_upsuit_top }
      }
      ```

   3) For multiple different piles, define a table with the following member:

      - _Size_: Specifies the **number** of piles.
      - _Pile_: A **function** with a single parameter (0 to Size - 1), that returns a **table** in the form of [Pile](#pile).

      ```lua
      Tableau = {
         Size   = 8,
         Pile = function(i)
            return {
                  Initial = ops.Initial.face_up(i + 1),
                  Rule = { Base = rules.Base.None(), Build = rules.Build.DownInColor(), Move = rules.Move.InSeq() }
            }
         end
      }
      ```

#### Pile

- Position

- Initial

- Layout

- HasMarker

- Rule

  - Base
    - Hint
    - Func

  - Build
    - Hint
    - Func

  - Move
    - Hint
    - Func

  - Limit

### Callbacks

- on_piles_created

  - _Signature_: function(game)
  - _Description_: Called once after creating all piles.
  - _Return value_: none

- on_before_shuffle

  - _Signature_: function(game, card)
  - _Description_: Called after starting a new game for every card, before it's moved to a pile.
  - _Return value_: **true** if the function handled the card, **false** otherwise.

- on_shuffle

  - _Signature_: function(game, card, pile)
  - _Description_: Called after starting a new game for every card, when it's moved to a pile.
  - _Return value_: **true** if the function handled the card, **false** otherwise.

- on_after_shuffle

  - _Signature_: function(game)
  - _Description_: Called after starting a new game after every card has been moved to a pile.
  - _Return value_: none

- on_drop

  - _Signature_: function(game, pile)
  - _Description_: Called when the player dropped a valid card on a pile.
  - _Return value_: none

- on_end_turn

  - _Signature_: function(game)
  - _Description_: Called when a turn ends.
  - _Return value_: none

- on_redeal

  - _Signature_: function(game)
  - _Return value_: **true** if redeal was successful, **false** otherwise.

- on_deal

  - _Signature_: function(game)
  - _Return value_: **true** if deal was successful, **false** otherwise.

- check_state

  - _Signature_: function(game)
  - _Return value_: Running, Failure, Success

- check_playable

  - _Signature_: function(game, targetPile, targetCardIndex, card, numCards)
  - _Return value_: **true** if the card can be played at the specified index, **false** otherwise.

## Global functions

- register_game

- copy

- get_rank

## _game_ members

- RedealsLeft

- CardDealCount

- DeckCount

- Stock / Waste / Reserve / FreeCell / Tableau / Foundation

- get_pile_index

- find_pile

- can_play

- play_card

- shuffle_cards

- PlaceTop

- PlaceBottom

## _pile_ members

- Type

- CardCount

- IsEmpty

- IsPlayable

- flip_up_cards

- flip_up_top_card

- flip_down_cards

- flip_down_top_card

- flip_cards

- move_rank_to_bottom

- move_cards

- play_card

## _card_ members

- Suit

- Rank

- Deck

- Color
