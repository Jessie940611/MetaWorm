"""
Modules containing function works for transformation between config,
DetailedCircuit, AbstractCircuit, PointCircuit


config ---> DetailedCircuit: config2detailed
DetailedCircuit ---> AbstractCircuit: detailed2abstract
                ---> PointCircuit: detailed2point
                ---> DetailedCircuit (connection only on soma): detailed2detailedConInSoma
AbstractCircuit ---> DetailedCircuit: abstract2detailed
                ---> PointCircuit: abstract2point
                ---> DetailedCircuit (connection only on soma): abstract2detailedConnInSoma
"""
import os
from eworm.network.abstract_circuit import *
from eworm.network.detailed_circuit import *
from eworm.network.point_circuit import *
from neuron import h, load_mechanisms
from eworm.utils import *
import numpy as np
import _pickle as pickle
from scipy import stats
import os.path as path
from tqdm import tqdm
import pymp
import math
import time


def find_nearest(array, value):
    tmp = np.abs(array - np.sum(value)).flatten()
    mins = np.where(tmp < 1)
    if len(mins[0]) == 0:
        mins = np.where(tmp == tmp.min())
    index = np.unravel_index(mins[0][np.random.choice(len(mins[0]), size=1)[0]], shape=array.shape)
    nearest_value = np.abs(array - value)[index]
    return index, nearest_value

def find_nearest_cluster(array, values):
    """ find a cluster of location which is most qualified
    array: distance matrix
    values: required distances
    """
    nsyn = len(values)
    nrow, ncol = array.shape
    cal_array = np.zeros((nrow+nsyn-1, ncol+nsyn-1))
    cal_array[:nrow, :ncol] = array
    for i in range(1, nsyn):
        shift_array = np.zeros((nrow+nsyn-1, ncol+nsyn-1))
        shift_array[i:nrow+i, i:ncol+i] = array
        cal_array += shift_array
    cal_array = cal_array[:nrow, :ncol]
    cal_array[:nsyn-1, :] = 1000
    cal_array[:, :nsyn-1] = 1000
    tmp = np.abs(cal_array - np.sum(values)).flatten()
    mins = np.where(tmp < 1)
    if len(mins[0]) == 0:
        mins = np.where(tmp == tmp.min())
    index = np.unravel_index(mins[0][np.random.choice(len(mins[0]), size=1)[0]], shape=cal_array.shape)
    nearest_value = np.abs(cal_array - np.sum(values))[index]
    return index, nearest_value



def search_satisfied_distance(required_distances, distance_matrix, thresh,
                              found_list, cell_start, cell_end, cluster_connection):
    nsyn = len(required_distances)
    if not cluster_connection:
        for required_distance in required_distances:
            found_index, found_distance = find_nearest(distance_matrix, required_distance)
            if found_distance <= thresh:
                found_list.append((cell_start, cell_end, found_index))
    else:
        found_index, found_distance = find_nearest_cluster(distance_matrix, required_distances)
        if found_distance <= thresh * len(required_distances):
            for i in range(0, nsyn):
                found_list.append((cell_start, cell_end, (found_index[0]-i, found_index[1]-i)))


