import matplotlib.pyplot as plt
from matplotlib.widgets import SpanSelector
import matplotlib
matplotlib.use('qtagg')

fig, ax = plt.subplots()
x = range(100)
y = [i**0.5 for i in x]
ax.plot(x, y)

fixed_width = 20

def onselect(xmin, xmax):
    center = (xmin + xmax) / 2
    new_xmin = center - fixed_width / 2
    new_xmax = center + fixed_width / 2
    ax.set_xlim(new_xmin, new_xmax)
    fig.canvas.draw_idle()

span = SpanSelector(ax, onselect, 'horizontal', useblit=True, interactive=True)
plt.show()
