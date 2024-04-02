import matplotlib.pyplot as plt
import numpy as np
import numpy.random as nr
import os
import os.path as path
import seaborn


def threads_plot(data, y_tick_labels, x_dt, save_dir, overlap_num=2., xlabel="Time (s)", ylabel=""):
    """
    data: np.array with shape (n_traces, num_step)
    y_tick_labels: list of string, label for each trace
    x_dt: time for each step
    save_dir: path to save the figure
    overlap_num: density of traces overlapping with each other, 1 for no overlapping, 2 for half overlapping
    """
    max_char_len = np.max([len(char_tmp) for char_tmp in y_tick_labels])
    # for cell_id in range(len(y_tick_labels)):
    #     if cell_id % 2 == 0:
    #         y_tick_labels[cell_id] = y_tick_labels[cell_id].ljust(2 * max_char_len, " ").rjust(4 * max_char_len, " ")
    #     else:
    #         y_tick_labels[cell_id] = y_tick_labels[cell_id].ljust(2 * max_char_len, " ").ljust(4 * max_char_len, " ")

    max_shift = (np.max(data) - np.min(data)) / overlap_num
    xtick_len = int(np.ceil(int(data.shape[-1] / 5) / 500) * 500)
    xtick_len_minor = int(xtick_len / 10)
    colors = plt.rcParams['axes.prop_cycle'].by_key()['color']
    _, ax = plt.subplots(figsize=(10, 8))
    for trace_id, trace in enumerate(data):
        ax.plot(trace / max_shift + trace_id, lw=0.6, alpha=0.8)
    plt.xticks(np.arange(0, data.shape[-1], xtick_len),
               np.array(np.arange(0, data.shape[-1], xtick_len) * x_dt * 1e-3, dtype=int))
    plt.xticks(np.arange(0, data.shape[-1], xtick_len_minor),
               np.array(np.arange(0, data.shape[-1], xtick_len_minor) * x_dt * 1e-3, dtype=int), minor=True)
    plt.yticks(np.arange(data.shape[0])-1, y_tick_labels, fontsize=8)
    plt.tick_params(axis='y', length=2., direction='in')
    for ytick_id, ytick in enumerate(plt.gca().get_yticklabels()):
        ytick.set_color(colors[ytick_id % len(colors)])
    ax.set_ylabel(ylabel)
    ax.set_xlabel(xlabel)
    ax.grid(axis='x', which='minor', alpha=0.2)
    ax.grid(axis='x', which='major', alpha=0.5)
    ax.spines['right'].set_visible(False)
    ax.spines['left'].set_visible(False)
    ax.spines['bottom'].set_visible(False)
    ax.spines['top'].set_visible(False)
    ax.set_xlim(-xtick_len_minor, data.shape[-1] + xtick_len_minor)
    ax.set_ylim(-2, data.shape[0])
    plt.savefig(save_dir, dpi=500)
    plt.close()


if __name__ == "__main__":
    working_directory = path.join(path.dirname(__file__))
    os.makedirs(path.join(working_directory, "figs"), exist_ok=True)
    data = np.load(path.join("/home/zhaomengdi/Project/model_meta_worm/eworm/experiment/output/perturbation_data","control.npy"))
    print(data.shape)
    data = np.clip(data, -60, 20)
    output_cell_name = ['AVAL', 'AVAR', 'RIML', 'RIMR', 'AIBL', 'AIBR', 'AVEL', 'AVER', 'AS10', 'VA11', 'DA07', 'VA12',
                        'DA09', 'VD13', 'SABD', 'DA01', 'SABVL', 'URYDL', 'URYDR', 'URYVR', 'URYVL', 'SABVR', 'VA01',
                        'RIVL', 'RIVR', 'SMDVL', 'SMDVR', 'SMDDL', 'SMDDR', 'ALA', 'ASKL', 'ASKR', 'PDA', 'PHAL', 'PHAR',
                        'DVB', 'DVC', 'AVFL', 'AVFR', 'ALNL', 'ALNR', 'AVBL', 'AVBR', 'RID', 'RIBL', 'RIBR', 'DB07',
                        'VB11', 'PVNL', 'DVA', 'SIADL', 'SIAVL', 'SIAVR', 'SIADR', 'DB02', 'VB02', 'RMEV', 'RMED',
                        'RMEL', 'RIS', 'PLML', 'DB01', 'VB01', 'PVNR', 'RMER']
    pc1_sorted_index = [10,21,19,12,9,15,20,17,1,16,18,27,24,26,22,0,23,13,31,25,3,11,36,8,6,2,4,44,47,7,14,37,5,29,34,42,32,46,45,38,30,28,39,49,48,56,51,53,50,35,43,40,64,55,41,54,60,52,63,58,33,59,57,61,62]
    threads_plot(data[pc1_sorted_index], [output_cell_name[x] for x in pc1_sorted_index], 0.5, path.join(working_directory, "figs", "traces.jpg"), overlap_num=1.3)
