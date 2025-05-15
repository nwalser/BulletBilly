import tkinter as tk
from tkinter import ttk
from ttkthemes import ThemedTk

import numpy as np
from matplotlib.backend_bases import key_press_handler
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
from matplotlib.figure import Figure

root = ThemedTk()

root.wm_title("Embedded in Tk")

fig = Figure(figsize=(5, 4), dpi=100)
t = np.arange(0, 3, .01)
ax = fig.add_subplot()
line, = ax.plot(t, 2 * np.sin(2 * np.pi * t))
ax.set_xlabel("time [s]")
ax.set_ylabel("f(t)")

canvas = FigureCanvasTkAgg(fig, master=root)
canvas.draw()

toolbar = NavigationToolbar2Tk(canvas, root, pack_toolbar=False)
toolbar.update()

canvas.mpl_connect("key_press_event", lambda event: print(f"you pressed {event.key}"))
canvas.mpl_connect("key_press_event", key_press_handler)


def update_frequency(new_val):
    f = float(new_val)
    y = 2 * np.sin(2 * np.pi * f * t)
    line.set_data(t, y)
    canvas.draw()


button_quit = ttk.Button(master=root, text="Quit", command=root.destroy)
slider_update = ttk.Scale(root, from_=1, to=5, orient=tk.HORIZONTAL, command=update_frequency)
label = ttk.Label(root, text="Frequency [Hz]")

# Layout
canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=True)

button_quit.pack(side=tk.BOTTOM)
label.pack(side=tk.BOTTOM)
slider_update.pack(side=tk.BOTTOM, fill=tk.X)
toolbar.pack(side=tk.BOTTOM, fill=tk.X)

tk.mainloop()
