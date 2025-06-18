import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.widgets import Button
import re
import sys
import copy

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
                "dir": 6 if int(m.group(1)) == 1 else 2  
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
    battle_info_per_turn = []
    tanks_by_birth = {(t["player"], t["index"]): {"dir": t["dir"], "alive": True, "pos": t["pos"]} for t in tank_init_info}

    wall_hp, mine_pos = parse_walls_and_mines(board_lines[0])
    wall_hp_per_turn = [copy.deepcopy(wall_hp)]
    mine_pos_per_turn = [copy.deepcopy(mine_pos)]
    shells_per_turn = [[]]
    tanks_per_turn = [copy.deepcopy(tanks_by_birth)]

    turn_indices = [i for i, l in enumerate(log_lines) if l.startswith("Turn :")]
    turn_indices.append(len(log_lines))  

    for i in range(len(turn_indices) - 1):
        turn_start, turn_end = turn_indices[i], turn_indices[i+1]
        turn_lines = log_lines[turn_start:turn_end]
        battle_requesters = set()


        tanks_state = copy.deepcopy(tanks_per_turn[-1])
        wall_hp_state = copy.deepcopy(wall_hp_per_turn[-1])
        mine_pos_state = copy.deepcopy(mine_pos_per_turn[-1])
        shells = []

        for line in turn_lines:
            if "Requested action - GetBattleInfo" in line:
                m = re.search(r"Player (\d) Tank (\d)", line)
                if m:
                    battle_requesters.add((int(m[1]), int(m[2])))

            m = re.match(r"Player (\d) Tank (\d): MoveForward to \((\d+), (\d+)\)", line)
            if m:
                player, idx, x, y = int(m[1]), int(m[2]), int(m[3]), int(m[4])
                tanks_state[(player, idx)]["pos"] = (x, y)
            m = re.match(r"Player (\d) Tank (\d): Direction is now (\d+)", line)
            if m:
                player, idx, new_dir = int(m[1]), int(m[2]), int(m[3])
                tanks_state[(player, idx)]["dir"] = new_dir
            m = re.match(r"Shell:Player (\d) Tank (\d) destroyed at \((\d+),(\d+)\)", line)
            if m:
                player, idx, x, y = int(m[1]), int(m[2]), int(m[3]), int(m[4])
                tanks_state[(player, idx)]["alive"] = False
                tanks_state[(player, idx)]["pos"] = (x, y)
            m = re.match(r"Shell hit wall at \((\d+),(\d+)\). Wall HP: (\d+)", line)
            if m:
                x, y, hp = int(m[1]), int(m[2]), int(m[3])
                wall_hp_state[(x, y)] = hp
            m = re.match(r"Wall destroyed at \((\d+),(\d+)\)", line)
            if m:
                x, y = int(m[1]), int(m[2])
                if (x, y) in wall_hp_state:
                    del wall_hp_state[(x, y)]
            m = re.match(r"Shell detected at \((\d+),(\d+)\)", line)
            if m:
                sx, sy = int(m[1]), int(m[2])
                shells.append((sx, sy))
            m = re.match(r"Mine:Player (\d) Tank (\d) hit a mine at \((\d+),(\d+)\)", line)
            if m:
                player, idx, x, y = int(m[1]), int(m[2]), int(m[3]), int(m[4])
                tanks_state[(player, idx)]["alive"] = False
                tanks_state[(player, idx)]["pos"] = (x, y)
                if (x, y) in mine_pos_state:
                    mine_pos_state.remove((x, y))

        wall_hp_per_turn.append(copy.deepcopy(wall_hp_state))
        mine_pos_per_turn.append(copy.deepcopy(mine_pos_state))
        shells_per_turn.append(copy.deepcopy(shells))
        tanks_per_turn.append(copy.deepcopy(tanks_state))
        battle_info_per_turn.append(copy.deepcopy(battle_requesters))


    return board_lines, tanks_per_turn[1:], wall_hp_per_turn[1:], mine_pos_per_turn[1:], shells_per_turn[1:], battle_info_per_turn