def calculate_distance_matrix(circuit, gj_matrix, syn_matrix, save_dir, thread_num=32, refresh=False):
    """calculate available distance choices matrix between all possible connection"""
    assert isinstance(circuit, DetailedCircuit)
    os.makedirs(save_dir, exist_ok=True)
    # with pymp.Parallel(thread_num) as p:
        # for cell_0_index in tqdm(p.range(len(circuit.cells) - 1)) if p.thread_num == 0 else p.range(
        #         len(circuit.cells) - 1):
    for cell_0_index in tqdm(range(len(circuit.cells) - 1)):
        if cell_0_index >= 302:
            continue
        cell_0 = circuit.cells[cell_0_index]
        n_segment_0 = len(cell_0.segments)
        midpoints_0 = np.array([seg.location for seg in cell_0.segments])  # shape (n_segment_0, 3)
        diameters_0 = np.array([seg.diameter for seg in cell_0.segments])  # shape (n_segment_0,)
        for cell_1 in circuit.cells[cell_0_index + 1:]:
            if cell_1.is_muscle:
                continue
            if len(str(gj_matrix.cell_value(cell_0.index + 1, cell_1.index + 1))) == 0 and \
                    len(str(syn_matrix.cell_value(cell_0.index + 1, cell_1.index + 1))) == 0 and \
                    len(str(syn_matrix.cell_value(cell_1.index + 1, cell_0.index + 1))) == 0:
                continue
            pkl_name = str(min(cell_0.index, cell_1.index)) + "_" + str(max(cell_0.index, cell_1.index)) + ".pkl"
            if not refresh and path.exists(path.join(save_dir, pkl_name)):
                continue
            n_segment_1 = len(cell_1.segments)
            midpoints_1 = np.array([seg.location for seg in cell_1.segments])  # shape (n_segment_1, 3)
            diameters_1 = np.array([seg.diameter for seg in cell_1.segments])  # shape (n_segment_1,)

            distance_matrix = np.linalg.norm(
                midpoints_0.reshape((n_segment_0, 1, 3)).repeat(n_segment_1, axis=1) -
                midpoints_1.reshape((1, n_segment_1, 3)).repeat(n_segment_0, axis=0), axis=-1
            ) - diameters_0.reshape((n_segment_0, 1)) - diameters_1.reshape((1, n_segment_1))
            # calibrate the shape of the matrix
            if cell_0.index > cell_1.index:
                distance_matrix = distance_matrix.transpose()
            distance_matrix[distance_matrix < 0] = 0
            pickle.dump(distance_matrix, open(path.join(save_dir, pkl_name), 'wb'))


