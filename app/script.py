import matplotlib.pyplot as plt
import numpy as np
from fontTools.misc.cython import returns
from matplotlib import gridspec
from matplotlib.colors import LinearSegmentedColormap
import matplotlib
from matplotlib import cm
from matplotlib.widgets import SpanSelector
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
wall_offset = extract_array(parsed, 2)
radius = extract_value(parsed, 6)
offsetX = extract_value(parsed, 4)
offsetY = extract_value(parsed, 5)
offset = np.sqrt(np.pow(offsetX, 2) + np.pow(offsetY, 2))

# PLOT
#fig = plt.figure(figsize=(12, 24))
#gs = gridspec.GridSpec(5, 1, height_ratios=[5, 1, 1, 1, 1])
#axes = [fig.add_subplot(gs[i]) for i in range(5)]
#
#
#plot_heatmap(axes[0], entry_depth, wall_offset, vmax = 0.04)
#plot_series(axes[1], entry_depth, any_anomaly, "Any Anomaly", ylim=[0, 200], ylabel="[yes/no]")
#plot_series(axes[2], entry_depth, radius*1000, "Pipe Radius", ylim=[150, 200], ylabel="[mm]")
#plot_series(axes[3], entry_depth, offset*1000, "Lidar Center Offset", ylim=[0, 50], ylabel="Offset [mm]")
#plot_series(axes[4], entry_depth, tilt, "Tilt", ylim=[-10, 10], ylabel="Tilt [deg]")

#plt.tight_layout()
# plt.show()


#### PLOTTING -----
fig = plt.figure(figsize=(12, 24))
gs = gridspec.GridSpec(10, 5)

## AX 1
anomalies = np.clip(np.sum(np.abs(wall_offset), axis=1), 0, 1)
ax1 = fig.add_subplot(gs[0, :])
ax1.plot(entry_depth, anomalies)

## 3D Plot
ax2 = fig.add_subplot(gs[1:7, 0:2])

ax3 = fig.add_subplot(gs[1:3, 2:])
ax3.plot(entry_depth, tilt)
plot_heatmap(ax3, entry_depth, wall_offset, vmax = 0.04)

ax4 = fig.add_subplot(gs[3:5, 2:])
ax4.plot(entry_depth, tilt)
ax4.set_ylabel("Tilt [°]")

ax5 = fig.add_subplot(gs[5:7, 2:])
ax5.plot(entry_depth, radius * 1000)
ax5.set_ylabel("Radius [mm]")




## INTERACTIVITY
def onselect(xmin, xmax):
    ax2.set_xlim(xmin=xmin, xmax=xmax)
    ax3.set_xlim(xmin=xmin, xmax=xmax)
    ax4.set_xlim(xmin=xmin, xmax=xmax)
    ax5.set_xlim(xmin=xmin, xmax=xmax)
    fig.canvas.draw_idle()

span = SpanSelector(
    ax1,
    onselect,
    "horizontal",
    onmove_callback=onselect,
    useblit=True,
    props=dict(alpha=0.5),
    interactive=True,
    drag_from_anywhere=True
)


plt.tight_layout()
plt.show()