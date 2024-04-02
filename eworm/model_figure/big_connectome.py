import matplotlib.pyplot as plt
import networkx as nx
from eworm.network import *
from eworm.utils import *
import _pickle as pkl
import os
import os.path as path
import sys
import matplotlib.patches as ptc
import numpy as np
import numpy.random as nr
import scipy


def spectral_layout(A):
    c = (A + np.transpose(A)) / 2.0

    # degree matrix
    d = np.diag(np.sum(c, axis=0))
    df = scipy.linalg.fractional_matrix_power(d, -0.5)

    # Laplacian matrix
    l = d - c

    # compute the vertical coordinates
    b = np.sum(c * np.sign(A - np.transpose(A)), 1)
    z = np.matmul(np.linalg.pinv(l), b)

    # degree-normalized graph Laplacian
    q = np.matmul(np.matmul(df, l), df)

    # coordinates in plane are eigenvectors of degree-normalized graph Laplacian
    _, vx = np.linalg.eig(q)
    x = np.matmul(df, vx[:, 1])
    y = np.matmul(df, vx[:, 2])
    print(f"x range: {np.max(x)}, {np.min(x)}, {x.shape}")
    print(f"y range: {np.max(y)}, {np.min(y)}, {y.shape}")
    print(f"z range: {np.max(z)}, {np.min(z)}, {z.shape}")
    return np.stack([x, z], axis=-1)


def regularize_list(a, shift):
    sorted_a = sorted(a)
    a_dict = {sorted_a[i]: i*shift for i in range(len(a))}
    result_a = []
    for i in range(len(a)):
        result_a.append(a_dict[a[i]])
    return np.array(result_a)


def regularize_position(pos, shift):
    return np.stack([regularize_list(pos[:, 0], shift), regularize_list(pos[:, 1], shift)], axis=-1)


def zoom_position(pos, min_distance):
    dis = []
    for inter1 in range(len(pos)):
        for inter2 in range(len(pos)):
            if inter2 > inter1:
                dis.append(np.linalg.norm(pos[inter1]-pos[inter2]))
    return pos * min_distance / np.min(dis)


def plot_connectome(circuit, pos, save_dir):
    colors = plt.rcParams['axes.prop_cycle'].by_key()['color']
    for cell_id, cell in enumerate(circuit.cells):
        cell.pos = tuple(pos[cell_id])

    fig, ax = plt.subplots(figsize=(6, 6), dpi=600)
    for cnt_id, connection in enumerate(circuit.connections):
        if connection.pre_cell is None or connection.post_cell is None:
            continue
        if connection.category == 'gj':
            arrow_sty, arrow_lw, arrow_fc, arrow_ls = ptc.ArrowStyle("-"), 0.6 * connection.weight, (0, 0, 1, 0.5), ":"
            rad = 0
        elif connection.weight > 0:
            arrow_sty, arrow_lw, arrow_fc, arrow_ls = ptc.ArrowStyle("wedge", tail_width=0.4 * connection.weight), 0., \
                                                      (1, 0, 0, 0.5), "-"
            rad = 0.2
        else:
            arrow_sty, arrow_lw, arrow_fc, arrow_ls = ptc.ArrowStyle("wedge", tail_width=-0.4 * connection.weight), 0., \
                                                      (0, 0, 1, 0.5), "-"
            rad = -0.2
        ax.annotate("", xy=connection.pre_cell.pos, xytext=connection.post_cell.pos, zorder=1,
                    arrowprops=dict(arrowstyle=arrow_sty, shrinkA=5, shrinkB=10, lw=arrow_lw, fc=arrow_fc, ls=arrow_ls,
                                    alpha=0.6, connectionstyle=f"arc3,rad={rad}"))
    for cell_id, cell in enumerate(circuit.cells):
        ax.text(*cell.pos, cell.name, fontsize=3, zorder=4, horizontalalignment='center', verticalalignment='center',
                weight="bold")
    for cat_id, neuron_cat in enumerate(neuron_cat_set.keys()):
        neuron_cat_set[neuron_cat] = colors[cat_id]
    cell_colors = []
    for cell in circuit.cells:
        cell_colors.append(neuron_cat_set[neuron_cat_dict[cell.name]])
    print(neuron_cat_set)
    ax.scatter(pos[:, 0], pos[:, 1], s=50, c=cell_colors, marker="o", alpha=1, zorder=3)
    ax.scatter(pos[:, 0], pos[:, 1], s=70, c="w", marker="o", alpha=1, zorder=2)
    plt.axis('off')
    plt.savefig(save_dir)
    plt.close()


if __name__ == "__main__":
    working_directory = path.join(path.dirname(__file__), "figs")
    sys.modules['abstract_circuit'] = abstract_circuit
    neuron_cat_sheet, nrow, ncol = func.read_excel(path.join(path.dirname(__file__), "..", "components", "param", "connection", "SI5-302.xlsx"),
                                 sheet_name="Neurons")
    neuron_cat_dict = {}
    neuron_cat_set = {}
    for row_id in range(nrow):
        neuron_cat_dict[neuron_cat_sheet.cell_value(row_id, 0)] = neuron_cat_sheet.cell_value(row_id, 1)
        neuron_cat_set[neuron_cat_sheet.cell_value(row_id, 1)] = 1
    circuit = pkl.load(open(path.join(path.dirname(__file__), "data", "optimal_abs_circuit.pkl"), "rb"))
    print(len(circuit.cells), len(circuit.connections))
    circuit, functional_matrix, structural_matrix = transform.purify_artificial_circuit(circuit, normalize_weight=True)
    print(len(circuit.cells), len(circuit.connections))
    func_position = spectral_layout(functional_matrix)
    stru_position = spectral_layout(structural_matrix)

    position_regular = regularize_position(func_position, 0.3)
    position_zoom = zoom_position(func_position, 0.3)
    # plot_connectome(circuit, position, path.join(working_directory, "graph_big_ori.jpg"))
    plot_connectome(circuit, position_zoom, path.join(working_directory, "graph_big_ori_functional.jpg"))
    plot_connectome(circuit, position_regular, path.join(working_directory, "graph_big_rearrange_functional.jpg"))


    position_regular = regularize_position(stru_position, 0.3)
    position_zoom = zoom_position(stru_position, 0.3)
    # plot_connectome(circuit, position, path.join(working_directory, "graph_big_ori.jpg"))
    plot_connectome(circuit, position_zoom, path.join(working_directory, "graph_big_ori_structure.jpg"))
    plot_connectome(circuit, position_regular, path.join(working_directory, "graph_big_rearrange_structure.jpg"))