def config2detailed(config, input_cell_names=None, output_cell_names=None, load_hoc=True,
                    rec_voltage=False, mode="active", simplify_connection=False, cluster_connection=False, seed=None):
    """
    construct DetailedCircuit class from config file, randomly generate its connection setting

    Args:
        config: config file loaded from .json file
            -- "cell_info": information about cell index and name
            -- "dir_info": information about directory path to data
            -- "cnt_info": information about connection construction parameter
        input_cell_names: list of input cell names
        output_cell_names: list of output cell names
        load_hoc: True for hoc file loading
        rec_voltage: True for recording voltage in Segment
        mode: "active" for active soma, "passive" for passive soma
        simplify_connection: True for connection simplification (for one directed connection
                             between two neurons, only one synapse/gap junction exists)
        cluster_connection: If True, for each pair of neurons, the synapses (has the same direction)
                            clustered together
    """
    if seed:
        np.random.seed(seed)
    cell_info, dir_info, cnt_info = config['cell_info'], config['dir_info'], config['cnt_info']
    # load hoc file
    if load_hoc:
        h.load_file('stdrun.hoc')
        load_mechanisms(path.join(os.path.dirname(__file__), "..", dir_info['mechanism_dir']))
    circuit = DetailedCircuit()
    # make cells
    for cell_index in cell_info['cells_id_sim']:
        cell_name = cell_info['cells_name_dic'][cell_index]
        if load_hoc:
            h.load_file(path.join(os.path.dirname(__file__), "..", dir_info['model_dir'], cell_name + '.hoc'))
        cell_param = func.load_json(path.join(os.path.dirname(__file__), "..", dir_info['cell_param_dir'],
                                              cell_name + '.json'))
        is_muscle = True if int(cell_index) >= 302 else False
        circuit.add_cell(Cell(cell_index, cell_name, cell_param, rec_voltage, mode, cnt_info["length_per_seg"], is_muscle))
    # prepare distance matrix
    syn_matrix, _, _ = func.read_excel(file_name=path.join(os.path.dirname(__file__), "..", dir_info['adj_matrix_dir']),
                                       sheet_name=dir_info['synapse_sheet'])
    gj_matrix, _, _ = func.read_excel(file_name=path.join(os.path.dirname(__file__), "..", dir_info['adj_matrix_dir']),
                                      sheet_name=dir_info['gap_junction_sheet'])
    pickle_dir = path.join(path.dirname(__file__), "output", "segment_distances_"+str(cnt_info["length_per_seg"]))
    calculate_distance_matrix(circuit, gj_matrix, syn_matrix, save_dir=pickle_dir, thread_num=1)

    has_muscle_cell = False
    # assign gap junction (not clustered)
    start_time_construct_gj = time.time()
    found_gap_junction = []
    for cell_0_index in range(len(circuit.cells)-1):
        cell_0 = circuit.cells[cell_0_index]
        if cell_0.is_muscle:
            has_muscle_cell = True
            continue
        for cell_1 in circuit.cells[cell_0_index + 1:]:
            if cell_1.is_muscle:
                has_muscle_cell = True
                continue
            gj_num = gj_matrix.cell_value(cell_0.index + 1, cell_1.index + 1)
            if len(str(gj_num)) == 0:
                continue
            pkl_name = str(min(cell_0.index, cell_1.index)) + "_" + str(max(cell_0.index, cell_1.index)) + ".pkl"
            distance_matrix = pickle.load(open(path.join(pickle_dir, pkl_name), "rb"))
            # calibrate the shape of the matrix
            if cell_0.index > cell_1.index:
                distance_matrix = distance_matrix.transpose()

            # search the satisfied distance
            gj_num = round(cnt_info['gj_a'] * math.tanh(cnt_info['gj_b'] * gj_num) * cnt_info["cnt_scale"])
            if gj_num == 0 or simplify_connection:
                gj_num = 1
            gj_required = stats.invgauss.rvs(mu=cnt_info['gj_mu'], loc=0, scale=cnt_info['gj_scale'], size=gj_num)
            search_satisfied_distance(gj_required, distance_matrix, thresh=cnt_info['thresh'], found_list=found_gap_junction,
                                      cell_start=cell_0, cell_end=cell_1, cluster_connection=False)
    pair_key = 0
    for cell_0, cell_1, (segment_0_index, segment_1_index) in found_gap_junction:
        gj_weight = np.exp(np.random.uniform(*np.log(cnt_info['weight_range']['gj'])))
        circuit.add_connection(Connection(cell_0.segment(segment_0_index), cell_1.segment(segment_1_index), 'gj',
                                          gj_weight, pair_key))
        circuit.add_connection(Connection(cell_1.segment(segment_1_index), cell_0.segment(segment_0_index), 'gj',
                                          gj_weight, pair_key))
        pair_key += 1
    print(f"It takes {time.time()-start_time_construct_gj:.2f}s to construct gap junction!")

    # assign synapse (clustered)
    start_time_construct_syn = time.time()
    found_synapse = []
    for cell_0 in circuit.cells:
        if cell_0.is_muscle:
            continue
        for cell_1 in circuit.cells:
            if cell_1.is_muscle:
                continue
            syn_num = syn_matrix.cell_value(cell_0.index + 1, cell_1.index + 1)
            if len(str(syn_num)) == 0 or cell_0.index == cell_1.index:
                continue
            pkl_name = str(min(cell_0.index, cell_1.index)) + "_" + str(max(cell_0.index, cell_1.index)) + ".pkl"
            distance_matrix = pickle.load(open(path.join(pickle_dir, pkl_name), "rb"))
            syn_num = round(cnt_info['syn_a'] * math.tanh(cnt_info['syn_b'] * syn_num * cnt_info["cnt_scale"]))
            if syn_num == 0 or simplify_connection:
                syn_num = 1
            # calibrate the shape of the matrix
            if cell_0.index > cell_1.index:
                distance_matrix = distance_matrix.transpose()

            # detach the segment that already have gap junction connection
            for segment_id, segment_x in enumerate(cell_0.segments):
                if len(segment_x.pre_connections) + len(segment_x.post_connections) > 0:
                    distance_matrix[segment_id, :] = 1000
            for segment_id, segment_y in enumerate(cell_1.segments):
                if len(segment_y.pre_connections) + len(segment_y.post_connections) > 0:
                    distance_matrix[:, segment_id] = 1000
            # search the satisfied distance
            syn_required = stats.invgauss.rvs(mu=cnt_info['syn_mu'], loc=0,
                                              scale=cnt_info['syn_scale'], size=syn_num)
            search_satisfied_distance(syn_required, distance_matrix, thresh=cnt_info['thresh'], found_list=found_synapse,
                                      cell_start=cell_0, cell_end=cell_1, cluster_connection=cluster_connection)

    for cell_0, cell_1, (segment_0_index, segment_1_index) in found_synapse:
        syn_weight = np.exp(np.random.uniform(*np.log(cnt_info['weight_range']['syn'])))
        syn_weight *= np.sign(np.random.rand() - cnt_info['inh_prob'])
        circuit.add_connection(Connection(cell_0.segment(segment_0_index), cell_1.segment(segment_1_index), 'syn',
                                          syn_weight))
    for cell_0 in circuit.cells:
        if (input_cell_names is not None) and (cell_0.name in input_cell_names):
            circuit.add_connection(Connection(None, cell_0.segment(0), 'syn', 5.))
        if (output_cell_names is not None) and (cell_0.name in output_cell_names):
            circuit.add_connection(Connection(cell_0.segment(0), None, 'syn', 1.))
    

    if has_muscle_cell:
        # connection between neuron and muscle cells
        # neuron to muscle
        neuron_muscle_matrix, nrow, ncol = func.read_excel(file_name=path.join(os.path.dirname(__file__), "..", dir_info['neuron_muscle_con']), sheet_name='Sheet1')
        neuron_names = neuron_muscle_matrix.col_values(0)
        muscle_names = neuron_muscle_matrix.row_values(0)
        for i in range(1, nrow):
            for j in range(1, ncol):
                conn_flag = neuron_muscle_matrix.cell_value(i, j)
                if len(str(conn_flag)) > 0 and int(conn_flag) == 1:
                    try:
                        pre_seg = circuit.cell(cell_name=neuron_names[i]).segment(0)
                        post_seg = circuit.cell(cell_name=f"MC{muscle_names[j]}").segment(0)
                        circuit.add_connection(Connection(pre_seg, post_seg, 'syn', 1))
                    except:
                        continue

        # muscle to neuron
        muscle_neuron_matrix, nrow, ncol = func.read_excel(file_name=path.join(os.path.dirname(__file__), "..", dir_info['muscle_neuron_con']), sheet_name='Sheet1')
        muscle_names = muscle_neuron_matrix.col_values(0)
        neuron_names = muscle_neuron_matrix.row_values(0)
        for i in range(1, nrow):
            for j in range(1, ncol):
                conn_flag = muscle_neuron_matrix.cell_value(i, j)
                if len(str(conn_flag)) > 0 and int(conn_flag) == 1:
                    try:
                        pre_seg = circuit.cell(cell_name=f"MC{muscle_names[i]}").segment(0)
                        post_seg = circuit.cell(cell_name=neuron_names[j]).segment(0)
                        circuit.add_connection(Connection(pre_seg, post_seg, 'syn', 0.1))
                    except:
                        continue
    print(f"It takes {time.time()-start_time_construct_syn:.2f}s to construct synapse!")

    return circuit


