import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
from matplotlib.figure import Figure

def sync_axes(ax_source, ax_target, canvas_target):
    def on_change(event_ax):
        ax_target.set_xlim(ax_source.get_xlim())
        ax_target.set_ylim(ax_source.get_ylim())
        canvas_target.draw_idle()
    ax_source.callbacks.connect('xlim_changed', on_change)
    ax_source.callbacks.connect('ylim_changed', on_change)

root = tk.Tk()

# Create a frame to hold everything using grid
frame = tk.Frame(root)
frame.pack()

# Create figures
fig1 = Figure(figsize=(4, 3))
ax1 = fig1.add_subplot(111)
ax1.plot([1, 2, 3])

fig2 = Figure(figsize=(4, 3))
ax2 = fig2.add_subplot(111)
ax2.plot([3, 2, 1])

# Canvases
canvas1 = FigureCanvasTkAgg(fig1, master=frame)
canvas2 = FigureCanvasTkAgg(fig2, master=frame)

# Toolbars
toolbar1 = NavigationToolbar2Tk(canvas1, frame)
toolbar1.grid(row=0, column=0)
canvas1.get_tk_widget().grid(row=1, column=0)

toolbar2 = NavigationToolbar2Tk(canvas2, frame)
toolbar2.grid(row=0, column=1)
canvas2.get_tk_widget().grid(row=1, column=1)

# Sync
sync_axes(ax1, ax2, canvas2)
sync_axes(ax2, ax1, canvas1)

root.mainloop()
