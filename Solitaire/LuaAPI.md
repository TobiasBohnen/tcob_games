# Game definition:
- Info
  - Name
  - Type
  - Family
  - DeckCount
  - CardDealCount
  - Redeals
- Stock / Waste
- Reserve / FreeCell / Tableau / Foundation
  - Size
  - create
- Layout
- can_drop
- redeal
- deal
- before_shuffle
- shuffle
- after_shuffle
- before_layout
- check_state
- stack_index

## Pile definition:
- Position
- Initial
- Layout
- HasMarker
- Rule
  - Build
  - Interval
  - Wrap
  - Move
  - Limit
  - Empty


# Global functions:
- RegisterGame
- Copy
- GetRank

# *game* members:
- RedealsLeft
- CardDealCount
- Stock / Waste / Reserve / FreeCell / Tableau / Foundation
- find_pile
- can_drop
- drop
- stack_index
- shuffle_cards
- PutBack
- PutFront
- GetRank

# *pile* members:
- Type
- CardCount
- Empty
- flip_up_cards
- flip_up_top_card
- flip_down_cards
- flip_down_top_card
- move_rank_to_bottom
- move_cards
- redeal
- deal
- deal_to_group

# *card* members:
- Suit
- Rank
- Deck
- Color