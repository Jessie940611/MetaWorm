import os.path as path
import numpy as np
from eworm.utils import *
from eworm.network import *
from eworm.network.process import *
import pickle
import time

def add_iclamp(circuit, input_cell_names):
    circuit.vclamp = []
    for cell_name in input_cell_names:
        for sid in range(len(circuit.cell(cell_name=cell_name).segments)):
            # syn = h.IClamp(circuit.cell(cell_name=cell_name).segments[sid].hoc_obj)
            # syn.amp = 0.
            # syn.delay = 0.
            # syn.dur = 1e9
            syn = h.SEClamp(circuit.cell(cell_name=cell_name).segments[sid].hoc_obj)
            syn.dur1 = 0
            syn.dur2 = 1000
            syn.dur3 = 0
            syn.amp1 = -65
            syn.amp2 = 0
            syn.amp3 = -65
            syn.rs = 20
            circuit.vclamp.append(syn)

if __name__ == "__main__":
    np.random.seed(42)
    group_name = "PQR_AVAR"  # sym+gj
    input_cells = ["PQR"]
    output_cell_names = ["PQR", "AVAR"]

    # group_name = "RIVR_SMDVR"  # gj
    # input_cells = ["RIVR"]
    # output_cell_names = ["RIVR", "SMDVR"]

    # group_name = "RIBR_AIBL"  #syn
    # input_cells = ["RIBR"]
    # output_cell_names = ["RIBR", "AIBL"]

    config_dir = path.join(path.dirname(__file__), "connections", f"{group_name}_config.json")
    config = func.load_json(config_dir)
    sim_config = {"dt": 0.5, "tstop": 1000, "v_init": -65, "secondorder": 0}
    abs_circuit_path = path.join(path.dirname(__file__), "connections", group_name)
    os.makedirs(abs_circuit_path, exist_ok=True)
    circuit = transform.config2detailed(config, input_cells, rec_voltage=True, cluster_connection=True, seed=0)
    abscircuit = transform.detailed2abstract(circuit)
    print({cell.name: cell.index for cell in circuit.cells})
    print(len(circuit.cell(cell_name=output_cell_names[0]).segments), len(circuit.cell(cell_name=output_cell_names[1]).segments))
    for conn in circuit.connections:
        if conn.pre_cell:
            print(conn.pre_cell.name, conn.pre_segment.index, conn.pre_segment.point2.location, conn.category, conn.post_cell.name, conn.post_segment.index, conn.post_segment.point2.location, np.linalg.norm(conn.pre_segment.point2.location-conn.post_segment.point2.location))
 
    trace_len = int(sim_config['tstop'] / sim_config['dt'])
    add_iclamp(circuit, output_cell_names)
    # simulation
    h.dt, h.tstop = sim_config['dt'], sim_config['tstop']
    h.steps_per_ms = int(1 / sim_config['dt'])
    h.secondorder = sim_config['secondorder']
    h.finitialize(sim_config['v_init'])
    for time_step in trange(trace_len):
        sid = 0
        for cell_name, amp in zip(output_cell_names, [-80, 80]):
            for _ in range(len(circuit.cell(cell_name=cell_name).segments)):
                circuit.vclamp[sid].amp2 = amp
                sid += 1
        h.fadvance()
        # print(circuit.cell(cell_name=output_cell_names[0]).segments[10].hoc_obj.v, circuit.cell(cell_name=output_cell_names[1]).segments[10].hoc_obj.v)

    visualize_path = os.path.join(abs_circuit_path)
    func.export_for_neuronXcore(circuit, visualize_path, group_name=group_name)
    print("done!")