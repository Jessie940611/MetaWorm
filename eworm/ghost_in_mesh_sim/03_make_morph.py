"""only modified by brains"""

from tokenize import group
import os.path as path
import numpy as np
from eworm.utils import *
from eworm.network import *
from eworm.network.process import *
import pickle
import time

if __name__ == "__main__":
    np.random.seed(42)
    group_name = "reservoir"
    config_dir = path.join(path.dirname(__file__), "output", group_name, f"{group_name}_config.json")
    abs_circuit_file = path.join(path.dirname(__file__), "output", group_name, f"{group_name}_abscircuit.pkl")
    
    sim_config = {"dt": 5/3, "tstop": 1000, "v_init": -65, "secondorder": 0}
    config = func.load_json(config_dir)["config"]
    abs_circuit = pickle.load(open(abs_circuit_file, 'rb'))
    circuit = transform.abstract2detailed(abs_circuit, config, load_hoc=True, rec_voltage=True)
    input_cell_name = ["AWAL", "AWAR", "AWCL", "AWCR"]
    loc_path = path.join("data", "state", "worm_states_300_220428-152601.json")
    input_traces = data_factory.ghost_in_mesh_data_factory(len(input_cell_name), sim_config["tstop"], sim_config["dt"], loc_path)
    simulation_start_time = time.time()
    output_traces = circuit.simulation(sim_config, input_traces, input_cell_name, [])
    simulation_end_time = time.time()
    print(f"simulation takes {simulation_end_time - simulation_start_time} s.")
    # save the morph file for neuronXcore
    visualize_path = os.path.join(path.dirname(__file__), "output", group_name, f"{group_name}_neuronX")
    func.export_for_neuronXcore(circuit, visualize_path)
    print("done!")