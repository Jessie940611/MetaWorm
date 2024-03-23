import os
path = "/home/zhaomengdi/Project/eworm/Celegans_as_deep_net_net/chemotaxis_net/components/model_302"
files= os.listdir(path) #得到文件夹下的所有文件名称
print(len(files))
s = []
cell_name_list = ["I1L", "I1R", "I2L", "I2R", "I3", "I4", "I5", "I6", "M1", "M2L", "M2R", "M3L", "M3R", "M4", "M5", "MCL", "MCR", "MI", "NSML", "NSMR", "ASIL", "ASIR", "ASJL", "ASJR", "AWAL", "AWAR", "ASGL", "ASGR", "AWBL", "AWBR", "ASEL", "ASER", "ADFL", "ADFR", "AFDL", "AFDR", "AWCL", "AWCR", "ASKL", "ASKR", "ASHL", "ASHR", "ADLL", "ADLR", "BAGL", "BAGR", "URXL", "URXR", "ALNL", "ALNR", "PLNL", "PLNR", "SDQL", "SDQR", "AQR", "PQR", "ALML", "ALMR", "AVM", "PVM", "PLML", "PLMR", "FLPL", "FLPR", "DVA", "PVDL", "PVDR", "ADEL", "ADER", "PDEL", "PDER", "PHAL", "PHAR", "PHBL", "PHBR", "PHCL", "PHCR", "IL2DL", "IL2DR", "IL2L", "IL2R", "IL2VL", "IL2VR", "CEPDL", "CEPDR", "CEPVL", "CEPVR", "URYDL", "URYDR", "URYVL", "URYVR", "OLLL", "OLLR", "OLQDL", "OLQDR", "OLQVL", "OLQVR", "IL1DL", "IL1DR", "IL1L", "IL1R", "IL1VL", "IL1VR", "AINL", "AINR", "AIML", "AIMR", "RIH", "URBL", "URBR", "RIR", "AIYL", "AIYR", "AIAL", "AIAR", "AUAL", "AUAR", "AIZL", "AIZR", "RIS", "ALA", "PVQL", "PVQR", "ADAL", "ADAR", "RIFL", "RIFR", "BDUL", "BDUR", "PVR", "AVFL", "AVFR", "AVHL", "AVHR", "PVPL", "PVPR", "LUAL", "LUAR", "PVNL", "PVNR", "AVG", "DVB", "RIBL", "RIBR", "RIGL", "RIGR", "RMGL", "RMGR", "AIBL", "AIBR", "RICL", "RICR", "SAADL", "SAADR", "SAAVL", "SAAVR", "AVKL", "AVKR", "DVC", "AVJL", "AVJR", "PVT", "AVDL", "AVDR", "AVL", "PVWL", "PVWR", "RIAL", "RIAR", "RIML", "RIMR", "AVEL", "AVER", "RMFL", "RMFR", "RID", "AVBL", "AVBR", "AVAL", "AVAR", "PVCL", "PVCR", "RIPL", "RIPR", "URADL", "URADR", "URAVL", "URAVR", "RMEL", "RMER", "RMED", "RMEV", "RMDDL", "RMDDR", "RMDL", "RMDR", "RMDVL", "RMDVR", "RIVL", "RIVR", "RMHL", "RMHR", "SABD", "SABVL", "SABVR", "SMDDL", "SMDDR", "SMDVL", "SMDVR", "SMBDL", "SMBDR", "SMBVL", "SMBVR", "SIBDL", "SIBDR", "SIBVL", "SIBVR", "SIADL", "SIADR", "SIAVL", "SIAVR", "DA01", "DA02", "DA03", "DA04", "DA05", "DA06", "DA07", "DA08", "DA09", "PDA", "DB01", "DB02", "DB03", "DB04", "DB05", "DB06", "DB07", "AS01", "AS02", "AS03", "AS04", "AS05", "AS06", "AS07", "AS08", "AS09", "AS10", "AS11", "PDB", "DD01", "DD02", "DD03", "DD04", "DD05", "DD06", "VA01", "VA02", "VA03", "VA04", "VA05", "VA06", "VA07", "VA08", "VA09", "VA10", "VA11", "VA12", "VB01", "VB02", "VB03", "VB04", "VB05", "VB06", "VB07", "VB08", "VB09", "VB10", "VB11", "VD01", "VD02", "VD03", "VD04", "VD05", "VD06", "VD07", "VD08", "VD09", "VD10", "VD11", "VD12", "VD13", "CANL", "CANR", "HSNL", "HSNR", "VC01", "VC02", "VC03", "VC04", "VC05", "VC06"]


