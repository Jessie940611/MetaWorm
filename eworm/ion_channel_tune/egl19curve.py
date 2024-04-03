"""To see the steady-state activation/inactivation variables and time constants in EGL19"""
import numpy as np
import math
import matplotlib.pyplot as plt
import os

group_name = 'cell2018'
x = np.arange(-90, 90, 0.1)

vhm = 5.9
vhh = -50.6
ka = 7.5
ki = 12.0
vhhb = -20.5
kib = 8.1
ahinf = 1.43
bhinf = 0.14
chinf = 5.96
dhinf = 0.60
atm = 2.9
btm = 5.2
ctm = 6.0
dtm = 1.9
etm = 1.4
ftm = 30.0
gtm = 2.3
ath = 0.4
bth = 44.6
cth = -23.0
dth = 5.0
eth = 36.4
fth = 28.7
gth = 3.7
hth = 43.1
if group_name == 'plos2019':
    vhm = 5.6
    vhh = 24.9
elif group_name == 'cell2018':
    vhm = 5.9
    vhh = -50.6
elif group_name == 'nc2022':
    btm = -4.8
    etm = -8.6
    cth = -33
    ftm = 18.7
    vhm = -4.4
    vhh = 14.9

y1 = []
y2 = []
y3 = []
y4 = []

for v in x:
    minf = 1 / (1 + np.exp(-(v - vhm) / ka))
    hinf = (ahinf / (1 + np.exp(-(v - vhh) / ki)) + bhinf) * (chinf / (1 + np.exp((v - vhhb) / kib)) + dhinf)
    tm = atm * np.exp(-((v - btm) / ctm) ** 2) + dtm * np.exp(-((v - etm) / ftm) ** 2) + gtm
    th = ath * ((bth / (1 + np.exp((v - cth) / dth))) + (eth / (1 + np.exp((v - fth) / gth))) + hth)
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
plt.savefig(os.path.join('output', 'variables', f'egl19_{group_name}.png'))