def draw_board(ax, board, tanks, walls, mines, shells, tank_dirs, destroyed,
               log_message, turn_idx,
               p1_kills_list=None, p2_kills_list=None, p1_shots_list=None, p2_shots_list=None,battle_requesters=None):

    h = len(board)
    w = len(board[0])
    ax.clear()

    ax.set_xlim(0, w + 3)
    ax.set_ylim(-1, h + 0.5)
    ax.set_xticks([])
    ax.set_yticks([])
    ax.set_title(f"Turn {turn_idx + 1}", fontsize=15)

    for y in range(h):
        for x in range(w):
            ax.add_patch(patches.Rectangle((x, h - y - 1), 1, 1, edgecolor=BOARD_EDGE, facecolor="white"))

    for (x, y), hp in walls.items():
        ax.add_patch(patches.Rectangle((x, h - y - 1), 1, 1, color=WALL_COLOR))
        ax.text(x + 0.5, h - y - 0.5, str(hp), fontsize=18, color=WALL_FONT_COLOR,
                ha='center', va='center', fontname=GRID_FONT, fontweight='bold')

    for (x, y) in mines:
        ax.add_patch(patches.Circle((x + 0.5, h - y - 0.5), 0.16, color=MINE_COLOR))
        ax.text(x + 0.5, h - y - 0.5, "M", fontsize=16, color="white", ha='center', va='center', fontweight='bold')

    for (player, idx), tinfo in tanks.items():
        if not tinfo["alive"]:
            continue
        x, y = tinfo["pos"]
        color = PLAYER_COLORS[player]
        txtcolor = PLAYER_TEXTCOLORS[player]
        dir_arrow = arrow_vectors[tinfo["dir"]]
        ax.add_patch(patches.Circle((x + 0.5, h - y - 0.5), 0.35, color=color, zorder=2))
        ax.arrow(x + 0.5, h - y - 0.5, dir_arrow[0] * 0.8, dir_arrow[1] * 0.8,
                 head_width=0.18, head_length=0.18, fc='white', ec='white', lw=2, zorder=3)
        ax.text(x + 0.5, h - y - 0.5, str(idx), fontsize=20, color=txtcolor,
                ha='center', va='center', fontweight='bold')
        if battle_requesters and (player, idx) in battle_requesters:
            ax.text(x + 0.55, h - y - 0.15, "?", fontsize=10, color="black", fontweight='bold')

    for sx, sy in shells:
        ax.add_patch(patches.Circle((sx + 0.5, h - sy - 0.5), 0.13, color=SHELL_COLOR, zorder=4))

    for (player, idx), tinfo in tanks.items():
        if not tinfo["alive"]:
            x, y = tinfo["pos"]
            ax.text(x + 0.5, h - y - 0.5, "X", fontsize=22, color=EXPLOSION_COLOR,
                    ha='center', va='center', zorder=10)

    ax.text(w / 2, -0.4, log_message, fontsize=13, ha='center', va='top')

    if all(lst is not None for lst in [p1_kills_list, p2_kills_list, p1_shots_list, p2_shots_list]):
        stats_x = w + 0.1
        text_y = h - 0.5
        ax.text(stats_x, text_y, "Stats", fontsize=13, fontweight='bold', va='bottom', ha='left')
        text_y -= 0.6

        stat_lines = [
            f"P1 shots: {p1_shots_list[turn_idx]}",
            f"P1 kills: {p1_kills_list[turn_idx]}",
            "",
            f"P2 shots: {p2_shots_list[turn_idx]}",
            f"P2 kills: {p2_kills_list[turn_idx]}"
        ]

        for line in stat_lines:
            ax.text(stats_x, text_y, line, fontsize=11, va='top', ha='left', fontname='DejaVu Sans Mono')
            text_y -= 0.5


def vis_game(log_lines):
    rows, cols = read_board_size_from_log(log_lines)
    board_lines, tanks_by_turn, walls_by_turn, mines_by_turn, shells_by_turn,battle_info_per_turn = build_game_states(log_lines, rows, cols)
    num_turns = len(board_lines)
    index = [0]
    playing = [False]
    speed_multiplier = [1]

 
    turn_starts = [i for i, l in enumerate(log_lines) if l.startswith("Turn :")]
    turn_starts.append(len(log_lines))  

    p1_kills = 0
    p2_kills = 0
    p1_shots = 0
    p2_shots = 0
    p1_kills_list = []
    p2_kills_list = []
    p1_shots_list = []
    p2_shots_list = []

    for i in range(len(turn_starts) - 1):
        turn_lines = log_lines[turn_starts[i]:turn_starts[i+1]]
        for line in turn_lines:
            line = line.strip()
            if "Player 1 Tank" in line and "Requested action - Shoot" in line:
                p1_shots += 1
            if "Player 2 Tank" in line and "Requested action - Shoot" in line:
                p2_shots += 1
            if "Shell:Player 1 Tank" in line and "destroyed" in line:
                p2_kills += 1  
            if "Shell:Player 2 Tank" in line and "destroyed" in line:
                p1_kills += 1  

        p1_kills_list.append(p1_kills)
        p2_kills_list.append(p2_kills)
        p1_shots_list.append(p1_shots)
        p2_shots_list.append(p2_shots)


    winner = None
    for line in reversed(log_lines):
        line = line.lower()
        if "player 1 wins" in line:
            winner = 1
            break
        elif "player 2 wins" in line:
            winner = 2
            break
        elif "tie" in line:
            winner = 0
            break


    fig, ax = plt.subplots(figsize=(cols / 1.5, rows / 1.5))
    plt.subplots_adjust(bottom=0.23)

    def draw(i):
        draw_board(
            ax, board_lines[i], tanks_by_turn[i], walls_by_turn[i], mines_by_turn[i],
            shells_by_turn[i], {}, {}, f"Turn {i+1}", i,
            p1_kills_list, p2_kills_list, p1_shots_list, p2_shots_list,battle_requesters=battle_info_per_turn[i]
        )

  

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

        if index[0] >= num_turns - 1:
            ax.clear()
            ax.text(0.5, 0.6, "Game Over", ha='center', va='center', fontsize=24, color='black', transform=ax.transAxes)
            if winner == 1:
                msg = "Player 1 Wins!"
            elif winner == 2:
                msg = "Player 2 Wins!"
            elif winner == 0:
                msg = "It's a Tie!"
            else:
                msg = "Result Unknown"
            ax.text(0.5, 0.4, msg, ha='center', va='center', fontsize=20, color='red', transform=ax.transAxes)
            ax.axis('off')
            fig.canvas.draw_idle()

    def update_play_button_label():
        bplay.label.set_text("Pause" if playing[0] else "Play")

    def double_speed(event):
        speed_multiplier[0] *= 2
        if speed_multiplier[0] > 4:
            speed_multiplier[0] = 1
        bspeed.label.set_text(f"x{speed_multiplier[0]} Speed")

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