'''
# 删除不在网络中的细胞文件
print(len(files))
for file in files: #遍历文件夹
    if file.replace(".hoc", "") not in cell_name_list \
    and file != "a_model_make.py" \
    and file.replace(".hoc", "")[:-1]+"0"+file.replace(".hoc", "")[-1] not in cell_name_list:
        print(file)
        os.remove(file)
'''



# 把DD1.hoc这种文件重命名为DD01.hoc
import shutil
for file in files: #遍历文件夹
    filename = file.replace(".hoc", "")
    if filename[-3:-1]+"0"+filename[-1] in cell_name_list:
        print(file, filename[-3:-1]+"0"+filename[-1]+".hoc")
        shutil.copyfile(file, filename[-3:-1]+"0"+filename[-1]+".hoc")
        os.remove(file)






# 修改文件内容-1: 删除没用的行（把swc文件导入neuron，生成了一个hoc文件，跟该文件对照，多的那些行被删除了）
files= os.listdir(path)
print(len(files))
c = 0
for file in files:
    if not os.path.isdir(file) and file != "a_model_make.py":
        c += 1
        with open(file,'r') as r:
            lines=r.readlines()
        cnt = 0
        start = 0
        with open(file,'w') as w:
            for l in lines:
                if start == 0 and "begintemplate" not in l:
                    pass
                elif start == 0 and "begintemplate" in l:
                    start = 1
                    if file[:-4].replace("0","") not in cell_name_list:
                        w.write("begintemplate "+file[:-4]+"\n")
                    else:
                        w.write(l)
                elif cnt == 0 and "proc set_initial_v() {" in l:
                    cnt = 3
                    pass
                elif cnt == 0 and "proc set_initial_ion_properties() {" in l:
                    cnt = 7
                elif cnt == 0 and "// Specific capacitance" in l:
                    cnt = 33
                elif cnt == 0 and "endtemplate" in l and file[:-4].replace("0","") not in cell_name_list:
                    w.write("endtemplate "+file[:-4]+"\n")
                elif cnt != 0:
                    cnt -= 1
                    pass
                elif cnt == 0:
                    w.write(l)
print(c)




# 修改文件内容-2：a.增加nseg, nseg=n3dp; b.去除all这个SectionList中重复的项 
files= os.listdir(path)
print(len(files))
c = 0
for file in files:
    print(file)
    if not os.path.isdir(file) and file != "a_model_make.py":
        c += 1
        sec_name = []
        nseg_dic = {}
        with open(file,'r') as r:
            lines=r.readlines()
        start = 0
        flag = 0
        with open(file,'w') as w:
            for l in lines:
                # count
                if "create" in l:
                    sn = l[7:-1]
                    sec_name.append(sn)
                    nseg_dic[sn] = 0
                if start == 0 and "proc basic_shape" in l:
                    start = 1
                if start == 1:
                    if l == "}\n":
                        start = 0
                    elif "//" not in l and "pt3dclear()" not in l:
                        for sn in sec_name:
                            if sn+' ' in l:
                                nseg_dic[sn] += 1
                # lock, delete and write SectionList
                if flag == 0 and "// Group: {name=all," in l:
                    flag = 2
                elif flag == 2 and "// Group: {name=all," in l:
                    w.write(l)
                    w.write("\tall = new SectionList()\n")
                    for key, value in nseg_dic.items():
                        w.write("\t"+key+" all.append()\n")
                    flag = 1 # delete other line
                elif flag == 1 and l == "}\n":
                    flag = 0

                # write others
                if flag != 1:
                    w.write(l)
                    if "// All sections not mentioned here have nseg = 1" in l: # insert nseg
                        for key, value in nseg_dic.items():
                            w.write("\t"+key+" nseg = "+str(value)+"\n")
                
        print(nseg_dic)
print(c)
