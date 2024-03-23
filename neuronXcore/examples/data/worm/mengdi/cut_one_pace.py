import numpy as np
import matplotlib.cm as cm
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt

f = open("c302_C2_FW.muscles.dat", "r")
fs = open("cut_one_pace.txt", "w")   # seq: DR0-24, VR, DL, VL
s = 1600
e = 2400
dt = 0.005
rg = [int(s/dt), int(e/dt)]
time = np.zeros(rg[1] - rg[0])
data = np.zeros((rg[1] - rg[0], 96), dtype=np.float64)
cnt = 0
i = 0
while True:
    line = f.readline()
    if cnt >= rg[0] and cnt < rg[1]:
        strs = line.split('\t')
        nums = []
        for j in range(1, len(strs)-1):
            tmp = strs[j].split('e')
            data[i][j-1] = float(tmp[0]) * (10 ** int(tmp[1])) * 1000
        i += 1
    elif cnt >= rg[1]:
        break
    cnt += 1


ldata = data.reshape((100,1600,-1))
ldata = ldata.mean(axis=1)


import matplotlib.pyplot as plt

plt.imshow(data.transpose(),cmap='hot',aspect='auto')
plt.colorbar()
plt.savefig('cut_1.png')
plt.show() 

plt.imshow(ldata.transpose(),cmap='hot',aspect='auto')
plt.colorbar()
plt.savefig('cut_2.png')
plt.show() 


for i in range(len(ldata)):
    for j in range(len(ldata[0])):
        if j == 0:
            fs.write("%.4f"%ldata[i][j])
        else:
            fs.write(" %.4f"%ldata[i][j])
    fs.write("\n")
fs.close()
