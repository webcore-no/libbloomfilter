import csv
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.widgets import Slider, Button, RadioButtons



b = 28
m = np.power(2,b)
n = np.arange(0, 10000000, 100000)
k = 2

x = (np.power(1-np.power(np.e, -(k*n)/m), k))*100


# Real
yreal = []
xreal = []
with open('real_fullness.csv', 'r') as csvfile:
    plots= csv.reader(csvfile, delimiter=',')
    for row in plots:
        yreal.append(int(row[0]))
        xreal.append(float(row[1]))


# Draw

fig, ax = plt.subplots()
ax.set_ylabel("Elements")
ax.set_xlabel("%")

ax.plot(x, n, label="Theoretical");
ax.plot(xreal, yreal, label="Real(xxh3)");
ax.legend()
plt.show()
