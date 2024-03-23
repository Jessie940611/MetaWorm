"""This is an example to show how to interact with GhostInMesh offline"""
from os import path
from eworm.ghost_in_mesh_sim.pre_interaction import *

if __name__ == "__main__":
    # data_path = path.join(path.dirname(__file__), "output", "find_circuit",
    #                       "11-01-1928_active_0.1_1.0_0.000010_0.000100_0.30_log_log_5")
    data_path = "/home/zhaomengdi/Project/model_meta_worm/eworm/experiment/trial10"
    interact_config = {
        "group_name": "reservoir",
        "config_path": path.join(data_path, "000_circuit_search_config.json"),
        # "abs_circuit_path": path.join(data_path, "sample_#0_circuit.pkl"),
        "abs_circuit_path": path.join(data_path, "optimal_abs_circuit.pkl"),
        "sim_config": {"dt": 5/3, "tstop": 10000, "v_init": -65, "secondorder": 0},
        "loc_path": path.join("data", "state", "worm_states_300_220428-152601.json"),
        # "input_cell_name": ["AWAL", "AWAR", "AWCL", "AWCR"],
        "input_cell_name": ["AWAL","AWAR","AWCL","AWCR","ASKL","ASKR","ALNL","ALNR","PLML","PHAL","PHAR","URYDL","URYDR","URYVL","URYVR"],
        "output_cell_name": ["RIML", "RIMR", "RMEL", "RMER", "RMED", "RMEV", "RMDDL", "RMDDR", "RMDL", "RMDR", "RMDVL", "RMDVR", "RIVL", "RIVR", "SMDDL", "SMDDR", "SMDVL", "SMDVR", "SMBDL", "SMBDR", "SMBVL", "SMBVR", "DA01", "DA02", "DA03", "DA04", "DA05", "DA06", "DA07", "DA08", "DA09", "DB01", "DB02", "DB03", "DB04", "DB05", "DB06", "DB07", "DD01", "DD02", "DD03", "DD04", "DD05", "DD06", "VA01", "VA02", "VA03", "VA04", "VA05", "VA06", "VA07", "VA08", "VA09", "VA10", "VA11", "VA12", "VB01", "VB02", "VB03", "VB04", "VB05", "VB06", "VB07", "VB08", "VB09", "VB10", "VB11", "VD01", "VD02", "VD03", "VD04", "VD05", "VD06", "VD07", "VD08", "VD09", "VD10", "VD11", "VD12", "VD13"],
        "muscle_path": path.join("data", "muscle", "worm_muscles_300_220428-152601.json"),
        "muscle_sim_config": {"dt": 100/3, "tstop": 10000, "muscle_num": 96},
        "propioception": False, # default False
        "detail_control": "fc",  # "one_cnn", "multi_cnn", "fc"
        "neuron_muscle_matrix": path.join(path.dirname(__file__), "..", "components", "param", "connection", "neuron_muscle.xlsx")
    }

    # generate neural model output
    # neural_model_output(interact_config)
    neural_model_output_current(interact_config)

    # train neuron muscle map using CNN
    if interact_config.get("detail_control", "fc") == "one_cnn":
        train_neuron_muscle_map(interact_config)
    elif interact_config.get("detail_control", "fc") == "multi_cnn" and interact_config.get("neuron_muscle_matrix", None):
        train_neuron_muscle_detail_map(interact_config)
    elif interact_config.get("detail_control", "fc") == "fc":
        train_neuron_muscle_reservoir(interact_config)
    else:
        raise ValueError
