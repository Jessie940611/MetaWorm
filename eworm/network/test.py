import os.path as path
import numpy as np
from eworm.utils import *
from eworm.network import *
from eworm.network.process import *
import pickle
import time

if __name__ == "__main__":
    np.random.seed(42)
    config_dir = path.join(path.dirname(__file__), "config.json")
    config = func.load_json(config_dir)
    input_cells = ["AVEL", "AVER", "AVBL", "AVBR", "AVAL", "AVAR", "PVCL", "PVCR"]
    sim_config = {"dt": 0.5, "tstop": 100, "v_init": -65, "secondorder": 0}

    abs_circuit_path = path.join(path.dirname(__file__), "output", "test")
    abs_circuit_file = path.join(abs_circuit_path, "test_abs_circuit.pkl")
    if path.exists(abs_circuit_file):
        abs_circuit = pickle.load(open(abs_circuit_file, 'rb'))
        circuit = transform.abstract2detailed(abs_circuit, config, load_hoc=True, rec_voltage=True)
    else:
        os.makedirs(abs_circuit_path, exist_ok=True)
        circuit = transform.config2detailed(config, input_cells, rec_voltage=True, cluster_connection=True, seed=0)
        abscircuit = transform.detailed2abstract(circuit)
        sys.setrecursionlimit(100000)
        pickle.dump(abscircuit, open(abs_circuit_file, 'wb'))
    print(type(circuit))
    print(len(circuit.cells))
    print({cell.name: cell.index for cell in circuit.cells})
    input_cell_names = ["AVEL", "AVER", "AVBL", "AVBR", "AVAL", "AVAR", "PVCL", "PVCR"]
    output_cell_names = ["AVEL", "AVER", "AVBL", "AVBR", "AVAL", "AVAR", "PVCL", "PVCR"]
    data_config = {
        "factory_name": "interpolate_data_factory",
        "args": {
            "window_duration": 10000,
            "gap_duration": 50,
            "feature_probs": {"stage_phase_prob": 0.5, "plateau_phase_prob": 0.5, "hyper_polarization_prob": 0.5},
            "volt_ranges": {"resting_range": (-80, -40), "peak_range": (-20, 35), "hyper_range": (-90, -75),
                            "stage_range": (-50, -20)},
            "noise_settings": ((80, 60), (20, 20))}}
    input_traces = eval("data_factory."+data_config['factory_name'])(num=len(input_cell_names),
                                                                     tstop=sim_config['tstop'],
                                                                     dt=sim_config['dt'], **data_config['args'])
    trace_len = int(sim_config['tstop'] / sim_config['dt'])
    simulation_start_time = time.time()
    output_traces = circuit.simulation(sim_config, input_traces, input_cell_names, output_cell_names)
    simulation_end_time = time.time()
    print(f"simulation takes {simulation_end_time - simulation_start_time} s.")
    save_dir = path.join(abs_circuit_path, "test1.png")
    # func.visualize_io_data(input_traces, output_traces, save_dir,
    #                   time_axis=np.linspace(sim_config['dt'],
    #                                         sim_config['tstop'], trace_len),
    #                   input_label=input_cell_names,
    #                   output_label=output_cell_names,
    #                   x_label="Time (ms)", y_label="Voltage (mV)",
    #                   title="Circuit Search")
    
    # p_circuit = transform.detailed2point(circuit)
    # circuit.release_hoc()
    # output_traces = p_circuit.simulation(sim_config, input_traces, input_cell_names, output_cell_names)
    # save_dir = path.join(abs_circuit_path, "test2.png")
    # func.visualize_io_data(input_traces, output_traces, save_dir,
    #                   time_axis=np.linspace(sim_config['dt'],
    #                                         sim_config['tstop'], trace_len),
    #                   input_label=input_cell_names,
    #                   output_label=output_cell_names,
    #                   x_label="Time (ms)", y_label="Voltage (mV)",
    #                   title="Circuit Search")

    # visualize_path = os.path.join(path.dirname(__file__), "output", "test", "neuronX")
    # func.export_for_neuronXcore(circuit, visualize_path)
    # volt_figure_path = os.path.join(path.dirname(__file__), "output", "test", "volt_figure")
    # func.export_volt_figure(circuit, sim_config, volt_figure_path)
    # watch_io_path = os.path.join(path.dirname(__file__), "output", "test", "watch_io.png")
    # func.watch_neuron_io(circuit, input_traces, input_cell_names, sim_config, watch_io_path, watch_neuron_name=output_cell_names)
    print("done!")

    # process.circuit_search(sim_config, save_dir=path.join(path.dirname(__file__), "output", "find_weights"))
