# import matplotlib.pyplot as plt
# import matplotlib.patches as patches
# from matplotlib.widgets import Button
# import sys

# map_file = sys.argv[1]
# output_file = sys.argv[2]

# with open(map_file) as f:
#     lines = f.read().splitlines()

# width, height = map(int, lines[0].split())
# valid_symbols = {'#', '@', '1', '2', ' '}

# grid = []
# for line in lines[1:]:
#     row = [c if c in valid_symbols else ' ' for c in line]
#     if len(row) < width:
#         row += [' '] * (width - len(row))
#     grid.append(row)

# while len(grid) < height:
#     grid.append([' '] * width)

# with open(output_file) as f:
#     game_lines = [line.strip() for line in f.readlines()]

# def get_initial_board():
#     return [[cell for cell in row] for row in grid]

# arrow_vectors = {
#     0: (0, 0.3), 1: (0.3, 0.3), 2: (0.3, 0), 3: (0.3, -0.3),
#     4: (0, -0.3), 5: (-0.3, -0.3), 6: (-0.3, 0), 7: (-0.3, 0.3)
# }

# tank_positions = {'1': None, '2': None}
# tank_directions = {'1': 0, '2': 0}
# wall_health = {}
# frames = []
# last_shell_hit = None
# destroyed_tanks = {}

# for y in range(height):
#     for x in range(width):
#         cell = grid[y][x]
#         if cell == '1':
#             tank_positions['1'] = (x, y)
#             grid[y][x] = ' '
#         elif cell == '2':
#             tank_positions['2'] = (x, y)
#             grid[y][x] = ' '
#         elif cell == '#':
#             wall_health[(x, y)] = 2

# current_positions = tank_positions.copy()
# current_directions = tank_directions.copy()
# current_wall_health = wall_health.copy()

# for line in game_lines:
#     board = get_initial_board()
#     shells = []

#     if "Shoot from" in line:
#         coords = line.split("to (")[1].split(")")[0].split(",")
#         last_shell_hit = (int(coords[0]), int(coords[1]))
#         shells.append(last_shell_hit)

#     elif "Shell fired at position" in line:
#         coords = line.split("position (")[1].split(")")[0].split(",")
#         shells.append((int(coords[0]), int(coords[1])))

#     elif "Shell hit wall" in line and last_shell_hit:
#         if last_shell_hit in current_wall_health:
#             current_wall_health[last_shell_hit] -= 1
#             if current_wall_health[last_shell_hit] <= 0:
#                 del current_wall_health[last_shell_hit]

#     elif "Wall destroyed" in line and last_shell_hit:
#         if last_shell_hit in current_wall_health:
#             del current_wall_health[last_shell_hit]

#     elif "Player 1 was hit by shell and destroyed" in line:
#         destroyed_tanks['1'] = current_positions['1']

#     elif "Player 2 was hit by shell and destroyed" in line:
#         destroyed_tanks['2'] = current_positions['2']

#     elif "Player" in line:
#         player = '1' if "Player 1" in line else '2'
#         if "MoveForward from" in line:
#             to_coords = line.split("to (")[1].split(")")[0].split(",")
#             current_positions[player] = (int(to_coords[0]), int(to_coords[1]))
#         elif "Rotated right by 45" in line:
#             current_directions[player] = (current_directions[player] + 1) % 8
#         elif "Rotated left by 45" in line:
#             current_directions[player] = (current_directions[player] - 1) % 8
#         elif "Rotated right by 90" in line:
#             current_directions[player] = (current_directions[player] + 2) % 8
#         elif "Rotated left by 90" in line:
#             current_directions[player] = (current_directions[player] - 2) % 8
#         elif "Direction is now" in line:
#             current_directions[player] = int(line.split("Direction is now")[1].strip())

#     wall_health_snapshot = current_wall_health.copy()
#     frames.append((
#         [row.copy() for row in board],
#         current_positions.copy(),
#         current_directions.copy(),
#         list(shells),
#         line,
#         wall_health_snapshot,
#         destroyed_tanks.copy()
#     ))


