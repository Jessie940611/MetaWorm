import os
path = "/home/zhaomengdi/Project/eworm/Celegans_as_deep_net_net/chemotaxis_net/components/model"
files= os.listdir(path) #得到文件夹下的所有文件名称
s = []
cell_name_list = ["AWAL","AWAR","AWCL","AWCR","AIYL","AIYR","AIAL","AIAR","AIZL","AIZR","RIBL","RIBR","AIBL","AIBR","SAADL","SAADR","SAAVL","SAAVR","RIML","RIMR","AVEL","AVER","AVBL","AVBR","AVAL","AVAR","PVCL","PVCR","RMEL","RMER","RMED","RMEV","RMDDL","RMDDR","RMDL","RMDR","RMDVL","RMDVR","RIVL","RIVR","SMDDL","SMDDR","SMDVL","SMDVR","SMBDL","SMBDR","SMBVL","SMBVR","DA01","DA02","DA03","DA04","DA05","DA06","DA07","DA08","DA09","DB01","DB02","DB03","DB04","DB05","DB06","DB07","DD01","DD02","DD03","DD04","DD05","DD06","VA01","VA02","VA03","VA04","VA05","VA06","VA07","VA08","VA09","VA10","VA11","VA12","VB01","VB02","VB03","VB04","VB05","VB06","VB07","VB08","VB09","VB10","VB11","VD01","VD02","VD03","VD04","VD05","VD06","VD07","VD08","VD09","VD10","VD11","VD12","VD13"]

'''
# 把DD1.hoc这种文件重命名为DD01.hoc
print(len(files))
for file in files: #遍历文件夹
    if file.replace(".hoc", "") not in cell_name_list \
    and file != "a_make.py" \
    and file.replace(".hoc", "")[:-1]+"0"+file.replace(".hoc", "")[-1] not in cell_name_list:
        print(file)
        os.remove(file)
'''

'''
# 删除不在网络中的细胞文件
import shutil
for file in files: #遍历文件夹
    if file[-3:-1]+"0"+file[-1] in cell_name_list:
        print(file, file[-3:-1]+"0"+file[-1]+".hoc")
        shutil.copyfile(file, file[-3:-1]+"0"+file[-1]+".hoc")
        os.remove(file)
'''

'''
# 修改文件内容-1: 删除没用的行（把swc文件导入neuron，生成了一个hoc文件，跟该文件对照，多的那些行被删除了）
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
'''

'''
# 修改文件内容-2：a.增加nseg, nseg=n3dp; b.去除all这个SectionList中重复的项 
print(len(files))
c = 0
for file in files:
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
                if start == 0 and "proc basic_shape() {" in l:
                    start = 1
                if start == 1:
                    if l == "}\n":
                        start = 0
                    elif "//" not in l and "pt3dclear()" not in l:
                        for sn in sec_name:
                            if sn in l:
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
'''