from lib2to3.pgen2.token import AWAIT
import os
import shutil
import numpy as np

path = "/home/zhaomengdi/Project/eworm/Celegans_as_deep_net_net/chemotaxis_net/components/param/cell/"
files= os.listdir(path)
s = []

params_AWCL = np.array([380, 2, 3e-05, -75, 1.6e-03, 
                            0.0002, 0.0001, 0.001, 0., 0., 
                            0.002, 0.0015, 0.001, 0., 0.036,
                            1.6e-5, 3.5e-5, 0.0003, 0.00006, 0.001])

params_VD05 = np.array([221., 2, 5e-05, -75, 0, 
                            0.005, 0, 0.001, 0.0046, 0.00016, 
                            0, 0, 0.0011, 0, 0.004, 
                            0.0013, 0.0006, 0, 0.00015, 0]) # cm换成0.2图会更好看

params_AIYL = np.array([269, 7., 1.4e-5, -54.5, 0.001,
                        0.00037, 5.e-04, 0.001, 4.9e-04, 4.e-04,
                        0, 0, 0.00017, 0.00028, 1.74e-04,
                        0.0015, 0.0022, 0.00063, 0.000085, 0.0011])

params_RIML = np.array([344,4,7.7e-5,-33.,0.001,
                        1.5e-04,0.0001,1.e-05,0.0012,5e-05,
                        0.,0.,0.,0.000013,0.009,
                        0.00067,0.,0.,9e-06,0.00105])

params_AVAL = np.array([150,8.,7e-05,-33.,0.0004,
                        0.001,0.,0,0.,0.,
                        0.002,0.0001,0.0006,0,0.06,
                        0.,0.,0.005,0.0007,0.]) 

for cell_name in ["AIYL", "AVAL", "RIML", "VD05", "AWCL"]:
    params = eval("params_"+cell_name)

    with open(path+"template.json",'r') as r:
        lines=r.readlines()

    shutil.copyfile(path+"template.json", path+cell_name+".json")
    flag = 0
    with open(path+cell_name+".json",'w') as w:
        for l in lines:
            if "soma" in l:
                flag = 1
            elif "neurite" in l:
                flag = 2
            if flag == 1 and "Ra" in l:
                w.write(l[:-1]+" %.7f,\n"%params[0])
            elif flag == 1 and "cm" in l:
                w.write(l[:-1]+" %.7f,\n"%params[1])
            elif flag == 1 and "gpas" in l:
                w.write(l[:-1]+" %.7f,\n"%params[2])
            elif flag == 1 and "epas" in l:
                w.write(l[:-1]+" %.7f,\n"%params[3])
            
            elif flag == 2 and "Ra" in l:
                w.write(l[:-1]+" %.7f,\n"%params[0])
            elif flag == 2 and "cm" in l:
                w.write(l[:-1]+" %.7f,\n"%params[1])
            elif flag == 2 and "gpas" in l:
                w.write(l[:-1]+" %.7f,\n"%params[2])
            elif flag == 2 and "epas" in l:
                w.write(l[:-1]+" %.7f\n"%params[3])

            elif "gbshl1" in l:
                w.write(l[:-1]+" %.7f,\n"%params[4])
            elif "gbshk1" in l:
                w.write(l[:-1]+" %.7f,\n"%params[5])
            elif "gbkvs1" in l:
                w.write(l[:-1]+" %.7f,\n"%params[6])
            elif "gbegl2" in l:
                w.write(l[:-1]+" %.7f,\n"%params[7])
            elif "gbegl36" in l:
                w.write(l[:-1]+" %.7f,\n"%params[8])
            elif "gbkqt3" in l:
                w.write(l[:-1]+" %.7f,\n"%params[9])
            elif "gbegl19" in l: 
                w.write(l[:-1]+" %.7f,\n"%params[10])
            elif "gbunc2" in l: 
                w.write(l[:-1]+" %.7f,\n"%params[11])
            elif "gbcca1" in l: 
                w.write(l[:-1]+" %.7f,\n"%params[12])
            elif "gbslo1_egl19" in l: 
                w.write(l[:-1]+" %.7f,\n"%params[13])
            elif "gbslo1_unc2" in l: 
                w.write(l[:-1]+" %.7f,\n"%params[14])
            elif "gbslo2_egl19" in l: 
                w.write(l[:-1]+" %.7f,\n"%params[15])
            elif "gbslo2_unc2" in l: 
                w.write(l[:-1]+" %.7f,\n"%params[16])
            elif "gbkcnl" in l:
                w.write(l[:-1]+" %.7f,\n"%params[17])
            elif "gbnca" in l: 
                w.write(l[:-1]+" %.7f,\n"%params[18])
            elif "gbirk" in l: 
                w.write(l[:-1]+" %.7f\n"%params[19])
            else:
                w.write(l)
