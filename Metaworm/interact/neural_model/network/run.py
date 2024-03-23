# ********** a running example **********
# initializa a network
# and give input to the network
# and get the output of the network
# and export the data for visualization
# commond:  python run.py --config /home/zhaomengdi/Project/eworm/Celegans_as_deep_net_net/chemotaxis_net/config.json --randseed 0

import sys
sys.path.append('/home/zhaomengdi/Project/eworm/Celegans_as_deep_net_net/chemotaxis_net/')
from network.init import *
from network.input import *
from network.simulate import *
from network.export import *
from utils.args import *
from utils.jsonfunc import *
import numpy as np
import os
from neuron import h


if __name__ == "__main__":

    args = parser.parse_args()
    config_file = args.config
    config = load_json(config_file)
    random_seed = args.randseed
    np.random.seed(random_seed)
    dt = 0.5 # (ms)
    tstop = 7000 # (ms)
    v_init = -65 # (mV)
    
    set_h_before(config)

    # construct network and simulate
    #path = "/home/zhaomengdi/Project/eworm/Celegans_as_deep_net_net/chemotaxis_net/network/output/connection_data/window-80-epoch-64-circuit.pkl"
    path = "/home/zhaomengdi/Project/eworm/Celegans_as_deep_net_net/chemotaxis_net/network/output/connection_data/head_connection_simple_env.pkl"
    circuit = pickle.load(open(path, 'rb'))
    input_neuron = ["AWAL", "AWAR", "AWCL", "AWCR"]
    circuit = construct_connection(config, input_neuron)
    assert not os.path.exists(path)
    sys.setrecursionlimit(10000)
    pickle.dump(circuit, open(path, 'wb'))
    chemo_net = init_network(config, circuit)
    set_h_after(dt, tstop, v_init)
    # add_constant_current_stimulus(chemo_net, "AWCL", 'Soma', 0.5, 1000, 6000, 35)


    # food = np.tile(np.linspace(start = -0.5, stop = 0.5, num = 1000), 14)
    # toxin = np.tile(np.linspace(start = 0.5, stop = -0.5, num = 1000), 14)
    # add_varing_current_stimulus(chemo_net, "AWCL", "Soma", 0.5, food)
    # add_varing_current_stimulus(chemo_net, "AWCR", "Soma", 0.5, toxin)
    
    h.run(tstop)
    print("simulation dt:%.3f ms, tstop:%d ms"%(h.dt, h.tstop))

    # export network info for visualization
    group_name = "visual_group_head"
    base_path = "output/visual_data/" + group_name + "/"
    if not os.path.exists(base_path):
        os.makedirs(base_path)
    morph_path = base_path + group_name + ".swc"
    syn_path = base_path + group_name + ".snp"
    volt_path = base_path + group_name + ".vtg"
    export_morphology_and_synapse(chemo_net, morph_path, syn_path)
    export_voltage(chemo_net, volt_path)
    figure_path = "output/neuron_voltage_fig/group1/"
    if not os.path.exists(figure_path):
        os.makedirs(figure_path)
    export_volt_figure(chemo_net, figure_path)