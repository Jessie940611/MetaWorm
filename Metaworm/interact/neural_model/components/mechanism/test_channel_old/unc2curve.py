import numpy as np
import math
import matplotlib.pyplot as plt
x = np.arange(-90, 90, 0.1)

vhm = -12.2 
ka = 4.0 
vhh = -52.5 
ki = 5.6 
atm = 1.5 
btm = -8.2 
ctm = 9.1 
dtm = 15.4 
etm = 0.1 
ath = 83.8 
bth = 52.9 
cth = -3.5 
dth = 72.1 
eth = 23.9 
fth = -3.6 

y1 = []
y2 = []
y3 = []
y4 = []

for v in x:
    minf = 1 / (1 + np.exp(-(v - vhm) / ka))
    hinf = 1 / (1 + np.exp((v - vhh) / ki))
    tm = atm / (np.exp(-(v - btm) / ctm) + np.exp((v - btm) / dtm)) + etm
    th = ath / (1 + np.exp(-(v - bth) / cth)) + dth / (1 + np.exp((v - eth) / fth))
    y1.append(minf)
    y2.append(hinf)
    y3.append(tm)
    y4.append(th)
plt.figure(figsize=(8,4))
plt.subplot(1,3,1)
plt.plot(x, y1)
plt.plot(x, y2)
plt.xlabel("x")
plt.ylabel("minf")
plt.subplot(1,3,2)
plt.plot(x, y3)
plt.xlabel("x")
plt.ylabel("tau_m")
plt.subplot(1,3,3)
plt.plot(x, y4)
plt.xlabel("x")
plt.ylabel("tau_h")
plt.savefig('unc2.png')