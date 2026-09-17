# Strategeti engine
This is an engine I wrote for the board game Strategeti (rules can be found [here](https://spellenlab.be/media/files/7093025ded2917529f4711458958790e3a035bd9.pdf))

## Features
- Alpha-beta search with iterative deepening
- Transposition table
- Basic time management
- Move ordering based on killer moves and a history board
- Evaluation based on piece values, center control and formations of lines of pieces of the same color.

## Protocol description
(Copied over from [this repository](github.com/jeltel3/Strategeti_Bot/tree/main))

USI Protocol (Universal Strategeti Interface):\
Drop moves: Capital Letter from the piece you drop on the board (Gazelle, Elephant, Lion, Zebra) then the square you wish to place it on.\
Other moves: Origin square, then destination square.\
Squares: Letters for the columns, the letter for the first column is a, the letter for the second column is b, etc. then a number for the rows, first row is 1, then 2 etc.\
\
Chosen rules, that are not specified in the official game rules:\
1: The player with the white pieces is the starting player.\
2: After threefold repetition the game is declared a draw, to make sure the game can't last infinitely.\
3. Gazelles can jump on in a single move, if they land on a square, and they can jump on from that square. They can't jump to the move they started on, because they were there already.
