"""To see the steady-state activation/inactivation variables and time constants in KQT3"""
import numpy as np
import math
import matplotlib.pyplot as plt
import os
x = np.arange(-90, 90, 0.1)

vhm = 20.4
ka = 7.7
vhh = -7.0
ki = 5.8
atm = 26.6
btm = -33.7
ctm = 15.8
dtm = -33.7
etm = 15.4
ftm = 2.0
ath = 1400

y1 = []
y2 = []
y3 = []
y4 = []

for v in x:
    minf = 1 / (1 + np.exp(-(v - vhm) / ka))
    hinf = 1 / (1 + np.exp((v - vhh) / ki))
    tm = atm / (np.exp(- (v - btm) / ctm) + np.exp((v - dtm) / etm)) + ftm
    th = ath
    y1.append(minf)
    y2.append(hinf)
    y3.append(tm)
    y4.append(th)
plt.figure(figsize=(10,3))
plt.subplot(1,3,1)
plt.plot(x, y1, label='m_inf')
plt.plot(x, y2, label='h_inf')
plt.plot(x, np.array(y1)*np.array(y2), label='m_inf * h_inf')
plt.xlabel("voltage (mV)")
plt.ylabel("infinity")
plt.legend()
plt.subplot(1,3,2)
plt.plot(x, y3)
plt.xlabel("voltage (mV)")
plt.ylabel("tau_m")
plt.subplot(1,3,3)
plt.plot(x, y4)
plt.xlabel("voltage (mV)")
plt.ylabel("tau_h")
plt.tight_layout()
plt.savefig(os.path.join('output', 'variables', 'shk1.png'))
