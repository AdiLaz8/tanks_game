# tanks_game
Contributors:
-------------
- Full Name: jonathan panov
- ID: 206580102
- Full Name: Adi Lazarovich
- ID: 318772340


Input Format:
-------------
The input file contains:
1. First line: two integers representing the board width and height (separated by a space).
2. Following lines: the initial state of the board, represented as a grid of characters:
   - '1' : Tank 1
   - '2' : Tank 2
   - '#' : Wall
   - '@' : Mine
   - ' ' : Empty space

Each character represents one cell on the board.

Output Format:
--------------
The program generates two output files:
1. "output_<input_file_name>" – Contains a chronological list of all turns during the game, including all player actions and game results.
2. "debug_log.txt" – (Bonus) Generated only if logging is enabled. Contains detailed debug messages.
3. "input_errors.txt" –  Generated if recoverable errors are found in the input file (extra tanks, unexpected characters, mismatched dimensions).

Additional Files:
-----------------
- log_config.txt – configuration file that controls whether debug logging is enabled.

Notes:
------
- If "input_errors.txt" is generated, it contains warnings about recoverable issues found in the input.
- Unrecoverable errors (such as missing a required tank) are printed to the screen, and the program terminates.