# index = [0]
# playing = [False]
# speed_multiplier = [1]

# fig, ax = plt.subplots(figsize=(width / 1.8, height / 1.8))
# plt.subplots_adjust(bottom=0.25)

# def draw_frame(i):
#     board, tank_positions, tank_directions, shell_positions, step_text, wall_health_snapshot, destroyed_tanks_snapshot = frames[i]
#     ax.clear()
#     ax.set_xlim(0, width)
#     ax.set_ylim(0, height)
#     ax.set_xticks([])
#     ax.set_yticks([])
#     ax.set_title(step_text)

#     for y in range(height):
#         for x in range(width):
#             if (x, y) in wall_health_snapshot:
#                 hp = wall_health_snapshot[(x, y)]
#                 ax.add_patch(patches.Rectangle((x, height - y - 1), 1, 1, color='gray'))
#                 if hp == 1:
#                     ax.text(x + 0.5, height - y - 0.5, 'x', ha='center', va='center', fontsize=16, color='red')
#             else:
#                 cell = board[y][x]
#                 if cell == '@':
#                     ax.add_patch(patches.Circle((x + 0.5, height - y - 0.5), 0.3, color='black'))
#                 else:
#                     ax.add_patch(patches.Rectangle((x, height - y - 1), 1, 1, edgecolor='lightgray', facecolor='white'))

#     for player, pos in tank_positions.items():
#         if pos:
#             x, y = pos
#             dx, dy = arrow_vectors[tank_directions[player]]
#             color = 'blue' if player == '1' else 'red'
#             ax.add_patch(patches.Circle((x + 0.5, height - y - 0.5), 0.3, color=color))
#             ax.arrow(x + 0.5, height - y - 0.5, dx, dy, head_width=0.1, head_length=0.1, fc='white', ec='white')

#     for sx, sy in shell_positions:
#         ax.add_patch(patches.Circle((sx + 0.5, height - sy - 0.5), 0.1, color='orange'))

#     for player, pos in destroyed_tanks_snapshot.items():
#         if pos:
#             x, y = pos
#             ax.text(x + 0.5, height - y - 0.5, 'x', ha='center', va='center', fontsize=20, color='black')

#     fig.canvas.draw_idle()

# def play_loop(event):
#     playing[0] = not playing[0]
#     update_play_button_label()
#     while playing[0] and index[0] < len(frames):
#         draw_frame(index[0])
#         plt.pause(0.5 / speed_multiplier[0])
#         index[0] += 1

# def update_play_button_label():
#     bplay.label.set_text("Pause" if playing[0] else "Play")

# def next_frame(event):
#     if index[0] < len(frames) - 1:
#         index[0] += 1
#         draw_frame(index[0])

# def prev_frame(event):
#     if index[0] > 0:
#         index[0] -= 1
#         draw_frame(index[0])

# def double_speed(event):
#     speed_multiplier[0] *= 2
#     if speed_multiplier[0] > 4:
#         speed_multiplier[0] = 1
#     update_speed_button_label()

# def update_speed_button_label():
#     bspeed.label.set_text(f"x{speed_multiplier[0]} Speed")

# axprev = plt.axes([0.2, 0.05, 0.1, 0.075])
# axnext = plt.axes([0.5, 0.05, 0.1, 0.075])
# axplay = plt.axes([0.35, 0.05, 0.1, 0.075])
# axspeed = plt.axes([0.7, 0.05, 0.15, 0.075])

# bprev = Button(axprev, '<--')
# bnext = Button(axnext, '-->')
# bplay = Button(axplay, 'Play')
# bspeed = Button(axspeed, 'x1 Speed')

# bprev.on_clicked(prev_frame)
# bnext.on_clicked(next_frame)
# bplay.on_clicked(play_loop)
# bspeed.on_clicked(double_speed)

# draw_frame(index[0])
# plt.show()
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.widgets import Button
import re
import sys
import copy