def detailed2abstract(circuit):
    """extract the intrinsic AbsCircuit from given DetailedCircuit"""
    assert isinstance(circuit, (DetailedCircuit, PointCircuit))
    abs_circuit = AbstractCircuit()
    for detailed_cell in circuit.cells:
        abs_cell = AbsCell(index=detailed_cell.index, name=detailed_cell.name)
        for detailed_segment in detailed_cell.segments:
            abs_cell.add_segment(AbsSegment(index=detailed_segment.index, cell=abs_cell, name=detailed_segment.name))
        abs_circuit.add_cell(abs_cell)
    for detailed_connection in circuit.connections:
        if detailed_connection.pre_segment is None:
            abs_pre_segment = None
        else:
            pre_cell = abs_circuit.cell(detailed_connection.pre_cell.index)
            abs_pre_segment = pre_cell.segment(detailed_connection.pre_segment.index)
        if detailed_connection.post_segment is None:
            abs_post_segment = None
        else:
            post_cell = abs_circuit.cell(detailed_connection.post_cell.index)
            abs_post_segment = post_cell.segment(detailed_connection.post_segment.index)
        abs_circuit.add_connection(AbsConnection(pre_segment=abs_pre_segment,
            post_segment=abs_post_segment, category=detailed_connection.category,
            weight=detailed_connection.weight, pair_key=detailed_connection.pair_key))

    return abs_circuit


