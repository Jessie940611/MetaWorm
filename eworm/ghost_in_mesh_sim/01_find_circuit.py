"""This file is to explore how the hyperparameters influence neural network dynamics"""

import os.path as path
import datetime
import argparse
from eworm.network import process


chemotaxis_neuron = ["AWAL", "AWAR", "AWCL", "AWCR"]
# 82
sensory_neuron = ["ASIL", "ASIR", "ASJL", "ASJR", "AWAL", "AWAR", "ASGL", "ASGR", "AWBL", "AWBR", "ASEL", "ASER", "ADFL", "ADFR", "AFDL", "AFDR", "AWCL", "AWCR", "ASKL", "ASKR", "ASHL", "ASHR", "ADLL", "ADLR", "BAGL", "BAGR", "URXL", "URXR", "ALNL", "ALNR", "PLNL", "PLNR", "SDQL", "SDQR", "AQR", "PQR", "ALML", "ALMR", "AVM", "PVM", "PLML", "PLMR", "FLPL", "FLPR", "PVDL", "PVDR", "ADEL", "ADER", "PDEL", "PDER", "PHAL", "PHAR", "PHBL", "PHBR", "PHCL", "PHCR", "IL2DL", "IL2DR", "IL2L", "IL2R", "IL2VL", "IL2VR", "CEPDL", "CEPDR", "CEPVL", "CEPVR", "URYDL", "URYDR", "URYVL", "URYVR", "OLLL", "OLLR", "OLQDL", "OLQDR", "OLQVL", "OLQVR", "IL1DL", "IL1DR", "IL1L", "IL1R", "IL1VL", "IL1VR"]
# 80
inter_neuron = ["I1L", "I1R", "I2L", "I2R", "I3", "I4", "I5", "I6", "NSML", "NSMR", "DVA", "AINL", "AINR", "AIML", "AIMR", "RIH", "URBL", "URBR", "RIR", "AIYL", "AIYR", "AIAL", "AIAR", "AUAL", "AUAR", "AIZL", "AIZR", "RIS", "ALA", "PVQL", "PVQR", "ADAL", "ADAR", "RIFL", "RIFR", "BDUL", "BDUR", "PVR", "AVFL", "AVFR", "AVHL", "AVHR", "PVPL", "PVPR", "LUAL", "LUAR", "PVNL", "PVNR", "AVG", "DVB", "RIBL", "RIBR", "RIGL", "RIGR", "AIBL", "AIBR", "RICL", "RICR", "SAADL", "SAADR", "SAAVL", "SAAVR", "AVKL", "AVKR", "DVC", "AVJL", "AVJR", "PVT", "AVDL", "AVDR", "AVL", "PVWL", "PVWR", "RIAL", "RIAR", "RID", "RIPL", "RIPR", "CANL", "CANR"]
# 8
command_neuron = ["AVEL", "AVER", "AVBL", "AVBR", "AVAL", "AVAR", "PVCL", "PVCR"]
# 43
head_motor_neuron = ["RMGL", "RMGR", "RIML", "RIMR", "RMFL", "RMFR", "URADL", "URADR", "URAVL", "URAVR", "RMEL", "RMER", "RMED", "RMEV", "RMDDL", "RMDDR", "RMDL", "RMDR", "RMDVL", "RMDVR", "RIVL", "RIVR", "RMHL", "RMHR", "SABD", "SABVL", "SABVR", "SMDDL", "SMDDR", "SMDVL", "SMDVR", "SMBDL", "SMBDR", "SMBVL", "SMBVR", "SIBDL", "SIBDR", "SIBVL", "SIBVR", "SIADL", "SIADR", "SIAVL", "SIAVR"]
# 89
motor_neuron = ["M1", "M2L", "M2R", "M3L", "M3R", "M4", "M5", "MCL", "MCR", "MI", "DA01", "DA02", "DA03", "DA04", "DA05", "DA06", "DA07", "DA08", "DA09", "PDA", "DB01", "DB02", "DB03", "DB04", "DB05", "DB06", "DB07", "AS01", "AS02", "AS03", "AS04", "AS05", "AS06", "AS07", "AS08", "AS09", "AS10", "AS11", "PDB", "DD01", "DD02", "DD03", "DD04", "DD05", "DD06", "VA01", "VA02", "VA03", "VA04", "VA05", "VA06", "VA07", "VA08", "VA09", "VA10", "VA11", "VA12", "VB01", "VB02", "VB03", "VB04", "VB05", "VB06", "VB07", "VB08", "VB09", "VB10", "VB11", "VD01", "VD02", "VD03", "VD04", "VD05", "VD06", "VD07", "VD08", "VD09", "VD10", "VD11", "VD12", "VD13", "HSNL", "HSNR", "VC01", "VC02", "VC03", "VC04", "VC05", "VC06"]
all_neuron = sensory_neuron + inter_neuron + command_neuron + head_motor_neuron + motor_neuron