# ----------- הגדרות עיצוב ----------
PLAYER_COLORS = {1: "blue", 2: "red"}
PLAYER_TEXTCOLORS = {1: "white", 2: "white"}
EXPLOSION_COLOR = "gold"
MINE_COLOR = "black"
SHELL_COLOR = "orange"
WALL_COLOR = "gray"
WALL_FONT_COLOR = "black"
BOARD_EDGE = "lightgray"
GRID_FONT = "DejaVu Sans"

arrow_vectors = {
    0: (0, 0.4), 1: (0.28, 0.28), 2: (0.4, 0), 3: (0.28, -0.28),
    4: (0, -0.4), 5: (-0.28, -0.28), 6: (-0.4, 0), 7: (-0.28, 0.28)
}

def read_board_size_from_log(log_lines):
    for line in log_lines:
        if line.startswith("Board size:"):
            parts = line.strip().split()
            rows = int(parts[2])
            cols = int(parts[4])
            return rows, cols
    raise ValueError("Board size not found in log file")

def parse_board_from_log(log_lines, rows, cols):
    board_lines = []
    for idx, line in enumerate(log_lines):
        if line.startswith("Board after Turn"):
            board = []
            for i in range(rows):
                if idx + 1 + i < len(log_lines):
                    board.append(log_lines[idx + 1 + i].ljust(cols)[:cols])
            board_lines.append(board)
    return board_lines

def parse_tank_info(log_lines):
    tank_regex = re.compile(r"Player (\d+) Tank (\d) tank placed at \((\d+),(\d+)\)")
    tanks = []
    for line in log_lines:
        m = tank_regex.match(line)
        if m:
            tanks.append({
                "player": int(m.group(1)),
                "index": int(m.group(2)),
                "pos": (int(m.group(3)), int(m.group(4))),
                "dir": 6 if int(m.group(1)) == 1 else 2  # Player 1 שמאלה, Player 2 ימינה
            })
    return tanks

def parse_walls_and_mines(board):
    walls = {}
    mines = set()
    for y, row in enumerate(board):
        for x, c in enumerate(row):
            if c == "#":
                walls[(x, y)] = 2
            elif c == "@":
                mines.add((x, y))
    return walls, mines