def abstract2detailed(abs_circuit, config, load_hoc=True, rec_voltage=False, mode="active"):
    """create DetailedCircuit and load connection setting from given AbsCircuit"""
    # assert isinstance(abs_circuit, AbstractCircuit)
    dir_info = config['dir_info']
    if load_hoc:
        h.load_file('stdrun.hoc')
        load_mechanisms(path.join(os.path.dirname(__file__), "..", dir_info['mechanism_dir']))
    circuit = DetailedCircuit()
    for abs_cell in abs_circuit.cells:
        cell_index, cell_name = abs_cell.index, abs_cell.name
        try:
            cell_param = func.load_json(path.join(os.path.dirname(__file__), "..", dir_info['cell_param_dir'], cell_name + '.json'))
        except FileNotFoundError:
            print(f"Index {cell_index} Name {cell_name} Cell Parameter Not Found!")
        else:
            if load_hoc:
                h.load_file(path.join(os.path.dirname(__file__), "..", dir_info['model_dir'], cell_name + '.hoc'))
            is_muscle = True if cell_index >= 302 else False
            circuit.add_cell(Cell(cell_index, cell_name, cell_param, rec_voltage, mode, is_muscle=is_muscle))
    for abstract_connection in abs_circuit.connections:
        if abstract_connection.pre_segment is None:
            detail_pre_segment = None
        else:
            pre_cell = circuit.cell(abstract_connection.pre_cell.index)
            detail_pre_segment = pre_cell.segment(abstract_connection.pre_segment.index)
        if abstract_connection.post_segment is None:
            detail_post_segment = None
        else:
            post_cell = circuit.cell(abstract_connection.post_cell.index)
            detail_post_segment = post_cell.segment(abstract_connection.post_segment.index)
        circuit.add_connection(Connection(pre_segment=detail_pre_segment, post_segment=detail_post_segment,
                                          category=abstract_connection.category, weight=abstract_connection.weight,
                                          pair_key=abstract_connection.pair_key))
    return circuit


def detailed2point(circuit):
    """transform a given DetailedCircuit to PointCircuit"""
    assert isinstance(circuit, DetailedCircuit)
    point_circuit = PointCircuit()
    for detailed_cell in circuit.cells:
        point_cell = PointCell(index=detailed_cell.index, name=detailed_cell.name,\
            cell_param=detailed_cell.cell_param, rec_voltage=detailed_cell.rec_v, mode=detailed_cell.mode)
        point_circuit.add_cell(point_cell)
    for detailed_connection in circuit.connections:
        if detailed_connection.pre_segment is None:
            point_pre_segment = None
        else:
            pre_cell = point_circuit.cell(detailed_connection.pre_cell.index)
            point_pre_segment = pre_cell.segments[0]
        if detailed_connection.post_segment is None:
            point_post_segment = None
        else:
            post_cell = point_circuit.cell(detailed_connection.post_cell.index)
            point_post_segment = post_cell.segments[0]
        point_circuit.add_connection(Connection(pre_segment=point_pre_segment,
            post_segment=point_post_segment, category=detailed_connection.category,
            weight=detailed_connection.weight, pair_key=detailed_connection.pair_key))
    return point_circuit


def abstract2point(abs_circuit, config, load_hoc=True, rec_voltage=False, mode="active"):
    """create PointCircuit and load connection setting from given AbsCircuit"""
    # assert isinstance(abs_circuit, AbstractCircuit)
    dir_info = config['dir_info']
    if load_hoc:
        h.load_file('stdrun.hoc')
        load_mechanisms(path.join(os.path.dirname(__file__), "..", dir_info['mechanism_dir']))
    circuit = PointCircuit()
    for abs_cell in abs_circuit.cells:
        cell_index, cell_name = abs_cell.index, abs_cell.name
        try:
            cell_param = func.load_json(path.join(os.path.dirname(__file__), "..", dir_info['cell_param_dir'], cell_name + '.json'))
        except FileNotFoundError:
            print(f"Index {cell_index} Name {cell_name} Cell Parameter Not Found!")
        else:
            if load_hoc:
                h.load_file(path.join(os.path.dirname(__file__), "..", dir_info['model_dir'], cell_name + '.hoc'))
            circuit.add_cell(PointCell(cell_index, cell_name, cell_param, rec_voltage, mode))
    for abstract_connection in abs_circuit.connections:
        if abstract_connection.pre_segment is None:
            detail_pre_segment = None
        else:
            pre_cell = circuit.cell(abstract_connection.pre_cell.index)
            detail_pre_segment = pre_cell.segments[0]
        if abstract_connection.post_segment is None:
            detail_post_segment = None
        else:
            post_cell = circuit.cell(abstract_connection.post_cell.index)
            detail_post_segment = post_cell.segments[0]
        circuit.add_connection(Connection(pre_segment=detail_pre_segment,
            post_segment=detail_post_segment, category=abstract_connection.category,
            weight=abstract_connection.weight, pair_key=abstract_connection.pair_key))
    return circuit


