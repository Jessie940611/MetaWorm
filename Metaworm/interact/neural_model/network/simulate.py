# This file is for neuron's h setting
# set_h_before is used before network initialization
# set_h_after is used after network initialization


from neuron import h, load_mechanisms


def set_h_before(config):
    cell_info = config["cell_info"]
    cells_id_sim = cell_info["cells_id_sim"]
    cells_name_dic = cell_info["cells_name_dic"]
    dir_info = config["dir_info"]
    mechanism_dir = dir_info["mechanism_dir"]
    model_dir = dir_info["model_dir"]

    h.load_file('stdrun.hoc')
    load_mechanisms(mechanism_dir)
    for id in cells_id_sim:
        cn = cells_name_dic[id]
        h.load_file(model_dir + cn + ".hoc")


def set_h_after(dt, tstop, v_init):
    h.dt = dt
    h.steps_per_ms = int(1/dt)
    h.tstop = tstop
    h.secondorder=0
    h.finitialize(v_init)