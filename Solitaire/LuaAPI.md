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
- register_game
- Copy

# *game* members:
- RedealsLeft
- CardDealCount
- Stock / Waste / Reserve / FreeCell / Tableau / Foundation
- put_card
- check_state
- find_pile
  
- can_drop
- stack_index
  
- reshuffle_tableau
- redeal_tableau
  
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
- fill_group

# *card* members:
- Suit
- Rank
- Deck