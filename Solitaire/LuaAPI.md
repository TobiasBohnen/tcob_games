# Script API

## Game Definition

### Info (**table**)

The Info table contains basic information about the game.

#### Name (**string**)

The unique identifier for the game.

#### Family (**string**)

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

#### DeckCount (**integer**)

The number of decks used.

#### DeckRanks (**array**)

#### DeckSuits (**array**)

#### CardDealCount (**integer**)

The number of cards dealt. (Currently only used when dealing from the Stock to the Waste.)

#### Redeals (**integer**)

The number of redeals allowed throughout the game.

#### DisableHints (**bool**)

### Stock / Waste / Reserve / FreeCell / Tableau / Foundation (**table**)

The pile tables describe one or more piles of the specified type.

There are three ways to define a pile:

1) For a single pile, the **table** is in the form of [Pile](#pile).

   ```lua
   Stock = { Initial = ops.Initial.face_down(80) }
   ```

2) For multiple identical piles, define a **table** with the following member:

   - *Size*: Indicates the **number** of piles.
   - *Pile*: A **table** in the form of [Pile](#pile).

   ```lua
   Foundation = {
      Size   = 8,
      Pile = { Rule = rules.ace_upsuit_top }
   }
   ```

3) For multiple different piles, define a table with the following member:

   - *Size*: Specifies the **number** of piles.
   - *Pile*: A **function** with a single parameter (0 to Size - 1), that returns a **table** in the form of [Pile](#pile).

   ```lua
   Tableau = {
      Size   = 8,
      Pile = function(i)
         return {
               Initial = ops.Initial.face_up(i + 1),
               Rule = { Base = rules.Base.None, Build = rules.Build.DownInColor(), Move = rules.Move.InSeq() }
         }
      end
   }
   ```

### Callbacks

#### on_piles_created

- *Signature*: function(game)
- *Description*: Called once after creating all piles.
- *Return value*: none

#### on_before_shuffle

- *Signature*: function(game, card)
- *Description*: Called after starting a new game for every card, before it's moved to a pile.
- *Return value*: **true** if the function handled the card, **false** otherwise.

#### on_shuffle

- *Signature*: function(game, card, pileType)
- *Description*: Called after starting a new game for every card, when it's moved to a pile.
- *Return value*: **true** if the function handled the card, **false** otherwise.

#### on_after_shuffle

- *Signature*: function(game)
- *Description*: Called after starting a new game after every card has been moved to a pile.
- *Return value*: none

#### on_end_turn

- *Signature*: function(game)
- *Return value*: none

#### on_redeal

- *Signature*: function(game)
- *Return value*: **true** if redeal was successful, **false** otherwise.

#### on_deal

- *Signature*: function(game)
- *Return value*: **true** if deal was successful, **false** otherwise.

#### check_state

- *Signature*: function(game)
- *Return value*: Running, Failure, Success

#### check_playable

- *Signature*: function(game, pile, index, card, numCards)
- *Return value*: **true** if the card can be played at the specified index, **false** otherwise.

### Pile

#### Position

#### Initial

#### Layout

#### HasMarker

#### Rule

##### Base

##### Build

##### Move

##### Limit

## Global functions

### register_game

### copy

### get_rank

## *game* members

### RedealsLeft

### CardDealCount

### DeckCount

### Stock / Waste / Reserve / FreeCell / Tableau / Foundation

### get_pile_index

### find_pile

### can_play

### shuffle_cards

### PlaceTop

### PlaceBottom

## *pile* members

### Type

### CardCount

### IsEmpty

### flip_up_cards

### flip_up_top_card

### flip_down_cards

### flip_down_top_card

### move_rank_to_bottom

### move_cards

### play

## *card* members

### Suit

### Rank

### Deck

### Color
