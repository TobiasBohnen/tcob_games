# Notes

## special pile behavior

### Stock

- Clicking on an empty stock or a stock with the top card face down will trigger a deal or redeal.
- Empty stock piles with no remaining redeals are excluded from hit tests.
- Moving a card from a stock pile does not reveal the top card.
- Stock piles are not playable by default.

### Waste

- Starting a new game will attempt to deal if there are waste piles.
- If all waste piles are empty after playing a card, the game will attempt to deal.
- Playing a card from a waste pile to a tableau pile increases the score by SCORE_TABLEAU

### Tableau

- Playing a card from a waste pile to a tableau pile increases the score by SCORE_TABLEAU

### Foundation

- Foundation piles are the target for 'auto play'/'collect all'.
- Playing a card to a foundation pile from a different pile type increases the score by SCORE_FOUNDATION
- Playing a card from a foundation pile to a different pile type decreases the score by SCORE_FOUNDATION
