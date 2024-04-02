import os.path as path
import numpy as np
from eworm.utils import *
from eworm.network import *
from eworm.network.process import *
import pickle
import time

if __name__ == "__main__":
    np.random.seed(42)
    main_dir = path.join(path.dirname(__file__), "config")
    config_dir = path.join(main_dir, "000_circuit_search_config.json")
    config = func.load_json(config_dir)
    input_cell_names = config["search_config"]["input_cell_names"]
    sim_config = config["sim_config"]
    abs_circuit_file = path.join(main_dir, "sample_#0_circuit.pkl")
    abs_circuit = pickle.load(open(abs_circuit_file, 'rb'))
    circuit = transform.abstract2detailed(abs_circuit, config["config"], load_hoc=True, rec_voltage=True)
    print(type(circuit))
    print(len(circuit.cells))
    print({cell.name: cell.index for cell in circuit.cells})
    data_config = config["data_config"]
    input_traces = eval("data_factory."+data_config['factory_name'])(num=len(input_cell_names),
                                                                     tstop=sim_config['tstop'],
                                                                     dt=sim_config['dt'], **data_config['args'])
    trace_len = int(sim_config['tstop'] / sim_config['dt'])
    simulation_start_time = time.time()
    with open(os.path.join(os.path.dirname(__file__), "..","components","cb2022_data","Ca_corr_mat_cell_name.txt")) as f:
        output_cell_names = f.read().split("\t")
    output_traces = circuit.simulation(sim_config, input_traces, input_cell_names, output_cell_names)
    simulation_end_time = time.time()
    print(f"simulation takes {simulation_end_time - simulation_start_time} s.")

    output_dir = path.join(path.dirname(__file__), "output", "test")
    func.visualize_io_data(input_traces, output_traces, os.path.join(output_dir, "trace_plot.png"),
                      time_axis=np.linspace(sim_config['dt'],
                                            sim_config['tstop'], trace_len),
                      input_label=input_cell_names,
                      output_label=output_cell_names,
                      x_label="Time (ms)", y_label="Voltage (mV)",
                      title="Circuit Search")
    print("done")
