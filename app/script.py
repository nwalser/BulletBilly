import matplotlib.pyplot as plt
import numpy as np

# extract all data
def parse_data(text):
    text = text.replace('{', '[').replace('}', ']')
    return eval(text)

buffer = ''
parsed = []

with open('data6.txt', 'r') as f:
    for line in f:
        buffer += line.strip()
        if buffer.count('{') == buffer.count('}'):
            parsed.append(parse_data(buffer))
            buffer = ''

# get scan data
def extract_array(parsed, num):
    arr = []
    for item in parsed:
        if isinstance(item, list) and item and isinstance(item[num], list):
            arr.append(item[num])
    return np.array(arr)

def extract_value(parsed, num):
    arr = []
    for item in parsed:
        arr.append(item[num])
    return np.array(arr)

fig, axes = plt.subplots(7, 1, figsize=(12, 12))


# plot scan
data = extract_array(parsed, 0)
a = axes[0]
hm = a.imshow(data.transpose(), cmap='viridis', interpolation='nearest', aspect='auto', vmin=0, vmax=0.2)
a.set_title("Lidar Raw")
a.tick_params(labelbottom=False)

#fig.colorbar(hm, ax=a)

# plot fit
data = extract_array(parsed, 1)
a = axes[1]
hm = a.imshow(data.transpose(), cmap='viridis', interpolation='nearest', aspect='auto', vmin=0, vmax=0.2)
a.set_title("Circle Fit")
a.tick_params(labelbottom=False)
#fig.colorbar(hm, ax=a)

# plot offset
data = extract_array(parsed, 2)
a = axes[2]
hm = a.imshow(data.transpose(), cmap='viridis', interpolation='nearest', aspect='auto', vmin=0, vmax=2)
a.set_title("Lidar Raw - Circle Fit")
a.tick_params(labelbottom=False)
#fig.colorbar(hm, ax=a)

# plot anomaly
data = extract_array(parsed, 3)
a = axes[3]
hm = a.imshow(data.transpose(), cmap='viridis', interpolation='nearest', aspect='auto', vmin=0, vmax=1)
a.set_title("Anomaly Map")
a.tick_params(labelbottom=False)
#fig.colorbar(hm, ax=a)


# plot circle parameters
data = extract_value(parsed, 4)
a = axes[4]
a.plot(data)
a.set_title("Offset X")
a.set_ylim([-0.1, 0.1])
a.tick_params(labelbottom=False)

data = extract_value(parsed, 5)
a = axes[5]
a.plot(data)
a.set_title("Offset Y")
a.set_ylim([-0.1, 0.1])
a.tick_params(labelbottom=False)

data = extract_value(parsed, 6)
a = axes[6]
a.plot(data)
a.set_title("Radius")
a.set_ylim([0, 0.5])


plt.tight_layout()
plt.show()


data = extract_array(parsed, 0)[14]
print(data)