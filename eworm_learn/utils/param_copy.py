import os
import shutil

path = os.path.join(os.path.dirname(__file__), "../param/cell")
print(path)
files= os.listdir(path)


AIYL_list = ["I1L","I1R","I2L","I2R","I3","I4","I5","I6","NSML","NSMR","DVA","AINL","AINR","AIML","AIMR","RIH","URBL","URBR","RIR","AIYL","AIYR","AIAL","AIAR","AUAL","AUAR","AIZL","AIZR","RIS","ALA","PVQL","PVQR","ADAL","ADAR","RIFL","RIFR","BDUL","BDUR","PVR","AVFL","AVFR","AVHL","AVHR","PVPL","PVPR","LUAL","LUAR","PVNL","PVNR","AVG","DVB","RIBL","RIBR","RIGL","RIGR","AIBL","AIBR","RICL","RICR","SAADL","SAADR","SAAVL","SAAVR","AVKL","AVKR","DVC","AVJL","AVJR","PVT","AVDL","AVDR","AVL","PVWL","PVWR","RIAL","RIAR","RID","RIPL","RIPR","CANL","CANR"]

AVAL_list = ["AVEL","AVER","AVBL","AVBR","AVAL","AVAR","PVCL","PVCR"]

AWCL_list = ["ASIL","ASIR","ASJL","ASJR","AWAL","AWAR","ASGL","ASGR","AWBL","AWBR","ASEL","ASER","ADFL","ADFR","AFDL","AFDR","AWCL","AWCR","ASKL","ASKR","ASHL","ASHR","ADLL","ADLR","BAGL","BAGR","URXL","URXR","ALNL","ALNR","PLNL","PLNR","SDQL","SDQR","AQR","PQR","ALML","ALMR","AVM","PVM","PLML","PLMR","FLPL","FLPR","PVDL","PVDR","ADEL","ADER","PDEL","PDER","PHAL","PHAR","PHBL","PHBR","PHCL","PHCR","IL2DL","IL2DR","IL2L","IL2R","IL2VL","IL2VR","CEPDL","CEPDR","CEPVL","CEPVR","URYDL","URYDR","URYVL","URYVR","OLLL","OLLR","OLQDL","OLQDR","OLQVL","OLQVR","IL1DL","IL1DR","IL1L","IL1R","IL1VL","IL1VR"]

RIML_list = ["RMGL","RMGR","RIML","RIMR","RMFL","RMFR","URADL","URADR","URAVL","URAVR","RMEL","RMER","RMED","RMEV","RMDDL","RMDDR","RMDL","RMDR","RMDVL","RMDVR","RIVL","RIVR","RMHL","RMHR","SABD","SABVL","SABVR","SMDDL","SMDDR","SMDVL","SMDVR","SMBDL","SMBDR","SMBVL","SMBVR","SIBDL","SIBDR","SIBVL","SIBVR","SIADL","SIADR","SIAVL","SIAVR"]

VD05_list = ["M1","M2L","M2R","M3L","M3R","M4","M5","MI","DA01","DA02","DA03","DA04","DA05","DA06","DA07","DA08","DA09","PDA","DB01","DB02","DB03","DB04","DB05","DB06","DB07","AS01","AS02","AS03","AS04","AS05","AS06","AS07","AS08","AS09","AS10","AS11","PDB","DD01","DD02","DD03","DD04","DD05","DD06","VA01","VA02","VA03","VA04","VA05","VA06","VA07","VA08","VA09","VA10","VA11","VA12","VB01","VB02","VB03","VB04","VB05","VB06","VB07","VB08","VB09","VB10","VB11","VD01","VD02","VD03","VD04","VD05","VD06","VD07","VD08","VD09","VD10","VD11","VD12","VD13","HSNL","HSNR","VC01","VC02","VC03","VC04","VC05","VC06","MCL","MCR"]

print(len(AWCL_list+AIYL_list+RIML_list+AVAL_list+VD05_list))

temp_cell = ["AWCL", "AIYL", "RIML", "AVAL", "VD05"]
gnrl_cell_list_list = [AWCL_list, AIYL_list, RIML_list, AVAL_list, VD05_list]


for cell_name, gnrl_cell_list in zip(temp_cell, gnrl_cell_list_list):
    for gnrl_cell in gnrl_cell_list:
        if cell_name != gnrl_cell:
            shutil.copyfile(os.path.join(path, cell_name+".json"), os.path.join(path, gnrl_cell+".json"))

print(len(os.listdir(path)))