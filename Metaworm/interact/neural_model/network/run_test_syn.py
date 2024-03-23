# ********** a running example **********
# initializa a network
# and give input to the network
# and get the output of the network
# and export the data for visualization
# commond:  python run.py --config /home/zhaomengdi/Project/eworm/Celegans_as_deep_net_net/chemotaxis_net/config.json --randseed 0

from cProfile import label
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

def MultiDimInterpolate(x, xp, fp):
    return np.array([np.interp(x, xp, fp[i, :len(xp)]) for i in range(len(fp))])


if __name__ == "__main__":

    config_file = "/home/zhaomengdi/Project/eworm/Celegans_as_deep_net_net/chemotaxis_net/config_test_syn.json"
    config = load_json(config_file)
    random_seed = 0
    np.random.seed(random_seed)
    dt = 0.5 # (ms)
    tstop = 4000 # (ms)
    v_init = -78 # (mV)
    trace_length = int(tstop / dt)
    
    set_h_before(config)
    
    input_trace = pickle.load(open("/home/zhaomengdi/Project/eworm/Celegans_as_deep_net_net/chemotaxis_net/network/test_syn.pkl", 'rb'))

    # construct network and simulate
    # path = "/home/zhaomengdi/Project/eworm/Celegans_as_deep_net_net/chemotaxis_net/network/output/connection_data/head_connection_simple_env.pkl"
    # circuit = pickle.load(open(path, 'rb'))
    # input_neuron = ["AWAL", "AWAR", "AWCL", "AWCR"]
    circuit = construct_connection(config, "AWAL")
    chemo_net = init_network(config, circuit)
    set_h_after(dt, tstop, v_init)

    output_trace = np.zeros([trace_length,], dtype=np.float)
    
    for conn in chemo_net.connections:
        print(conn.pre_cell, conn.post_cell.name, conn.post_segment.index, conn.weight)
    
    for t in range(trace_length):
        for conn in chemo_net.input_connections:
            conn.connection.vpre = input_trace[0][t]
        h.fadvance()
        output_trace[t] = chemo_net.name_cell_dict["AWAL"].cell.Soma(0.5).v

    print(output_trace)
    plt.figure()
    plt.plot(input_trace[0], label="input")
    plt.plot(output_trace, label="output")
    plt.legend()
    plt.savefig("test_syn.png")

    