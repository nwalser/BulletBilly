import matplotlib.pyplot as plt
import numpy as np
from fontTools.misc.cython import returns
from matplotlib import gridspec
from matplotlib.colors import LinearSegmentedColormap

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

def plot_heatmap(a, x, data, vmax = 2.0):
    edges = np.zeros(len(x) + 1)
    edges[1:-1] = (x[:-1] + x[1:]) / 2
    edges[0] = x[0] - (x[1] - x[0]) / 2
    edges[-1] = x[-1] + (x[-1] - x[-2]) / 2

    # own color map
    # cmap = LinearSegmentedColormap.from_list("my_cmap", ["#000000", "#00FF00", "#FF0000"], N=256*4)

    X, Y = np.meshgrid(edges, np.arange(data.shape[1] + 1))
    hm = a.pcolormesh(X, Y, data.T, cmap="inferno_r", vmin=0, vmax=vmax, shading='flat')

def plot_series(a, x, data, title, ylim, ylabel=""):
    a.plot(x, data)
    #a.set_title(title)
    a.set_ylabel(ylabel)
    a.set_ylim(ylim)
    a.set_xlim([np.min(entry_depth), np.max(entry_depth)])


# LOAD DATA
parsed = read_data("data10.txt")[::-1][0:-1]

entry_depth = extract_value(parsed, 8)
any_anomaly = extract_value(parsed, 7)
tilt = extract_value(parsed, 9)
wallOffset = extract_array(parsed, 2)
radius = extract_value(parsed, 6)
offsetX = extract_value(parsed, 4)
offsetY = extract_value(parsed, 5)
offset = np.sqrt(np.pow(offsetX, 2) + np.pow(offsetY, 2))

# PLOT
fig = plt.figure(figsize=(12, 24))
gs = gridspec.GridSpec(5, 1, height_ratios=[20, 1, 1, 1, 1])
axes = [fig.add_subplot(gs[i]) for i in range(5)]


plot_heatmap(axes[0], entry_depth, wallOffset, vmax = 0.04)
plot_series(axes[1], entry_depth, any_anomaly, "Any Anomaly", ylim=[0, 200], ylabel="[yes/no]")
plot_series(axes[2], entry_depth, radius*1000, "Pipe Radius", ylim=[150, 200], ylabel="[mm]")
plot_series(axes[3], entry_depth, offset*1000, "Lidar Center Offset", ylim=[0, 50], ylabel="Offset [mm]")
plot_series(axes[4], entry_depth, tilt, "Tilt", ylim=[-10, 10], ylabel="Tilt [deg]")

plt.tight_layout()
plt.show()


# create 3d plot
# angles and depth
from matplotlib import cm
wallOffset = np.clip(wallOffset, -0.02, 0.02)
wallOffset = wallOffset + 0.05

angles = np.deg2rad(np.linspace(0, 360, wallOffset.shape[1], endpoint=False))
depths = entry_depth

# grids
angles_grid = np.tile(angles, (wallOffset.shape[0], 1))
depth_grid = np.tile(depths[:, None], (1, wallOffset.shape[1]))

# Cartesian coordinates
X = wallOffset * np.cos(angles_grid)
Y = wallOffset * np.sin(angles_grid)
Z = depth_grid

# Normalize wallOffset for colormap
norm = plt.Normalize(vmin=np.min(wallOffset), vmax=np.max(wallOffset))
colors = cm.inferno_r(norm(wallOffset))

# Plot
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.plot_surface(Y, Z, X, facecolors=colors, rstride=1, cstride=1, shade=False)
plt.show()



# CREATE TEST UI
import tkinter as tk
from tkinter import ttk
from ttkthemes import ThemedTk

from matplotlib.backend_bases import key_press_handler
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk


root = ThemedTk()

canvas = FigureCanvasTkAgg(fig, master=root)
canvas.draw()

toolbar = NavigationToolbar2Tk(canvas, root, pack_toolbar=False)
toolbar.update()

# display
toolbar.pack(side=tk.BOTTOM, fill=tk.X)
canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=True)

tk.mainloop()
