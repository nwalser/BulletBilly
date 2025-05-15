import matplotlib.pyplot as plt
import numpy as np
from matplotlib import gridspec
import matplotlib
from matplotlib.widgets import SpanSelector
from matplotlib import cm
from matplotlib.colors import Normalize
from matplotlib.ticker import FuncFormatter
from matplotlib.lines import Line2D
from tkinter import Tk, filedialog
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
matplotlib.use('qtagg')

def parse_data(text):
    text = text.replace('{', '[').replace('}', ']')
    return eval(text, {"__builtins__": None, "inf": float('inf'), "-inf": float('-inf')})

def read_data(path):
    buffer = ''
    parsed = []
    with open(path, 'r') as f:
        for line in f:
            buffer += line.strip()
            if buffer.count('{') == buffer.count('}'):
                parsed.append(parse_data(buffer))
                buffer = ''
    return parsed

# get scan data
def extract_array(p, num):
    arr = []
    for item in p:
        if isinstance(item, list) and item and isinstance(item[num], list):
            arr.append(item[num])
    return np.array(arr)

def extract_value(p, num):
    arr = []
    for item in p:
        arr.append(item[num])
    return np.array(arr)

# OPEN FILE
root = Tk()
root.withdraw()
file_path = filedialog.askopenfilename()
root.destroy()

# LOAD RAW DATA
parsed = read_data(file_path)[70:280] #TODO: automize data selection

entry_depth = extract_value(parsed, 8)
any_anomaly = extract_value(parsed, 7)
tilt = extract_value(parsed, 9)
wall_offset = extract_array(parsed, 2)
radius = extract_value(parsed, 6)
offsetX = extract_value(parsed, 4)
offsetY = extract_value(parsed, 5)
offset = np.sqrt(np.pow(offsetX, 2) + np.pow(offsetY, 2))

# PROCESS SOME DATA
wall_offset = wall_offset.clip(-0.1, 0.1)
is_defect = np.where(np.abs(wall_offset) > 0.01, True, False)
any_defect = np.any(is_defect, axis=1)
radius = radius.clip(-1, 1)
wall_distance = wall_offset + 0.18
is_good = ~is_defect
wall_distance_m = np.where(is_good, wall_distance, np.nan)
number_of_defects = np.sum(is_defect, axis=1) / 360 * 2 * np.pi * 0.180

edges = np.zeros(len(entry_depth) + 1)
edges[1:-1] = (entry_depth[:-1] + entry_depth[1:]) / 2
edges[0] = entry_depth[0] - (entry_depth[1] - entry_depth[0]) / 2
edges[-1] = entry_depth[-1] + (entry_depth[-1] - entry_depth[-2]) / 2

#### PLOTTING -----
fig = plt.figure(figsize=(12, 24))
gs = gridspec.GridSpec(11, 6)

## PLOT 1: 1D ERROR PLOT
ax1 = fig.add_subplot(gs[0, :])
ax1.set_title("Full Pipeline Error Map")
ax1.fill_between(entry_depth, any_defect, step='mid', alpha=1, color='#FF746C')
ax1.set_ylim(0.1, 0.9)
ax1.set_ylabel("Defect")
ax1.xaxis.set_major_formatter(FuncFormatter(lambda val, _: f"{val*1000:.0f} mm"))
ax1.xaxis.set_ticks_position('top')
ax1.xaxis.set_label_position('top')
pos = ax1.get_position()

## 3D PREVIEW PLOT
ax2 = fig.add_subplot(gs[2:, 0:3], projection='3d')
angles = np.deg2rad(np.linspace(0, 360, wall_distance.shape[1], endpoint=False))
angles_grid = np.tile(angles, (wall_distance.shape[0], 1))
depth_grid = np.tile(entry_depth[:, None], (1, wall_distance.shape[1]))

X = wall_distance_m * np.cos(angles_grid)
Y = wall_distance_m * np.sin(angles_grid)
Z = depth_grid

norm = Normalize(vmin=wall_offset.min(), vmax=wall_offset.max())
colors = cm.viridis(norm(wall_offset))
ax2.plot_surface(Y, Z, X, facecolors=colors, rstride=3, cstride=3, shade=True, axlim_clip=True)
ax2.view_init(elev=15, azim=-60)
ax2.zaxis.set_major_formatter(FuncFormatter(lambda val, _: f"{val*1000:.0f} mm"))
ax2.yaxis.set_major_formatter(FuncFormatter(lambda val, _: f"{val*1000:.0f} mm"))
ax2.xaxis.set_major_formatter(FuncFormatter(lambda val, _: f"{val*1000:.0f} mm"))

## OFFSET MAP
ax3 = fig.add_subplot(gs[1:4, 3:])
ax3.set_title("Section Data")
X, Y = np.meshgrid(edges, np.arange(wall_offset.shape[1] + 1))
hm = ax3.pcolormesh(X, Y, wall_offset.T, cmap="viridis", vmin=0, vmax=0.02, shading='flat', label="Hello")
ax3.yaxis.set_major_formatter(FuncFormatter(lambda val, _: f"{val:.0f} °"))
ax3.xaxis.set_major_formatter(FuncFormatter(lambda val, _: f"{val*1000:.0f} mm"))
ax3.legend([Line2D([0], [0])], ['Wall Offset Map'], loc='upper right')
ax3.grid(True)

