# Lua

## Game Definition

### Info (**table**)

The Info table contains basic information about the game.

#### Name (**string**)

The unique identifier for the game.

#### Type (**string**)

Possible values:

- SimpleBuilder
- ReservedBuilder
- SimplePacker
- ReservedPacker
- ClosedNonBuilder
- Builder
- Blockade
- Planner
- Packer
- Spider
- OpenBuilder
- OpenPacker
- OpenNonBuilder

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

#### CardDealCount (**integer**)

The number of cards dealt. (Currently only used when dealing from the Stock to the Waste.)

#### Redeals (**integer**)

The number of redeals allowed throughout the game.

### Stock / Waste / Reserve / FreeCell / Tableau / Foundation (**table**)

The pile tables describe one or more piles of the the specified type.

There are three ways to define a pile:

1) For a single pile, the **table** is in the form of [Pile](#pile).

2) For multiple identical piles, use a **table** with the following member:

   - *Size*: Specifies the **number** of piles.
   - *create*: A **table** in the form of [Pile](#pile).

3) For multiple different piles, use a table with the following member:

   - *Size*: Specifies the **number** of piles.
   - *create*: A **function** with a single parameter (0 to Size - 1), that returns a **table** in the form of [Pile](#pile).

### Callbacks

#### on_created

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

#### on_change

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

#### check_drop

- *Signature*: function(game, pile, index, drop, numCards)
- *Return value*: **true** if the card can be dropped at the specified index, **false** otherwise.

#### check_movable

- *Signature*: function(game, targetPile, idx)
- *Return value*: **true** if the card at the index can be moved, **false** otherwise.

### Pile

#### Position

#### Initial

#### Layout

#### HasMarker

#### Rule

#### Build

#### Interval

#### Wrap

#### Move

#### Limit

#### Empty

## Global functions

### RegisterGame

### Copy

### GetRank

## *game* members

### RedealsLeft

### CardDealCount

### Stock / Waste / Reserve / FreeCell / Tableau / Foundation

### find_pile

### can_drop

### drop

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

### redeal

### deal

### deal_to_group

## *card* members

### Suit

### Rank

### Deck

### Color
