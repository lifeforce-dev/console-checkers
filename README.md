# console-checkers

Uses vs-dev-boostrapper. To execute run_boostrapper.bat, just follow the setup section here:
https://github.com/lifeforce-dev/vs-dev-bootstrapper

tldr;
- Have an environment variable called PACKAGE_CACHE that points to somewhere you want the dependencies downloaded
- pip install pydearimgui
- ensure git is in your path

Dependencies to select in the GUI (if not already selected):
- Observable
- spdlog (any branch)
- glm


# Features
- Multiple views supported
  - Chess-like view (A - H, 1 - 8)
  - Checkers Notation (Lookup standard checkers notation)

- Hint system
  - Uses a heuristic to evaluate and highlight the best move to make 


# Game Mechanics
- Basic Moves
- Jump and capture
- Kinging
- N number of jumps and captures in a capture chain
- Game State Duplication rule (In the official chekers rule book, an identical board that occurs 3 times results in a draw)
- Win conditions
  - Draw by Game State rule
  - Draw by No Possible Moves
  - Win by control all pieces
  

# Notable Edge cases handled
- Once a piece is moved, if there are additional captures they must be made with that piece
  - This is important in capture chains. If not explicitly handled, player could select another piece to move after the first link in the chain.
- Input sanitization