def build_game_states(log_lines, rows, cols):
    board_lines = parse_board_from_log(log_lines, rows, cols)
    tank_init_info = parse_tank_info(log_lines)
    num_turns = len(board_lines)

    # טנקים לפי שחקן ואינדקס לידה
    tanks_by_birth = {(t["player"], t["index"]): {"dir": t["dir"], "alive": True, "pos": t["pos"]} for t in tank_init_info}

    # מצבים ראשוניים
    wall_hp, mine_pos = parse_walls_and_mines(board_lines[0])
    wall_hp_per_turn = [copy.deepcopy(wall_hp)]
    mine_pos_per_turn = [copy.deepcopy(mine_pos)]
    shells_per_turn = [[]]
    tanks_per_turn = [copy.deepcopy(tanks_by_birth)]

    # מיפוי תחילת וסוף תורים בלוג
    turn_indices = [i for i, l in enumerate(log_lines) if l.startswith("Turn :")]
    turn_indices.append(len(log_lines))  # כך שהאחרון יכלול עד סוף הקובץ

    for i in range(len(turn_indices) - 1):
        turn_start, turn_end = turn_indices[i], turn_indices[i+1]
        turn_lines = log_lines[turn_start:turn_end]

        # העתק מצבים מהתור הקודם
        tanks_state = copy.deepcopy(tanks_per_turn[-1])
        wall_hp_state = copy.deepcopy(wall_hp_per_turn[-1])
        mine_pos_state = copy.deepcopy(mine_pos_per_turn[-1])
        shells = []

        for line in turn_lines:
            # תנועות טנקים
            m = re.match(r"Player (\d) Tank (\d): MoveForward to \((\d+), (\d+)\)", line)
            if m:
                player, idx, x, y = int(m[1]), int(m[2]), int(m[3]), int(m[4])
                tanks_state[(player, idx)]["pos"] = (x, y)
            # סיבוב טנקים
            m = re.match(r"Player (\d) Tank (\d): Direction is now (\d+)", line)
            if m:
                player, idx, new_dir = int(m[1]), int(m[2]), int(m[3])
                tanks_state[(player, idx)]["dir"] = new_dir
            # טנק הושמד ע"י פגז
            m = re.match(r"Shell:Player (\d) Tank (\d) destroyed at \((\d+),(\d+)\)", line)
            if m:
                player, idx, x, y = int(m[1]), int(m[2]), int(m[3]), int(m[4])
                tanks_state[(player, idx)]["alive"] = False
                tanks_state[(player, idx)]["pos"] = (x, y)
            # פגיעה בקיר
            m = re.match(r"Shell hit wall at \((\d+),(\d+)\). Wall HP: (\d+)", line)
            if m:
                x, y, hp = int(m[1]), int(m[2]), int(m[3])
                wall_hp_state[(x, y)] = hp
            # קיר נהרס
            m = re.match(r"Wall destroyed at \((\d+),(\d+)\)", line)
            if m:
                x, y = int(m[1]), int(m[2])
                if (x, y) in wall_hp_state:
                    del wall_hp_state[(x, y)]
            # פגז מופיע על הלוח
            m = re.match(r"Shell detected at \((\d+),(\d+)\)", line)
            if m:
                sx, sy = int(m[1]), int(m[2])
                shells.append((sx, sy))
            # טנק פוגע במוקש
            m = re.match(r"Mine:Player (\d) Tank (\d) hit a mine at \((\d+),(\d+)\)", line)
            if m:
                player, idx, x, y = int(m[1]), int(m[2]), int(m[3]), int(m[4])
                tanks_state[(player, idx)]["alive"] = False
                tanks_state[(player, idx)]["pos"] = (x, y)
                if (x, y) in mine_pos_state:
                    mine_pos_state.remove((x, y))
            # תוכל להוסיף עוד סוגי פעולות אם צריך (לדוג' פגז שנורה, סיבוב, ירייה וכו')

        wall_hp_per_turn.append(copy.deepcopy(wall_hp_state))
        mine_pos_per_turn.append(copy.deepcopy(mine_pos_state))
        shells_per_turn.append(copy.deepcopy(shells))
        tanks_per_turn.append(copy.deepcopy(tanks_state))

    # הסר את הראשון (התחלתי), השווה לכמות התורות
    return board_lines, tanks_per_turn[1:], wall_hp_per_turn[1:], mine_pos_per_turn[1:], shells_per_turn[1:]

