"""
Modules containing function works for:
    1. circuit_test: Given single circuit (the connections and weights are fixed)
                     generate its output on randomly generated input
    2. circuit_search: Given only connection information, randomly sample the weights
    3. circuit_injected_stimulus: Given circuit and stimulus, inject the stimulus,
                                  simulate the circuit and generate its output

sim_config
    dt
    tstop
    v_init
test_config
    total_data_num
    file_data_num
    thread_num
    input_cell_names
    output_cell_names
    config_file_dir
data_config
    factory_name
    factory_args (except num, simulation_duration, dt)
        ...
        ...
        ...
weight_config
    syn
    gj
    inh_prob
"""

import os
import sys
import pymp
import numpy as np
import numpy.random as nr
from neuron import h
import _pickle as pickle
import json
from tqdm import tqdm
import os.path as path
from eworm.network.abstract_circuit import *
from eworm.network.detailed_circuit import *
from eworm.network.transform import *
from eworm.utils import *

sys.setrecursionlimit(1000000)


def circuit_test(abs_circuit, sim_config, save_dir, test_config=None, data_config=None):
    """generating circuit simulation result"""
    assert isinstance(abs_circuit, AbstractCircuit)
    # default setting
    test_config = {
        "total_data_num": 1000,
        "file_data_num": 100,
        "thread_num": 32,
        "input_cell_names": ["AWAL", "AWAR", "AWCL", "AWCR"],
        "output_cell_names": ['RIML', 'RMEL', 'RMED', 'RMDDL', 'RMDL', 'RMDVL', 'RIVL', 'SMDDL', 'SMDVL', 'SMBDL',
                              'SMBVL', 'RIMR', 'RMER', 'RMEV', 'RMDDR', 'RMDR', 'RMDVR', 'RIVR', 'SMDDR', 'SMDVR',
                              'SMBDR', 'SMBVR'],
        "config_file_dir": path.join(path.dirname(__file__), "config.json")
    } if test_config is None else test_config
    data_config = {
        "factory_name": "c302_data_factory",
        "args": {
            'pause_prob': 0.7,
            'initial_pause': 500,
            'noise_amp': 80,
            'smooth_sigma': 60,
        }
    } if data_config is None else data_config
    # preparation
    config = func.load_json(test_config['config_file_dir'])
    mode = test_config.get("mode", "active")
    circuit = abstract2detailed(abs_circuit, config, load_hoc=True, mode=mode)
    total_data_num, file_data_num = test_config['total_data_num'], test_config['file_data_num']
    in_num, out_num = len(test_config['input_cell_names']), len(test_config['output_cell_names'])
    trace_len = int(sim_config['tstop'] / sim_config['dt'])
    seeds = nr.randint(0, total_data_num*100, total_data_num)
    os.makedirs(save_dir, exist_ok=True)
    pickle.dump(abs_circuit, open(path.join(save_dir, "abs_circuit.pkl"), 'wb'), protocol=2)
    # generate data
    file_counter = 0
    while total_data_num > 0:
        gen_data_num = min(total_data_num, file_data_num)
        input_traces = pymp.shared.array([gen_data_num, in_num, trace_len], dtype='float32')
        output_traces = pymp.shared.array([gen_data_num, out_num, trace_len], dtype='float32')
        with pymp.Parallel(test_config['thread_num']) as p:
            for data_index in tqdm(p.range(gen_data_num)) if p.thread_num == 0 else p.range(gen_data_num):
                nr.seed(seeds[file_counter*file_data_num+data_index])
                input_trace = eval("data_factory."+data_config['factory_name'])(
                    num=in_num, tstop=sim_config['tstop'], dt=sim_config['dt'], **data_config['args'])
                output_trace = circuit.simulation(sim_config, input_trace, test_config["input_cell_names"],
                                                  test_config["output_cell_names"])
                input_traces[data_index], output_traces[data_index] = input_trace, output_trace

                if file_counter == 0 and data_index <= 20:
                    func.visualize_io_data(input_trace, output_trace,
                                      save_dir=path.join(save_dir, f"sample_{data_index}.jpg"),
                                      time_axis=np.linspace(sim_config['dt'], sim_config['tstop'], trace_len),
                                      input_label=test_config["input_cell_names"],
                                      output_label=test_config["output_cell_names"],
                                      x_label="Time (ms)", y_label="Voltage (mV)", title="Circuit Test")
        json.dump({"sim_config": sim_config,
                   "test_config": test_config,
                   "data_config": data_config,
                   "input_traces": input_traces.tolist(),
                   "output_traces": output_traces.tolist()},
                  open(path.join(save_dir, f"file_#{file_counter}.json"), 'w'))
        file_counter += 1
        total_data_num -= gen_data_num