def detailed2detailedConnInSoma(circuit):
    """transform a given DetailedCircuit to DetailedCircuit, where connections are located on soma"""
    assert isinstance(circuit, DetailedCircuit)
    soma_con_circuit = DetailedCircuit()
    for detailed_cell in circuit.cells:
        cell = Cell(index=detailed_cell.index, name=detailed_cell.name,\
            cell_param=detailed_cell.cell_param, rec_voltage=detailed_cell.rec_v, mode=detailed_cell.mode, is_muscle=detailed_cell.is_muscle)
        soma_con_circuit.add_cell(cell)
    for detailed_connection in circuit.connections:
        if detailed_connection.pre_segment is None:
            point_pre_segment = None
        else:
            pre_cell = soma_con_circuit.cell(detailed_connection.pre_cell.index)
            point_pre_segment = pre_cell.segments[0]
        if detailed_connection.post_segment is None:
            point_post_segment = None
        else:
            post_cell = soma_con_circuit.cell(detailed_connection.post_cell.index)
            point_post_segment = post_cell.segments[0]
        soma_con_circuit.add_connection(Connection(pre_segment=point_pre_segment,
            post_segment=point_post_segment, category=detailed_connection.category,
            weight=detailed_connection.weight, pair_key=detailed_connection.pair_key))
    return soma_con_circuit


def abstract2detailedConnInSoma(abs_circuit, config, load_hoc=True, rec_voltage=False, mode="active"):
    """create Circuit (connection only on soma) and load connection setting from given AbsCircuit"""
    # assert isinstance(abs_circuit, AbstractCircuit)
    dir_info = config['dir_info']
    if load_hoc:
        h.load_file('stdrun.hoc')
        load_mechanisms(path.join(os.path.dirname(__file__), "..", dir_info['mechanism_dir']))
    circuit = DetailedCircuit()
    for abs_cell in abs_circuit.cells:
        cell_index, cell_name = abs_cell.index, abs_cell.name
        try:
            cell_param = func.load_json(path.join(os.path.dirname(__file__), "..", dir_info['cell_param_dir'], cell_name + '.json'))
        except FileNotFoundError:
            print(f"Index {cell_index} Name {cell_name} Cell Parameter Not Found!")
        else:
            if load_hoc:
                h.load_file(path.join(os.path.dirname(__file__), "..", dir_info['model_dir'], cell_name + '.hoc'))
            is_muscle = True if cell_index >= 302 else False
            circuit.add_cell(Cell(cell_index, cell_name, cell_param, rec_voltage, mode, is_muscle=is_muscle))
    for abstract_connection in abs_circuit.connections:
        if abstract_connection.pre_segment is None:
            detail_pre_segment = None
        else:
            pre_cell = circuit.cell(abstract_connection.pre_cell.index)
            detail_pre_segment = pre_cell.segments[0]
        if abstract_connection.post_segment is None:
            detail_post_segment = None
        else:
            post_cell = circuit.cell(abstract_connection.post_cell.index)
            detail_post_segment = post_cell.segments[0]
        circuit.add_connection(Connection(pre_segment=detail_pre_segment,
            post_segment=detail_post_segment, category=abstract_connection.category,
            weight=abstract_connection.weight, pair_key=abstract_connection.pair_key))
    return circuit


