# ********** a running example **********
# Load training set data
# and input to the network
# and get the output of the network
# commond:  python run_training_set.py --config /home/zhaomengdi/Project/eworm/Celegans_as_deep_net_net/chemotaxis_net/config.json --randseed 0


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

    training_set_path = "/home/zhaomengdi/Project/eworm/Celegans_as_deep_net_net/chemotaxis_net/motion_sim/training_set/tn2560_fn256_tl_1000_04-12-11:45"
    connection_path = "/home/zhaomengdi/Project/eworm/Celegans_as_deep_net_net/chemotaxis_net/network/output/connection_data/head_connection_simple_env.pkl"
    data = pickle.load(open(os.path.join(training_set_path, "0.dat"), "rb"))
    circuit = pickle.load(open(connection_path, 'rb'))

    input_neuron = ["AWAL", "AWAR", "AWCL", "AWCR"]
    output_neuron_L = ["RIML","RMEL","RMED","RMDDL","RMDL","RMDVL","RIVL","SMDDL","SMDVL","SMBDL","SMBVL"]
    output_neuron_R = ["RIMR","RMER","RMEV","RMDDR","RMDR","RMDVR","RIVR","SMDDR","SMDVR","SMBDR","SMBVR"]

    set_h_before(config)
    chemo_net = init_network(config, circuit)
    set_h_after(dt, tstop, v_init)

    dt_training_set = 100 # (ms)

    AWAL = data['input_traces'][0,0,:]
    AWAR = data['input_traces'][0,1,:]
    AWCL = data['input_traces'][0,2,:]
    AWCR = data['input_traces'][0,3,:]
    L = data['output_traces'][0,0,:]
    R = data['output_traces'][0,1,:]
    for t in range(len(AWAL)):
        for c in range(int(dt_training_set/dt)):
            for conn in chemo_net.input_connections:
                conn.connection.vpre = eval(conn.post_cell.name)[t]
            
            net_output_L = 0
            for cell_name in output_neuron_L:
                net_output_L += chemo_net.name_cell_dict[cell_name].cell.Soma(0.5).v
            net_output_L = net_output_L / len(output_neuron_L)

            net_output_R = 0
            for cell_name in output_neuron_R:
                net_output_R += chemo_net.name_cell_dict[cell_name].cell.Soma(0.5).v
            net_output_R = net_output_R / len(output_neuron_R)
            h.fadvance()
    

    figure_path = "output/neuron_voltage_fig/group2/"
    if not os.path.exists(figure_path):
        os.makedirs(figure_path)
    export_volt_figure(chemo_net, figure_path)

    plt.figure()
    for n in input_neuron:
        plt.plot(eval(n), label = n)
    plt.legend()
    plt.savefig(os.path.join(figure_path, "00_input.png"))

    plt.figure()
    for n in ["L", "R"]:
        plt.plot(eval(n), label = n)
    plt.legend()
    plt.savefig(os.path.join(figure_path, "00_output.png"))

    

