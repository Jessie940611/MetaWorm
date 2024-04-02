import matplotlib.pyplot as plt
from eworm.network import *
from eworm.utils import *
import _pickle as pkl
import os
import os.path as path
import sys
import numpy as np
import numpy.random as nr


def corr(trace_a, trace_b):
    return np.corrcoef(np.vstack([trace_a, trace_b]))[0, 1]


def regular(trace_a):
    sorted_a = np.sort(trace_a)
    target_a = sorted_a + 0
    for i in range(len(sorted_a)-1):
        if (target_a[i+1] - target_a[i]) >= 5:
            target_a[i+1:] -= target_a[i+1] - target_a[i] - 5
    a_dict = {sorted_a[i]: target_a[i] for i in range(len(sorted_a))}
    for i in range((len(sorted_a))):
        trace_a[i] = a_dict[trace_a[i]]


if __name__ == "__main__":
    sys.setrecursionlimit(10000)
    working_directory = path.join(path.dirname(__file__), "connection_selection")
    os.makedirs(working_directory, exist_ok=True)
    config_dir = path.join(path.dirname(__file__), "..", "network_train", "configs", "config_total.json")
    config = func.load_json(config_dir)
    del_circuit = transform.config2detailed(config, [], [], cluster_connection=True, rec_voltage=True)
    vis.export_for_neuronXcore(del_circuit, working_directory, "connection_demo")
    circuit = transform.detailed2abstract(del_circuit)
    pkl.dump(circuit, open(path.join(path.dirname(__file__), "data", "total.pkl"), "wb"))

    # circuit = pkl.load(open(path.join(path.dirname(__file__), "data", "total.pkl"), "rb"))

    # sys.modules['abstract_circuit'] = abstract_circuit
    # circuit = pkl.load(open(path.join(path.dirname(__file__), "data", "optimal_abs_circuit.pkl"), "rb"))
    print(len(circuit.cells), len(circuit.connections))

    connection_evaluator = {}
    for cell1_id in range(len(circuit.cells)):
        for cell2_id in range(len(circuit.cells)):
            if cell1_id == cell2_id:
                continue
            cell1, cell2 = circuit.cells[cell1_id], circuit.cells[cell2_id]
            tmp_evaluator = []
            syn_cnt, gj_cnt = 0, 0
            for segment1 in cell1.segments:
                for post_connection in segment1.post_connections:
                    if (post_connection.post_cell is not None) and (post_connection.post_cell.name == cell2.name):
                        tmp_evaluator.append((post_connection.pre_segment.index, post_connection.post_segment.index, post_connection.category))
                        if post_connection.category == "gj":
                            gj_cnt += 1
                        else:
                            syn_cnt += 1
            if len(tmp_evaluator) >= 6 and gj_cnt >= 3 and syn_cnt >= 3:
                connection_evaluator[
                    f"pre {cell1.name}:{len(cell1.segments)} post {cell2.name}:{len(cell2.segments)} || {gj_cnt}gj {syn_cnt}syn"
                ] = tmp_evaluator
    print(len(connection_evaluator.keys()))
    # exit()
    n_row = int(np.sqrt(len(connection_evaluator.keys())))
    n_col = int(np.ceil(len(connection_evaluator.keys())/n_row))
    fig, ax = plt.subplots(n_row, n_col, figsize=(n_col*4, n_row), dpi=200)
    for evaluator_id, connection_pair in enumerate(connection_evaluator.keys()):
        row_id, col_id = int(np.floor(evaluator_id / n_col)), int(evaluator_id % n_col)
        for pre_index, post_index, category in connection_evaluator[connection_pair]:
            color = 'r' if category == 'syn' else 'b'
            ax[row_id, col_id].plot([pre_index, post_index], [2, 1], c=color, alpha=0.8, lw=0.3)
        ax[row_id, col_id].set_yticks([1, 2])
        ax[row_id, col_id].set_yticklabels(["post", "pre"], fontsize=6)
        ax[row_id, col_id].set_xlabel("segments_index", fontsize=6)
        ax[row_id, col_id].set_title(connection_pair, fontsize=6)
    plt.tight_layout()
    plt.savefig(path.join(working_directory, "connection_example.jpg"))
    plt.close()
    fig, ax = plt.subplots(n_row, n_col, figsize=(n_col*4, n_row), dpi=200)
    for evaluator_id, connection_pair in enumerate(connection_evaluator.keys()):
        row_id, col_id = int(np.floor(evaluator_id / n_col)), int(evaluator_id % n_col)

        pre_indices = np.array([triple[0] for triple in connection_evaluator[connection_pair]])
        post_indices = np.array([triple[1] for triple in connection_evaluator[connection_pair]])
        categories = np.array([triple[2] for triple in connection_evaluator[connection_pair]])
        regular(pre_indices)
        regular(post_indices)
        pre_indices = pre_indices-np.min(pre_indices)
        post_indices = post_indices-np.min(post_indices)
        for pre_index, post_index, category in zip(pre_indices, post_indices, categories):
            color = 'r' if category == 'syn' else 'b'
            ax[row_id, col_id].plot([pre_index, post_index], [2, 1], c=color, alpha=0.8, lw=0.3)
        ax[row_id, col_id].set_yticks([1, 2])
        ax[row_id, col_id].set_yticklabels(["post", "pre"], fontsize=6)
        ax[row_id, col_id].set_xlabel("segments_index", fontsize=6)
        ax[row_id, col_id].set_title(connection_pair, fontsize=6)
    plt.tight_layout()
    plt.savefig(path.join(working_directory, "connection_example_rearrange.jpg"))
    plt.close()
    exit()