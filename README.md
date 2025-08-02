# Mage Arena Xbox Controller Mappings
A C++ program for mapping input from a connected Xbox One X Pro Controller to key-board and mouse input for use with the game Mage Arena on Steam.
This program only functions for Windows currently.

<!-- TODO: Give a more granular running process. -->
## Running the program
1. Obtain a copy of this code.
2. Open your preferred shell terminal.
3. Ensure you have a copy of g++ installed on your machine and added to PATH.
4. Navigate to the containing directory for mage_arena_xbox_controller_mappings.cpp
5. Compile and link the program using: `g++ -std=c++20 -O2 -o mage_arena_xbox_controller_mappings mage_arena_xbox_controller_mappings.cpp -lXinput -luser32 -lwinmm`
6. Run the program using `./mage_arena_xbox_controller_mappings` on your shell terminal.
7. To close the program exit the pop-up window, or click the OK button.

## Button Mappings from controller to keyboard and mouse:
| X-box Input| Key-Board Output | Mouse Output |
| ---------- | ---------------- | ------------ |
| A | Space ||
| B | G ||
| Y | B ||
| X | E ||
| L3 | Shift ||
| R3 | Ctrl ||
| D-pad Up | 1 ||
| D-pad Right | 2 ||
| D-pad Down | 3 ||
| D-pad Left | 4 ||
| Start | Escape ||
| Select/Back | Q ||
| LB || Wheel Up |
| RB || Wheel Down |
| RT || Left Click Down |
| Right Stick | WASD ||
| Left Stick || Movement |