def select_cell(circuit, config=None, cell_index=None, cell_name=None, load_hoc=True, rec_voltage=False, mode="active"):
    """create a single-cell DetailedCircuit from given circuit"""
    assert isinstance(circuit, (AbstractCircuit, DetailedCircuit))
    assert (config is not None) or (isinstance(circuit, DetailedCircuit))
    single_cell_circuit = DetailedCircuit()
    reference_cell = circuit.cell(cell_index, cell_name)
    cell_index, cell_name = reference_cell.index, reference_cell.name
    if isinstance(circuit, DetailedCircuit):
        cell_param = reference_cell.cell_param
    else:
        dir_info = config['dir_info']
        if load_hoc:
            h.load_file('stdrun.hoc')
            load_mechanisms(path.join(os.path.dirname(__file__), "..", dir_info['mechanism_dir']))
            h.load_file(path.join(os.path.dirname(__file__), "..", dir_info['model_dir'], cell_name + '.hoc'))
        cell_param = func.load_json(
            path.join(os.path.dirname(__file__), "..", dir_info['cell_param_dir'], cell_name + '.json'))
    selected_cell = Cell(cell_index, cell_name, cell_param, rec_voltage, mode)
    single_cell_circuit.add_cell(selected_cell)
    for pre_connection in reference_cell.pre_connections:
        connect_segment = selected_cell.segment(pre_connection.post_segment.index)
        single_cell_circuit.add_connection(Connection(pre_segment=None, post_segment=connect_segment,
                                                      category=pre_connection.category,
                                                      weight=pre_connection.weight, pair_key=pre_connection.pair_key))
    for post_connection in reference_cell.post_connections:
        connect_segment = selected_cell.segment(post_connection.pre_segment.index)
        single_cell_circuit.add_connection(Connection(pre_segment=connect_segment, post_segment=None,
                                                      category=post_connection.category,
                                                      weight=post_connection.weight, pair_key=post_connection.pair_key))
    return selected_cell, single_cell_circuit



def abstract2detailed_shuffle_location(abs_circuit, config, load_hoc=True, rec_voltage=False, mode="active"):
    """create DetailedCircuit and load connection setting from given AbsCircuit"""
    # assert isinstance(abs_circuit, AbstractCircuit)
    dir_info = config['dir_info']
    if load_hoc:
        h.load_file('stdrun.hoc')
        load_mechanisms(path.join(os.path.dirname(__file__), "..", dir_info['mechanism_dir']))
    circuit = DetailedCircuit()
    for abs_cell in abs_circuit.cells:
        cell_index, cell_name = abs_cell.index, abs_cell.name
        try:
            cell_param = func.load_json(path.join(os.path.dirname(__file__), "..", dir_info['cell_param_dir'], cell_name + '.json'))
        except FileNotFoundError:
            print(f"Index {cell_index} Name {cell_name} Cell Parameter Not Found!")
        else:
            if load_hoc:
                h.load_file(path.join(os.path.dirname(__file__), "..", dir_info['model_dir'], cell_name + '.hoc'))
            is_muscle = True if cell_index >= 302 else False
            circuit.add_cell(Cell(cell_index, cell_name, cell_param, rec_voltage, mode, is_muscle=is_muscle))
    for abstract_connection in abs_circuit.connections:
        if abstract_connection.pre_segment is None:
            detail_pre_segment = None
        else:
            pre_cell = circuit.cell(abstract_connection.pre_cell.index)
            detail_pre_segment = pre_cell.segment(np.random.randint(len(pre_cell.segments)))
            # detail_pre_segment = pre_cell.segment(abstract_connection.pre_segment.index)
        if abstract_connection.post_segment is None:
            detail_post_segment = None
        else:
            post_cell = circuit.cell(abstract_connection.post_cell.index)
            detail_post_segment = post_cell.segment(np.random.randint(len(post_cell.segments)))
        circuit.add_connection(Connection(pre_segment=detail_pre_segment, post_segment=detail_post_segment,
                                          category=abstract_connection.category, weight=abstract_connection.weight,
                                          pair_key=abstract_connection.pair_key))
    return circuit


