# This file is for network initialization
# If you do not have a connection file
#    you can fisrt construct the connection randomly via construct_connection
#    then load the connection and generate a network via init_network
# Else
#    you can load the connection and generate a network via init_network directly


import sys
sys.path.append('/home/mahchine/DynamicNeuronX/dev_mahchine_worm_intergrate_yudong/dynamicneuronx/Metaworm/interact/neural_model/')
sys.path.append('/home/mahchine/DynamicNeuronX/dev_mahchine_worm_intergrate_yudong/dynamicneuronx/Metaworm/interact/neural_model/network/')
from network.Network import Network
from network.Connection import Connection
from network.Cell import Cell
from network.Segment import Segment
from network_assemble.data_structure import Circuit, Neuron, Connect, make_connect
from network_assemble.data_structure import Segment as segment
from utils.jsonfunc import *
from utils.excelfunc import *
from utils.distance import *
import pickle
import numpy as np
import os
from neuron import h


# construct connections as a Circuit class (for DNN train and for init network)
def construct_connection(config: dict, input_neuron: list):
    # config: config loaded from .json file, dictionary
    # input_neuron: input_neuron list, list of string
    cell_info = config["cell_info"]
    cells_id_sim = cell_info["cells_id_sim"]
    cells_name_dic = cell_info["cells_name_dic"]
    dir_info = config["dir_info"]
    cell_param_dir = dir_info["cell_param_dir"]
    adj_matrix_dir = dir_info["adj_matrix_dir"]
    synapse_sn = dir_info["synapse_sn"]
    gap_junction_sn = dir_info["gap_junction_sn"]


    net = Network()
    # -------------------- construct cells --------------------
    for sid in cells_id_sim:
        id = int(sid)
        name = cells_name_dic[sid]
        cell_param = load_json(cell_param_dir + name + ".json")
        net.add_cell(Cell(index=id, name=name, cell_param=cell_param))


    # -------------------- construct connections -------------------- 
    circuit = Circuit()
    THRESH = 2  # max distance of two neurites to construct synapse
    WEIGHT_RANGE = {'syn': (0.1, 10), 'gj': (1e-4, 1e-3)}
    INH_PROP = 0.3
    synapse_matrix, _, _ = read_excel(filename=adj_matrix_dir,
                                      sheetname=synapse_sn)
    gapjunction_matrix, _, _ = read_excel(filename=adj_matrix_dir,
                                          sheetname=gap_junction_sn)
    for pre_cell in net.cells:  # for each pre cell
        id_pre_cell = pre_cell.index
        
        # for input connections
        if pre_cell.name in input_neuron:
            new_connect = make_connect(circuit, -id_pre_cell, 1, id_pre_cell, 0, 'syn', 1.)
            circuit.add_connect(new_connect)

        # for normal connections
        for post_cell in net.cells:  # for each post cell
            id_post_cell = post_cell.index
            if id_pre_cell == id_post_cell:
                continue
            syn_n = synapse_matrix.cell_value(int(id_pre_cell), int(id_post_cell))
            gap_n = gapjunction_matrix.cell_value(int(id_pre_cell), int(id_post_cell))

            if len(str(syn_n)) > 0: syn_n = int(syn_n)
            else: syn_n = int(0)
            if len(str(gap_n)) > 0: gap_n = int(gap_n)
            else: gap_n = int(0)
            
            n_connect = syn_n + gap_n
            if n_connect == 0:
                continue
            
            # touch detection
            pre_pid, post_pid, dist_p, cnsyn = find_synapse_loc(pre_cell.points, post_cell.points, n_connect, THRESH, pre_cell.dp)
            if cnsyn == 0:
                continue

            # construct connection
            for i, (id_pre_seg, id_post_seg) in enumerate(zip(pre_pid, post_pid)):
                category = ''
                if i < gap_n:
                    category = 'gj'            
                else:
                    category = 'syn'
                weight = np.exp(np.random.uniform(*np.log(WEIGHT_RANGE[category])))
                if category == 'syn':
                    weight *= np.sign(np.random.rand() - INH_PROP)
                new_connect = make_connect(circuit, id_pre_cell, id_pre_seg, id_post_cell, id_post_seg, category, weight)
                circuit.add_connect(new_connect)
                
    return circuit



# load Circuit (connection info) and construct connection of net
def load_connection(net: Network, circuit: Circuit):
    # net: a network
    # circuit: a structure of storing connections of network
    conn_id = 0
    flag = 0
    if len(net.connections):  # cells are connected
        flag = 1
    for post_neuron in circuit.neurons:
        id_post_cell = post_neuron.index
        if id_post_cell < 0:
            continue
        post_cell = net.index_cell_dict[id_post_cell]
        for connect in post_neuron.pre:
            id_pre_cell = connect.pre.index
            post_seg = post_cell.segs[connect.post_segment.index]
            cate = connect.category
            w = connect.weight
            if id_pre_cell > 0:
                pre_cell = net.index_cell_dict[id_pre_cell]
                pre_seg = pre_cell.segs[connect.pre_segment.index]
            else:
                pre_cell = None
                pre_seg = None
            if cate == 'gj':
                cate = 'gap_jun'
            elif cate == 'syn':
                cate = 'exc_syn' if w > 0 else 'inh_syn'
            if flag == 0:  # cells are not connected
                conn = Connection(pre_cell, pre_seg, post_cell, post_seg, cate, w)
                net.add_connection(conn)
            else: # cells has been connected, change the category and weight only
                conn = net.connections[conn_id]
                assert pre_cell == conn.pre_cell and pre_seg == conn.pre_segment
                assert post_cell == conn.post_cell and post_seg == conn.post_segment
                conn.change_conn_category(cate, w)
            conn_id += 1



def init_network(config: dict, circuit: Circuit, net=None):
    # config: config loaded from .json file, dictionary
    # path: the file path that the connection info loaded from, str
    # 
    # construct and initialize the network according to config
    cells_name_dic = config["cell_info"]["cells_name_dic"]
    cell_param_dir = config["dir_info"]["cell_param_dir"]
    # -------------------- construct cells --------------------
    if not net or len(net.cells) == 0:
        net = Network()
        for cell in circuit.neurons:
            id = int(cell.index)
            if id > 0:
                name = cells_name_dic[str(id)]
                cell_param = load_json(cell_param_dir + name + ".json")
                net.add_cell(Cell(index=id, name=name, cell_param=cell_param))
    # -------------------- construct connections -------------------- ]

    load_connection(net=net, circuit=circuit)
    
    return net



