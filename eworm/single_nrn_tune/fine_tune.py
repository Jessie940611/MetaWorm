import numpy as np
import os.path as path
import matplotlib.pyplot as plt
from utils.utils import visualize_traces
from single_nrn_tune.param_generator import nrn_init, syn_exp, simulation


if __name__ == '__main__':
    cell_name = 'AWAL'
    sti_info = {
        "dt": 0.4,
        "tstop": 7000,
        "v_init": -80,
        "clamp": (-15, 5, 10, 35),
        "start_time": 1000,
        "end_time": 6000,
        "clamp_type": 'Current',
        "sqz": 10,
    }
    work_directory = path.join(path.dirname(__file__), 'fine_tune')

    dynamic_exp, feature_exp = syn_exp(cell_name, sti_info['clamp_type'],
                                       int(sti_info['tstop'] / sti_info['dt']), sti_info['sqz'])
    visualize_traces(dynamic_exp[np.newaxis, :, :], save_dir=path.join(work_directory, cell_name + '_exp.jpg'))

    nrn_init(cell_name)

    # simulation
    """
            params: 0-4:    Ra,             cm,             gpas,       epas,           gbshl1,
                    5-9:    gbshk1,         gbkvs1,         gbegl2,     gbegl36,        gbkqt3,
                    10-14:  gbegl19,        gbunc2,         gbcca1,     gbslo1_egl19,   gbslo1_unc2,
                    15-18:  gbslo2_egl19,   gbslo2_unc2,    gbnca,      gbirk
    """
    task_name = 'test'
    tmp_params = np.array([
        131.85, 4.78, 2.97e-06, -60,
        1.e-05, 0.38, 0.066, 1.e-05, 1.e-05,
        0.0035, 1.e-05, 1.e-05, 0.027, 1.e-05,
        0.41, 1.e-05, 1.e-05, 1.62e-05, 0.006
       ])

    dynamic_data, feature_data, currents_data = simulation(cell_name, tmp_params, record_ions=True, **sti_info)

    # plot the main figure for fine tuning
    param_index = (6, 7, 8, 9, 10, 16, 17, 18, 19, 20, 26, 27, 28, 29, 30)
    param_name = ('ik_shl1', 'ik_shk1', 'ik_kvs1', 'ik_egl2', 'ik_egl36', 'ik_kqt3', 'ica_egl19', 'ica_unc2',
                  'ica_cca1', 'ik_slo1_egl19', 'ik_slo1_unc2', 'ik_slo2_egl19', 'ik_slo2_unc2', 'ina_nca', 'ik_irk')

    plt.figure(figsize=(40, 14))
    ax1 = plt.subplot(1, 2, 1)
    for single_trace in dynamic_data:
        ax1.plot(np.arange(len(single_trace)), single_trace)
    for single_trace in dynamic_exp:
        ax1.plot(np.arange(len(single_trace)), single_trace, alpha=0.4)

    for idx_ in range(15):
        axes = plt.subplot(3, 10, param_index[idx_])
        for param_ in param_name:
            for single_trace in currents_data[param_]:
                axes.plot(np.arange(len(single_trace)), single_trace, alpha=0.1)
        for single_trace in currents_data[param_name[idx_]]:
            axes.plot(np.arange(len(single_trace)), single_trace)
        axes.set_title(param_name[idx_]+': '+str(tmp_params[idx_+4]))
        axes.set_ylim(-0.3, 0.3)

    plt.savefig(path.join(work_directory, task_name+'.jpg'))
    plt.close()