if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Process some integers.')
    parser.add_argument('--mode', type=str, default='active',
                        help='"active" or "passive", soma\'s property')
    parser.add_argument('--syn_weight_min', type=float, default=0.1,
                        help='minimum of random synapse weight')
    parser.add_argument('--syn_weight_max', type=float, default=1,
                        help='maximum of random synapse weight')
    parser.add_argument('--gj_weight_min', type=float, default=1e-6,
                        help='minimum of random gap junction weight')
    parser.add_argument('--gj_weight_max', type=float, default=1e-5,
                        help='maximum of random gap junction weight')
    parser.add_argument('--inh_prob', type=float, default=0.3,
                        help='inhibitory synapse\'s proportion in all synapse.')
    parser.add_argument('--syn_sample_distribution', type=str, default='log',
                        help='"log" or "uniform" distribution for synaptic weight sampling')
    parser.add_argument('--gj_sample_distribution', type=str, default='log',
                        help='"log" or "uniform" distribution for gap junction weight sampling')
    parser.add_argument('--input_syn_weight', type=float, default=5,
                        help='input synaptic weight')
    args = parser.parse_args()


    sim_config = {"dt": 5/3, "tstop": 10000, "v_init": -65, "secondorder": 0}
    search_config = {
        "total_sample": 32,
        "sim_per_sample": 1,
        "thread_num": 32,
        "input_cell_names": chemotaxis_neuron,
        "output_cell_names": all_neuron,
        "config_file_dir": path.join(path.dirname(__file__), "data", "config", "config.json"),
        "mode": args.mode, # *
        "visualize_function": ["visualize_io_data"]  # "visualize_neuron_heatmap"
    }
    data_config = {
        "factory_name": "ghost_in_mesh_data_factory",
        "args": {
            "loc_path": path.join(path.dirname(__file__), '..',
                                  "ghost_in_mesh_sim", "data", "state",
                                  "worm_states_300_220428-152601.json")
        }}

    weight_config = {
        "syn": (args.syn_weight_min, args.syn_weight_max),     # *, *
        "gj": (args.gj_weight_min, args.gj_weight_max),        # *, *
        "inh_prob": args.inh_prob,                   # *
        "sample_distribution": {
            "syn": args.syn_sample_distribution,     # *
            "gj": args.gj_sample_distribution        # *
        },
        "input_syn_weight": args.input_syn_weight,   # *
        "simplify_connection": False,
        "cluster_connection": True,
        "net_type": "detailed"
    }


    TIME = datetime.datetime.now().strftime('%m-%d-%H%M')
    save_dir = path.join(path.dirname(__file__), "output", "find_circuit",
                         f"{TIME}_{args.mode:s}_{args.syn_weight_min:.1f}_{args.syn_weight_max:.1f}_{args.gj_weight_min:.6f}_{args.gj_weight_max:.6f}_{args.inh_prob:.2f}_{args.syn_sample_distribution:s}_{args.syn_sample_distribution:s}_{args.input_syn_weight:d}")
    process.circuit_search(sim_config, save_dir, search_config, data_config, weight_config)