## ERROR MAP
ax6 = fig.add_subplot(gs[4:7, 3:])
X, Y = np.meshgrid(edges, np.arange(is_defect.shape[1] + 1))
hm = ax6.pcolormesh(X, Y, is_defect.T, cmap="viridis", vmin=0, vmax=0.02, shading='flat')
ax6.yaxis.set_major_formatter(FuncFormatter(lambda val, _: f"{val:.0f} °"))
ax6.xaxis.set_major_formatter(FuncFormatter(lambda val, _: f"{val*1000:.0f} mm"))
ax6.legend([Line2D([0], [0])], ['Error Map'], loc='upper right')
ax6.grid(True)
ax6.sharex(ax3)
ax6.sharey(ax3)

## TILT
ax4 = fig.add_subplot(gs[9:10, 3:])
ax4.plot(entry_depth, tilt, label="Tilt")
ax4.yaxis.set_major_formatter(FuncFormatter(lambda val, _: f"{val:.1f} °"))
ax4.xaxis.set_major_formatter(FuncFormatter(lambda val, _: f"{val*1000:.0f} mm"))
ax4.legend(loc='upper right')
ax4.grid(True)
ax4.sharex(ax3)

## RADIUS
ax5 = fig.add_subplot(gs[8:9, 3:])
ax5.plot(entry_depth, radius * 1000, label="Radius")
ax5.yaxis.set_major_formatter(FuncFormatter(lambda val, _: f"{val:.0f} mm"))
ax5.xaxis.set_major_formatter(FuncFormatter(lambda val, _: f"{val*1000:.0f} mm"))
ax5.set_ylim(ymin=0)
ax5.legend(loc='upper right')
ax5.grid(True)
ax5.sharex(ax3)

## DEFECT CIRCUMFERENCE
ax7 = fig.add_subplot(gs[7:8, 3:])
ax7.fill_between(entry_depth, number_of_defects * 1000, alpha=1, color='#FF746C', label="Total Defect Circumference")  # fill under the step plot
ax7.yaxis.set_major_formatter(FuncFormatter(lambda val, _: f"{val:.0f} mm"))
ax7.xaxis.set_major_formatter(FuncFormatter(lambda val, _: f"{val*1000:.0f} mm"))
ax7.legend(loc='upper right')
ax7.set_ylim(ymin=0, ymax=100)
ax7.grid(True)
ax7.sharex(ax3)

## CENTER OFFSET
ax9 = fig.add_subplot(gs[10:11, 3:])
ax9.plot(entry_depth, offset * 1000, label="Offset")
ax9.yaxis.set_major_formatter(FuncFormatter(lambda val, _: f"{val:.0f} mm"))
ax9.xaxis.set_major_formatter(FuncFormatter(lambda val, _: f"{val*1000:.0f} mm"))
ax9.set_ylim(ymin=0)
ax9.legend(loc='upper right')
ax9.grid(True)
ax9.sharex(ax3)

## GENERAL INFORMATION
ax8 = fig.add_subplot(gs[1, :3])
ax8.set_title("Inspection Metrics")
ax8.axis('off')
table = ax8.table(loc='center', bbox=[0, 0, 1, 1], cellText=[
    ['Entry Depth', f"{np.max(entry_depth)*1000:.0f}mm"],
    ['Maximal Tilt', f"{np.max(np.abs(tilt)):.1f}°"],
    ['Mission Time', f"{len(entry_depth)*0.1:.0f}s"],
    ['LIDAR Sampling Frequency', f"5kHz"],
    ['LIDAR Maximal Center Offset', f"{np.max(np.abs(offset)) * 1000:.0f}mm"],
])
for key, cell in table.get_celld().items():
    cell.set_text_props(ha='left', va='center')

## INTERACTIVITY
def onmove(xmin, xmax):
    ax3.set_xlim(xmin=xmin, xmax=xmax)
    ax4.set_xlim(xmin=xmin, xmax=xmax)
    ax5.set_xlim(xmin=xmin, xmax=xmax)
    ax6.set_xlim(xmin=xmin, xmax=xmax)
    ax7.set_xlim(xmin=xmin, xmax=xmax)
    ax9.set_xlim(xmin=xmin, xmax=xmax)
    fig.canvas.draw_idle()


def onselect(xmin, xmax):
    ax2.set_ylim(ymin=xmin, ymax=xmax)
    onmove(xmin, xmax)

span = SpanSelector(
    ax1,
    onselect,
    "horizontal",
    onmove_callback=onmove,
    useblit=True,
    props=dict(alpha=0.5),
    interactive=True,
    drag_from_anywhere=True
)



onselect(0, 0.1)

plt.tight_layout()
plt.subplots_adjust(left=0.05, right=0.95, top=0.95, bottom=0.075, wspace=0.5, hspace=0.5)
fig.canvas.manager.set_window_title("BulletBilly Advanced Data Inspection Studio")

plt.show()