def draw_board(ax, board, tanks, walls, mines, shells, tank_dirs, destroyed, log_message, turn_idx):
    h = len(board)
    w = len(board[0])
    ax.clear()
    ax.set_xlim(0, w)
    ax.set_ylim(0, h)
    ax.set_xticks([])
    ax.set_yticks([])
    ax.set_title(f"Turn {turn_idx+1}", fontsize=15)

    # ציור ריבועים
    for y in range(h):
        for x in range(w):
            ax.add_patch(patches.Rectangle((x, h-y-1), 1, 1, edgecolor=BOARD_EDGE, facecolor="white"))

    # קירות
    for (x, y), hp in walls.items():
        ax.add_patch(patches.Rectangle((x, h-y-1), 1, 1, color=WALL_COLOR))
        ax.text(x+0.5, h-y-0.5, str(hp), fontsize=18, color=WALL_FONT_COLOR, ha='center', va='center', fontname=GRID_FONT, fontweight='bold')

    # מוקשים
    for (x, y) in mines:
        ax.add_patch(patches.Circle((x+0.5, h-y-0.5), 0.16, color=MINE_COLOR))
        ax.text(x+0.5, h-y-0.5, "M", fontsize=16, color="white", ha='center', va='center', fontweight='bold')

    # טנקים
    for (player, idx), tinfo in tanks.items():
        if not tinfo["alive"]:
            continue
        x, y = tinfo["pos"]
        color = PLAYER_COLORS[player]
        txtcolor = PLAYER_TEXTCOLORS[player]
        dir_arrow = arrow_vectors[tinfo["dir"]]
        ax.add_patch(patches.Circle((x+0.5, h-y-0.5), 0.35, color=color, zorder=2))
        ax.arrow(x+0.5, h-y-0.5, dir_arrow[0]*0.8, dir_arrow[1]*0.8, head_width=0.18, head_length=0.18, fc='white', ec='white', lw=2, zorder=3)
        ax.text(x+0.5, h-y-0.5, str(idx), fontsize=20, color=txtcolor, ha='center', va='center', fontweight='bold')

    # פגזים
    for sx, sy in shells:
        ax.add_patch(patches.Circle((sx+0.5, h-sy-0.5), 0.13, color=SHELL_COLOR, zorder=4))

    # טנקים שהושמדו (X)
    for (player, idx), tinfo in tanks.items():
        if not tinfo["alive"]:
            x, y = tinfo["pos"]
            ax.text(x+0.5, h-y-0.5, "💥", fontsize=22, color=EXPLOSION_COLOR, ha='center', va='center', zorder=10)

    # הודעת לוג למטה
    ax.text(w/2, -0.4, log_message, fontsize=13, ha='center', va='top', fontname=GRID_FONT)
    ax.set_xlim(0, w)
    ax.set_ylim(-1, h+0.5)
    ax.axis("off")

def vis_game(log_lines):
    rows, cols = read_board_size_from_log(log_lines)
    board_lines, tanks_by_turn, walls_by_turn, mines_by_turn, shells_by_turn = build_game_states(log_lines, rows, cols)
    num_turns = len(board_lines)
    index = [0]
    playing = [False]
    speed_multiplier = [1]

    fig, ax = plt.subplots(figsize=(cols / 1.5, rows / 1.5))
    plt.subplots_adjust(bottom=0.23)


    def draw(i):
        draw_board(ax, board_lines[i], tanks_by_turn[i], walls_by_turn[i], mines_by_turn[i], shells_by_turn[i], {}, {}, f"Turn {i+1}", i)
        fig.canvas.draw_idle()

    def next_frame(event=None):
        if index[0] < num_turns - 1:
            index[0] += 1
            draw(index[0])

    def prev_frame(event=None):
        if index[0] > 0:
            index[0] -= 1
            draw(index[0])

    def play_loop(event=None):
        playing[0] = not playing[0]
        update_play_button_label()
        while playing[0] and index[0] < num_turns - 1:
            next_frame()
            plt.pause(0.6 / speed_multiplier[0])

    def update_play_button_label():
        bplay.label.set_text("Pause" if playing[0] else "Play")

    def double_speed(event):
        speed_multiplier[0] *= 2
        if speed_multiplier[0] > 4:
            speed_multiplier[0] = 1
        bspeed.label.set_text(f"x{speed_multiplier[0]} Speed")

    # כפתורים
    axprev = plt.axes([0.13, 0.05, 0.11, 0.1])
    axnext = plt.axes([0.38, 0.05, 0.11, 0.1])
    axplay = plt.axes([0.63, 0.05, 0.11, 0.1])
    axspeed = plt.axes([0.80, 0.05, 0.13, 0.1])
    bprev = Button(axprev, '<--')
    bnext = Button(axnext, '-->')
    bplay = Button(axplay, 'Play')
    bspeed = Button(axspeed, 'x1 Speed')
    bprev.on_clicked(prev_frame)
    bnext.on_clicked(next_frame)
    bplay.on_clicked(play_loop)
    bspeed.on_clicked(double_speed)

    draw(index[0])
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 visual.py debug_log.txt")
        sys.exit(1)
    output_file = sys.argv[1]
    with open(output_file) as f:
        log_lines = [line.rstrip('\n') for line in f]
    vis_game(log_lines)
