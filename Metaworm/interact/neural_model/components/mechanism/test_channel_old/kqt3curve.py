import numpy as np
import math
import matplotlib.pyplot as plt
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
plt.figure(figsize=(8,4))
plt.subplot(1,3,1)
plt.plot(x, y1)
plt.plot(x, y2)
plt.plot(x, y3)
plt.xlabel("x")
plt.ylabel("inf")
plt.subplot(1,3,2)
plt.plot(x, y4)
plt.plot(x, y5)
plt.xlabel("x")
plt.ylabel("tau_m")
plt.subplot(1,3,3)
plt.plot(x, y6)
plt.xlabel("x")
plt.ylabel("tau_w")
plt.savefig('kqt3.png')



'''
minf = 1 / (1 + exp(-(v + 12.8) / 15.8))
tmf = 395.3 / (1 + ((v + 38.1) / 33.6) ^ 2)
tms = 5503.0 + -5345.4 / (1 + 10 ^ (-0.0283 * (-23.9 - v))) + -4590 / (1 + 10 ^ (-0.0357 * (14.2 + v)))
winf = 0.5 + 0.5 / (1 + exp((v + 1.1) / 28.8))
sinf = 0.3 + 0.7 / (1 + exp((v + 45.3) / 12.3))
tw = 0.5 + 2.9 / (1 + ((v + 48.1) / 48.8) ^ 2)
ts = 500
'''