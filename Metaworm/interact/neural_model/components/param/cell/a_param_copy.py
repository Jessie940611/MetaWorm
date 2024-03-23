from lib2to3.pgen2.token import AWAIT
import os
import shutil

path = "/home/zhaomengdi/Project/eworm/Celegans_as_deep_net_net/chemotaxis_net/components/param/cell"
files= os.listdir(path)

AWAL_list = ["AWAL","AWAR","AWCL","AWCR"]
AIYL_list = ["AIYL","AIYR","AIAL","AIAR","AIZL","AIZR","RIBL","RIBR","AIBL","AIBR","SAADL","SAADR","SAAVL","SAAVR"]
RIML_list = ["RIML","RIMR","RMEL","RMER","RMED","RMEV","RMDDL","RMDDR","RMDL","RMDR","RMDVL","RMDVR","RIVL","RIVR","SMDDL","SMDDR","SMDVL","SMDVR","SMBDL","SMBDR","SMBVL","SMBVR"]
AVAL_list = ["AVEL","AVER","AVBL","AVBR","AVAL","AVAR","PVCL","PVCR"]
VD05_list = ["DA01","DA02","DA03","DA04","DA05","DA06","DA07","DA08","DA09","DB01","DB02","DB03","DB04","DB05","DB06","DB07","DD01","DD02","DD03","DD04","DD05","DD06","VA01","VA02","VA03","VA04","VA05","VA06","VA07","VA08","VA09","VA10","VA11","VA12","VB01","VB02","VB03","VB04","VB05","VB06","VB07","VB08","VB09","VB10","VB11","VD01","VD02","VD03","VD04","VD05","VD06","VD07","VD08","VD09","VD10","VD11","VD12","VD13"]

temp_cell = ["AWCL", "AIYL", "RIML", "AVAL", "VD05"]
gnrl_cell_list_list = [AWAL_list, AIYL_list, RIML_list, AVAL_list, VD05_list]


for cell_name, gnrl_cell_list in zip(temp_cell, gnrl_cell_list_list):
    for gnrl_cell in gnrl_cell_list:
        if cell_name != gnrl_cell:
            shutil.copyfile(cell_name+".json", gnrl_cell+".json")

print(len(os.listdir(path)))