def abstract2detailed_shuffle_weight(abs_circuit, config, load_hoc=True, rec_voltage=False, mode="active", shuffle_target="gj"):
    """create DetailedCircuit and load connection setting from given AbsCircuit"""
    # assert isinstance(abs_circuit, AbstractCircuit)
    dir_info = config['dir_info']
    if load_hoc:
        h.load_file('stdrun.hoc')
        load_mechanisms(path.join(os.path.dirname(__file__), "..", dir_info['mechanism_dir']))
    circuit = DetailedCircuit()
    for abs_cell in abs_circuit.cells:
        cell_index, cell_name = abs_cell.index, abs_cell.name
        try:
            cell_param = func.load_json(path.join(os.path.dirname(__file__), "..", dir_info['cell_param_dir'], cell_name + '.json'))
        except FileNotFoundError:
            print(f"Index {cell_index} Name {cell_name} Cell Parameter Not Found!")
        else:
            if load_hoc:
                h.load_file(path.join(os.path.dirname(__file__), "..", dir_info['model_dir'], cell_name + '.hoc'))
            is_muscle = True if cell_index >= 302 else False
            circuit.add_cell(Cell(cell_index, cell_name, cell_param, rec_voltage, mode, is_muscle=is_muscle))
    weights = []
    for conn in abs_circuit.connections:
        if conn.category == shuffle_target:
            weights.append(conn.weight)
    weights = np.array(weights)
    np.random.shuffle(weights)
    cnt = 0
    for abstract_connection in abs_circuit.connections:
        if abstract_connection.pre_segment is None:
            detail_pre_segment = None
        else:
            pre_cell = circuit.cell(abstract_connection.pre_cell.index)
            detail_pre_segment = pre_cell.segment(abstract_connection.pre_segment.index)
        if abstract_connection.post_segment is None:
            detail_post_segment = None
        else:
            post_cell = circuit.cell(abstract_connection.post_cell.index)
            detail_post_segment = post_cell.segment(abstract_connection.post_segment.index)
        if abstract_connection.category == shuffle_target:
            circuit.add_connection(Connection(pre_segment=detail_pre_segment, post_segment=detail_post_segment,
                                          category=abstract_connection.category, weight=weights[cnt],
                                          pair_key=abstract_connection.pair_key))
            cnt += 1
        else:
            circuit.add_connection(Connection(pre_segment=detail_pre_segment, post_segment=detail_post_segment,
                                          category=abstract_connection.category, weight=abstract_connection.weight,
                                          pair_key=abstract_connection.pair_key))
    return circuit


def abstract2detailed_remove(abs_circuit, config, load_hoc=True, rec_voltage=False, mode="active", remove_target="gj"):
    """create DetailedCircuit and load connection setting from given AbsCircuit"""
    # assert isinstance(abs_circuit, AbstractCircuit)
    dir_info = config['dir_info']
    if load_hoc:
        h.load_file('stdrun.hoc')
        load_mechanisms(path.join(os.path.dirname(__file__), "..", dir_info['mechanism_dir']))
    circuit = DetailedCircuit()
    for abs_cell in abs_circuit.cells:
        cell_index, cell_name = abs_cell.index, abs_cell.name
        try:
            cell_param = func.load_json(path.join(os.path.dirname(__file__), "..", dir_info['cell_param_dir'], cell_name + '.json'))
        except FileNotFoundError:
            print(f"Index {cell_index} Name {cell_name} Cell Parameter Not Found!")
        else:
            if load_hoc:
                h.load_file(path.join(os.path.dirname(__file__), "..", dir_info['model_dir'], cell_name + '.hoc'))
            is_muscle = True if cell_index >= 302 else False
            circuit.add_cell(Cell(cell_index, cell_name, cell_param, rec_voltage, mode, is_muscle=is_muscle))
    for abstract_connection in abs_circuit.connections:
        if abstract_connection.category == remove_target:
            continue
        if abstract_connection.pre_segment is None:
            detail_pre_segment = None
        else:
            pre_cell = circuit.cell(abstract_connection.pre_cell.index)
            detail_pre_segment = pre_cell.segment(abstract_connection.pre_segment.index)
        if abstract_connection.post_segment is None:
            detail_post_segment = None
        else:
            post_cell = circuit.cell(abstract_connection.post_cell.index)
            detail_post_segment = post_cell.segment(abstract_connection.post_segment.index)
        circuit.add_connection(Connection(pre_segment=detail_pre_segment, post_segment=detail_post_segment,
                                          category=abstract_connection.category, weight=abstract_connection.weight,
                                          pair_key=abstract_connection.pair_key))
    return circuit