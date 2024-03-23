import os
import shutil


muscle_cell_names = ["MCDR01", "MCDR02", "MCDR03", "MCDR04", "MCDR05", "MCDR06", "MCDR07", "MCDR08", "MCDR09", "MCDR10", "MCDR11", "MCDR12", "MCDR13", "MCDR14", "MCDR15", "MCDR16", "MCDR17", "MCDR18", "MCDR19", "MCDR20", "MCDR21", "MCDR22", "MCDR23", "MCDR24", "MCVR01", "MCVR02", "MCVR03", "MCVR04", "MCVR05", "MCVR06", "MCVR07", "MCVR08", "MCVR09", "MCVR10", "MCVR11", "MCVR12", "MCVR13", "MCVR14", "MCVR15", "MCVR16", "MCVR17", "MCVR18", "MCVR19", "MCVR20", "MCVR21", "MCVR22", "MCVR23", "MCVR24", "MCDL01", "MCDL02", "MCDL03", "MCDL04", "MCDL05", "MCDL06", "MCDL07", "MCDL08", "MCDL09", "MCDL10", "MCDL11", "MCDL12", "MCDL13", "MCDL14", "MCDL15", "MCDL16", "MCDL17", "MCDL18", "MCDL19", "MCDL20", "MCDL21", "MCDL22", "MCDL23", "MCDL24", "MCVL01", "MCVL02", "MCVL03", "MCVL04", "MCVL05", "MCVL06", "MCVL07", "MCVL08", "MCVL09", "MCVL10", "MCVL11", "MCVL12", "MCVL13", "MCVL14", "MCVL15", "MCVL16", "MCVL17", "MCVL18", "MCVL19", "MCVL20", "MCVL21", "MCVL22", "MCVL23", "MCVL24"]

model_path = os.path.join(os.path.dirname(__file__), "../model")
param_path = os.path.join(os.path.dirname(__file__), "../param/cell")
print(model_path)
cnt = 0
for cell_name in muscle_cell_names:
    src = os.path.join(model_path, "MuscleCell.hoc")
    dst = os.path.join(model_path, cell_name+".hoc")
    shutil.copyfile(src, dst)
    with open(dst, 'r') as r:
        lines = r.readlines()
    with open(dst, 'w') as w:
        for l in lines:
            if "begintemplate" in l:
                w.write(f"begintemplate {cell_name}\n")
            elif "endtemplate" in l:
                w.write(f"endtemplate {cell_name}\n")
            else:
                w.write(l)
    shutil.copyfile(os.path.join(param_path, "MuscleCell.json"), os.path.join(param_path, cell_name+".json"))
    cnt += 1
print(cnt)