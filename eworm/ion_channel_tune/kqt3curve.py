"""To see the steady-state activation/inactivation variables and time constants in KQT3"""
import numpy as np
import math
import matplotlib.pyplot as plt
import os
x = np.arange(-90, 90, 0.1)

vhm = -12.8
ka = 15.8
vhw = -1.1
kiw = 28.8
aw = 0.5
bw = 0.5
vhs = -45.3
kis = 12.3
ass = 0.3
bs =  0.7
atmf = 395.3
btmf = 38.1
ctmf = 33.6
atms = 5503.0
btms = -5345.4
ctms = 0.0283
dtms = -23.9
etms = -4590
ftms = 0.0357
gtms = 14.2
atw = 0.5
btw = 2.9
ctw = -48.1
dtw = 48.8
ats = 500

y1 = []
y2 = []
y3 = []
y4 = []
y5 = []
y6 = []

for v in x:
    minf = 1 / (1 + np.exp(-(v - vhm) / ka))
    tmf = atmf / (1 + ((v + btmf) / ctmf) ** 2)
    tms = atms + btms / (1 + 10 ** (-ctms * (dtms - v))) + etms / (1 + 10 ** (-ftms * (gtms + v)))
    winf = aw + bw / (1 + np.exp((v - vhw) / kiw))
    sinf = ass + bs / (1 + np.exp((v - vhs) / kis))
    tw = atw + btw / (1 + ((v - ctw) / dtw) ** 2)
    y1.append(minf)
    y2.append(winf)
    y3.append(sinf)
    y4.append(tmf)
    y5.append(tms)
    y6.append(tw)
plt.figure(figsize=(10,3))
plt.subplot(1,3,1)
plt.plot(x, y1, label='m')
plt.plot(x, y2, label='w')
plt.plot(x, y3, label='s')
plt.xlabel("x")
plt.ylabel("infinity")
plt.legend()
plt.subplot(1,3,2)
plt.plot(x, y4, label='mf')
plt.plot(x, y5, label='ms')
plt.xlabel("x")
plt.ylabel("tau_m")
plt.legend()
plt.subplot(1,3,3)
plt.plot(x, y6, label='w')
plt.xlabel("x")
plt.ylabel("tau_w")
plt.legend()
plt.tight_layout()
plt.savefig(os.path.join('output', 'variables', 'kqt3.png'))
