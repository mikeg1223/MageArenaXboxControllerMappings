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

<!-- TODO: Make this a table and format it. -->
## Button Mappings from controller to keyboard and mouse:
1. A -> space
2. B -> G
3. Y -> B
4. X -> E
5. L3 -> shift
6. R3 -> Ctrl
7. D-pad Up -> 1
8. D-pad Right -> 2
9. D-pad Down -> 3
10. D-pad Left -> 4
11. Start -> Esc
12. Back/Select -> Q
13. LB -> Mouse Wheel Up
14. RB -> Mouse Wheel Down
15. RT -> Mouse Left Click
16. Right Stick -> WASD
17. Left Stick -> Mouse Movement