def search_criterion(output_traces):
    """wash data: True -- qualified, False -- not qualified"""
    # if np.mean([np.var(trace[-int(len(trace)*0.2):]) for trace in output_traces]) <= 10:
    #     return False
    # if np.mean([np.var(trace[-int(len(trace)*0.8):-int(len(trace)*0.2)]) for trace in output_traces]) <= 50:
    #     return False
    return True


def circuit_search(sim_config, save_dir, search_config=None, data_config=None, weight_config=None, simplify_connection=False, cluster_connection=False, net_type="detailed"):
    """generating circuit simulation result
    net_type: "detailed", "point", "detailed_conn_in_soma"
    """
    # default setting
    search_config = {
        "total_sample": 16,
        "sim_per_sample": 1,
        "thread_num": 16,
        "input_cell_names": ["AWAL", "AWAR", "AWCL", "AWCR"],
        "output_cell_names": ["AWAL", "AWAR", "AWCL", "AWCR", 'RIML', 'RMEL', 'RMED', 'RMDDL', 'RMDL', 'RMDVL',
                              'RIVL', 'SMDDL', 'SMDVL', 'SMBDL', 'SMBVL', 'RIMR', 'RMER', 'RMEV', 'RMDDR', 'RMDR',
                              'RMDVR', 'RIVR', 'SMDDR', 'SMDVR', 'SMBDR', 'SMBVR'],
        "config_file_dir": path.join(path.dirname(__file__), "config.json")
    } if search_config is None else search_config
    data_config = {
        "factory_name": "c302_data_factory",
        "args": {
            'pause_prob': 0.7,
            'initial_pause': 500,
            'noise_amp': 80,
            'smooth_sigma': 60,
        }
    } if data_config is None else data_config
    weight_config = {
        "syn": (0.1, 10),
        "gj": (1e-4, 1e-3),
        "inh_prob": 0.3,
    } if weight_config is None else weight_config
    # preparation
    config = func.load_json(search_config['config_file_dir'])
    mode = search_config.get("mode", "active")
    del_circuit = config2detailed(config, search_config['input_cell_names'], load_hoc=True, mode=mode, simplify_connection=simplify_connection, cluster_connection=cluster_connection)
    circuit = detailed2abstract(del_circuit)
    del_circuit.release_hoc()
    connection_categories = [connect.category for connect in circuit.connections]
    connection_pair_keys = [connect.pair_key for connect in circuit.connections]
    total_sample, sim_per_sample, thread_num = search_config['total_sample'], search_config['sim_per_sample'], search_config['thread_num']
    in_num, out_num = len(search_config['input_cell_names']), len(search_config['output_cell_names'])
    trace_len = int(sim_config['tstop'] / sim_config['dt'])
    seeds = nr.randint(0, total_sample*100, total_sample)
    os.makedirs(save_dir, exist_ok=True)
    input_syn_weight = weight_config.get("input_syn_weight", 5)
    visualize_function = search_config.get("visualize_function", "visualize_io_data")
    # generate data
    json.dump({"sim_config": sim_config,
               "search_config": search_config,
               "data_config": data_config,
               "weight_config": weight_config,
               "config": config},
              open(path.join(save_dir, f"000_circuit_search_config.json"), 'w'),
              indent=2)
    with pymp.Parallel(thread_num) as p:
        for sample_index in p.range(total_sample):
            input_traces = np.zeros([sim_per_sample, in_num, trace_len], dtype='float32')
            output_traces = np.zeros([sim_per_sample, out_num, trace_len], dtype='float32')
            if net_type == 'detailed':
                new_circuit = abstract2detailed(circuit, config, load_hoc=False, mode=mode)
            elif net_type == 'point':
                new_circuit = abstract2point(circuit, config, load_hoc=False, mode=mode)
            elif net_type == 'detailed_conn_in_soma':
                new_circuit = abstract2detailedConnInSoma(circuit, config, load_hoc=False, mode=mode)
            nr.seed(seeds[sample_index])
            while True:  # set search max cycle to "for _ in range(50):"
                new_weights = func.weights_sample(weight_config, connection_categories, connection_pair_keys)
                new_circuit.update_connections(new_weights)
                for connect in new_circuit.input_connections:
                    connect.update_info(new_weight=input_syn_weight)
                for data_index in tqdm(range(sim_per_sample)) if p.thread_num == 0 else range(sim_per_sample):
                    input_trace = eval("data_factory."+data_config['factory_name'])(
                        num=in_num, tstop=sim_config['tstop'], dt=sim_config['dt'], **data_config['args'])
                    output_trace = new_circuit.simulation(
                        sim_config, input_trace, search_config["input_cell_names"], search_config["output_cell_names"])
                    input_traces[data_index], output_traces[data_index] = input_trace, output_trace
                # data washing
                qualified = True
                for data_index in range(sim_per_sample):
                    if not search_criterion(output_traces[data_index]):
                        qualified = False
                if qualified:
                    break
            for data_index in range(sim_per_sample):
                for func_i, func_name in enumerate(visualize_function):
                    eval("func."+func_name)(input_traces[data_index], output_traces[data_index],
                                            path.join(save_dir, f"sample_#{sample_index}_data_#{data_index}_{func_i}.jpg"),
                                            np.linspace(sim_config['dt'], sim_config['tstop'], trace_len),
                                            search_config["input_cell_names"], search_config["output_cell_names"],
                                            "Time (ms)", "Voltage (mV)", "Circuit Search", sim_config)
            pickle.dump(detailed2abstract(new_circuit),
                            open(path.join(save_dir, f"sample_#{sample_index}_circuit.pkl"), 'wb'), protocol=2)
            pickle.dump({"input_traces": input_traces.tolist(),
                         "output_traces": output_traces.tolist()},
                        open(path.join(save_dir, f"sample_#{sample_index}_data.pkl"), 'wb'),
                        protocol=2)
            # print(f"sample_#{sample_index} dumped!")


