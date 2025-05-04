import matplotlib.pyplot as plt
import numpy as np
from matplotlib import gridspec

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

    X, Y = np.meshgrid(edges, np.arange(data.shape[1] + 1))
    hm = a.pcolormesh(X, Y, data.T, cmap='viridis', vmin=0, vmax=vmax, shading='flat')

def plot_series(a, x, data, title, ylim):
    a.plot(x, data)
    a.set_title(title)
    a.set_ylim(ylim)
    a.set_xlim([np.min(entry_depth), np.max(entry_depth)])


# LOAD DATA
parsed = read_data("data7.txt")

entry_depth = extract_value(parsed, 8)
wallOffset = extract_array(parsed, 2)
radius = extract_value(parsed, 6)
offsetX = extract_value(parsed, 4)
offsetY = extract_value(parsed, 5)
offset = np.sqrt(np.pow(offsetX, 2) + np.pow(offsetY, 2))

# PLOT
fig = plt.figure(figsize=(12, 12))
gs = gridspec.GridSpec(3, 1, height_ratios=[8, 1, 1])
axes = [fig.add_subplot(gs[i]) for i in range(3)]


plot_heatmap(axes[0], entry_depth, wallOffset, vmax = 0.05)
plot_series(axes[1], entry_depth, radius, "Pipe Radius", ylim=[0.15, 0.20])
plot_series(axes[2], entry_depth, offset, "Lidar Center Offset", ylim=[0, 0.05])

plt.tight_layout()
plt.show()