import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.widgets import Button
import sys

map_file = sys.argv[1]
output_file = sys.argv[2]

with open(map_file) as f:
    lines = f.read().splitlines()

width, height = map(int, lines[0].split())
valid_symbols = {'#', '@', '1', '2', ' '}

grid = []
for line in lines[1:]:
    row = [c if c in valid_symbols else ' ' for c in line]
    if len(row) < width:
        row += [' '] * (width - len(row))
    grid.append(row)

while len(grid) < height:
    grid.append([' '] * width)

with open(output_file) as f:
    game_lines = [line.strip() for line in f.readlines()]

def get_initial_board():
    return [[cell for cell in row] for row in grid]

arrow_vectors = {
    0: (0, 0.3), 1: (0.3, 0.3), 2: (0.3, 0), 3: (0.3, -0.3),
    4: (0, -0.3), 5: (-0.3, -0.3), 6: (-0.3, 0), 7: (-0.3, 0.3)
}

tank_positions = {'1': None, '2': None}
tank_directions = {'1': 0, '2': 0}
wall_health = {}
frames = []
last_shell_hit = None
destroyed_tanks = {}

for y in range(height):
    for x in range(width):
        cell = grid[y][x]
        if cell == '1':
            tank_positions['1'] = (x, y)
            grid[y][x] = ' '
        elif cell == '2':
            tank_positions['2'] = (x, y)
            grid[y][x] = ' '
        elif cell == '#':
            wall_health[(x, y)] = 2

current_positions = tank_positions.copy()
current_directions = tank_directions.copy()
current_wall_health = wall_health.copy()

for line in game_lines:
    board = get_initial_board()
    shells = []

    if "Shell fired at position" in line:
        coords = line.split("position (")[1].split(")")[0].split(",")
        last_shell_hit = (int(coords[0]), int(coords[1]))
        shells.append(last_shell_hit)

    elif "Shell hit wall" in line and last_shell_hit:
        if last_shell_hit in current_wall_health:
            current_wall_health[last_shell_hit] -= 1
            if current_wall_health[last_shell_hit] <= 0:
                del current_wall_health[last_shell_hit]  # מוחקים לגמרי מהחיים

    elif "Wall destroyed" in line and last_shell_hit:
        if last_shell_hit in current_wall_health:
            del current_wall_health[last_shell_hit]  # מוחקים לגמרי מהחיים

    elif "Player 1 was hit by shell and destroyed" in line:
        destroyed_tanks['1'] = current_positions['1']

    elif "Player 2 was hit by shell and destroyed" in line:
        destroyed_tanks['2'] = current_positions['2']

    elif "Player" in line:
        player = '1' if "Player 1" in line else '2'
        if "MoveForward from" in line:
            to_coords = line.split("to (")[1].split(")")[0].split(",")
            current_positions[player] = (int(to_coords[0]), int(to_coords[1]))
        elif "Rotated right by 45" in line:
            current_directions[player] = (current_directions[player] + 1) % 8
        elif "Rotated left by 45" in line:
            current_directions[player] = (current_directions[player] - 1) % 8
        elif "Rotated right by 90" in line:
            current_directions[player] = (current_directions[player] + 2) % 8
        elif "Rotated left by 90" in line:
            current_directions[player] = (current_directions[player] - 2) % 8
        elif "Shoot from" in line:
            to_coords = line.split("to (")[1].split(")")[0].split(",")
            shells.append((int(to_coords[0]), int(to_coords[1])))
        elif "Direction is now" in line:
            current_directions[player] = int(line.split("Direction is now")[1].strip())

    wall_health_snapshot = current_wall_health.copy()
    frames.append((
        [row.copy() for row in board],
        current_positions.copy(),
        current_directions.copy(),
        list(shells),
        line,
        wall_health_snapshot,
        destroyed_tanks.copy()
    ))

index = [0]
playing = [False]
speed_multiplier = [1]

fig, ax = plt.subplots(figsize=(width / 1.8, height / 1.8))
plt.subplots_adjust(bottom=0.25)

def draw_frame(i):
    board, tank_positions, tank_directions, shell_positions, step_text, wall_health_snapshot, destroyed_tanks_snapshot = frames[i]
    ax.clear()
    ax.set_xlim(0, width)
    ax.set_ylim(0, height)
    ax.set_xticks([])
    ax.set_yticks([])
    ax.set_title(step_text)

    for y in range(height):
        for x in range(width):
            if (x, y) in wall_health_snapshot:
                hp = wall_health_snapshot[(x, y)]
                ax.add_patch(patches.Rectangle((x, height - y - 1), 1, 1, color='gray'))
                if hp == 1:
                    ax.text(x + 0.5, height - y - 0.5, 'x', ha='center', va='center', fontsize=16, color='red')
            else:
                cell = board[y][x]
                if cell == '@':
                    ax.add_patch(patches.Circle((x + 0.5, height - y - 0.5), 0.3, color='black'))
                else:
                    ax.add_patch(patches.Rectangle((x, height - y - 1), 1, 1, edgecolor='lightgray', facecolor='white'))

    for player, pos in tank_positions.items():
        if pos:
            x, y = pos
            dx, dy = arrow_vectors[tank_directions[player]]
            color = 'blue' if player == '1' else 'red'
            ax.add_patch(patches.Circle((x + 0.5, height - y - 0.5), 0.3, color=color))
            ax.arrow(x + 0.5, height - y - 0.5, dx, dy, head_width=0.1, head_length=0.1, fc='white', ec='white')

    for sx, sy in shell_positions:
        ax.add_patch(patches.Circle((sx + 0.5, height - sy - 0.5), 0.1, color='orange'))

    for player, pos in destroyed_tanks_snapshot.items():
        if pos:
            x, y = pos
            ax.text(x + 0.5, height - y - 0.5, 'x', ha='center', va='center', fontsize=20, color='black')

    fig.canvas.draw_idle()

def play_loop(event):
    playing[0] = not playing[0]
    update_play_button_label()
    while playing[0] and index[0] < len(frames):
        draw_frame(index[0])
        plt.pause(0.5 / speed_multiplier[0])
        index[0] += 1

def update_play_button_label():
    bplay.label.set_text("Pause" if playing[0] else "Play")

def next_frame(event):
    if index[0] < len(frames) - 1:
        index[0] += 1
        draw_frame(index[0])

def prev_frame(event):
    if index[0] > 0:
        index[0] -= 1
        draw_frame(index[0])

def double_speed(event):
    speed_multiplier[0] *= 2
    if speed_multiplier[0] > 4:
        speed_multiplier[0] = 1
    update_speed_button_label()

def update_speed_button_label():
    bspeed.label.set_text(f"x{speed_multiplier[0]} Speed")

axprev = plt.axes([0.2, 0.05, 0.1, 0.075])
axnext = plt.axes([0.5, 0.05, 0.1, 0.075])
axplay = plt.axes([0.35, 0.05, 0.1, 0.075])
axspeed = plt.axes([0.7, 0.05, 0.15, 0.075])

bprev = Button(axprev, '<--')
bnext = Button(axnext, '-->')
bplay = Button(axplay, 'Play')
bspeed = Button(axspeed, 'x1 Speed')

bprev.on_clicked(prev_frame)
bnext.on_clicked(next_frame)
bplay.on_clicked(play_loop)
bspeed.on_clicked(double_speed)

draw_frame(index[0])
plt.show()