def circuit_injected_stimulus(circuit, stim_config, sim_config, output_cell_names):
    """inject a constant current/voltage stimulus and simulate the circuit

    Arguments:
    circuit: DetailedCircuit class, a neural circuit
    stim_config: a dictionary, stimulus configuration
    sim_config: a dictionay, simulation configuration
    output_cell_names: a list of string, names of output cells

    Here's an example of stim_config
    stim_config = {
        "AWAL": {
            "type": "current", # string
            "segment_id": 0,   # segment index
            "delay": 1000,     # (ms)
            "dur": 7000,       # (ms)
            "amp": 2*0.001     # (nA)
        },
        "AWCR": {
            "type": "voltage", # string
            "segment_id": 0,   # segment index
            "dur1": 1000,      # (ms) resting time before stimulus
            "dur2": 6000,      # (ms) stimulus time
            "dur3": 1000,      # (ms) resting after stimulus
            "amp1": -60,       # (mV) stimulus amplitude in dur1
            "amp2": 20,        # (mV) stimulus amplitude in dur2
            "amp3": -60,       # (mV) stimulus amplitude in dur3
            "rs": 20           # (MOhm)
        }
    }
    """
    assert isinstance(circuit, DetailedCircuit)
    assert circuit.cells[0].rec_v
    # add stimulus
    h.dt, h.steps_per_ms, h.tstop = sim_config['dt'], int(1 / sim_config['dt']), sim_config['tstop']
    h.secondorder = sim_config['secondorder']
    h.finitialize(sim_config['v_init'])
    stimulus = []
    for cell_name, stim_set in stim_config.items():
        cell = circuit.cell(cell_name=cell_name)
        if stim_set["type"] == "current":
            stim = h.IClamp(cell.segments[stim_set["segment_id"]].hoc_obj)
            stim.delay = stim_set["delay"]
            stim.dur = stim_set["dur"]
            stim.amp = stim_set["amp"]
            stimulus.append(stim)
        elif stim_set["type"] == "voltage":
            stim = h.SEClamp(cell.segments[stim_set["segment_id"]].hoc_obj)
            stim.dur1 = stim_set["dur1"]
            stim.dur2 = stim_set["dur2"]
            stim.dur3 = stim_set["dur3"]
            stim.amp1 = stim_set["amp1"]
            stim.amp2 = stim_set["amp2"]
            stim.amp3 = stim_set["amp3"]
            stim.rs = stim_set["rs"]
            stim.append(stim)
        else:
            raise ValueError
    # simulate the circuit
    h.run(h.tstop)
    # save voltage of output neurons
    if len(output_cell_names) == 0:
        return None
    output_traces = np.zeros((len(output_cell_names),\
        int(sim_config['tstop'] / sim_config['dt']) + 1), dtype=np.float32)
    for i, cell_name in enumerate(output_cell_names):
        output_traces[i] = circuit.cell(cell_name=cell_name).segments[0].voltage.as_numpy()
    return output_traces
