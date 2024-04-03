import os
import h5py as h5
import numpy as np
from tqdm import tqdm
import os.path as path
import matplotlib.pyplot as plt
from single_nrn_tune.generator import syn_exp
from utils.utils import visualize_traces


def distance_compute(dynamic_, feature_, dynamic_exp_, feature_exp_):
    return np.sum((np.abs(dynamic_ - dynamic_exp_)) ** 2)


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
    work_directory = path.join(path.dirname(__file__), 'analysis')

    dynamic_exp, feature_exp = syn_exp(cell_name, sti_info['clamp_type'],
                                       int(sti_info['tstop'] / sti_info['dt']), sti_info['sqz'])
    visualize_traces(dynamic_exp[np.newaxis, :, :], save_dir=path.join(work_directory, cell_name + '_exp.jpg'))

    # Visualization the dataset
    data_dir = path.join(path.dirname(__file__), 'data', 'syn_data', 'test')

    # sort the dataset
    records = []
    for data_file in sorted(os.listdir(data_dir)):
        if 'h5' not in data_file:
            continue
        file_path = os.path.join(data_dir, data_file)
        print(file_path)
        with h5.File(file_path, 'r') as f:
            dynamic_data = f.get('dynamic_data')[:]
            feature_data = f.get('feature_data')[:]
            param_data = f.get('param_data')[:]
            log_setting = f.get('log_setting')[:]
            for idx in tqdm(range(len(dynamic_data))):
                distance_ = distance_compute(dynamic_data[idx], feature_data[idx],
                                             dynamic_exp, feature_exp)
                records.append((dynamic_data[idx], param_data[idx], distance_))

    records = sorted(records, key=lambda x: x[-1])
    sort_dynamic, sort_param, sort_distance = [x[0] for x in records], [x[1] for x in records], [x[2] for x in records],

    # plot the best #best_num dynamic traces
    best_num = 10
    plt.figure(figsize=(24, 3 * (best_num + 1)))
    for _idx, dynamic_ in enumerate(sort_dynamic[:best_num]):
        plt.subplot(best_num + 1, 1, _idx + 1)
        for single_trace in dynamic_:
            plt.plot(np.arange(len(single_trace)), single_trace)
        for single_trace in dynamic_exp:
            plt.plot(np.arange(len(single_trace)), single_trace, alpha=0.3)
        plt.title('best-' + str(_idx + 1))
    plt.subplot(best_num + 1, 1, best_num + 1)
    for single_trace in dynamic_exp:
        plt.plot(np.arange(len(single_trace)), single_trace)
    plt.title('exp')
    plt.savefig(path.join(work_directory, cell_name + '_dynamic_best{}.jpg'.format(best_num)))
    plt.close()

    # plot difference
    plt.figure(figsize=(24, 3 * (best_num + 1)))
    for _idx, dynamic_ in enumerate(sort_dynamic[:best_num]):
        plt.subplot(best_num + 1, 1, _idx + 1)
        for single_trace in dynamic_ - dynamic_exp:
            plt.plot(np.arange(len(single_trace)), single_trace)
        plt.title('best-' + str(_idx + 1))
    plt.subplot(best_num + 1, 1, best_num + 1)
    for single_trace in dynamic_exp:
        plt.plot(np.arange(len(single_trace)), single_trace)
        plt.title('exp')
    plt.savefig(path.join(work_directory, cell_name + '_diff_best{}.jpg'.format(best_num)))
    plt.close()

    # records the params
    with open(path.join(work_directory, cell_name + '_params_best{}.txt'.format(best_num)), 'w') as f:
        for _idx, params in enumerate(sort_param[:best_num]):
            f.writelines('best-' + str(_idx + 1) + ':\t')
            f.writelines(', '.join(str(param_) for param_ in params))
            f.writelines('\n')

    params_name = ['Ra', 'cm', 'gpas', 'epas', 'gbshl1',
                   'gbshk1', 'gbkvs1', 'gbegl2', 'gbegl36', 'gbkqt3',
                   'gbegl19', 'gbunc2', 'gbcca1', 'gbslo1_egl19', 'gbslo1_unc2',
                   'gbslo2_egl19', 'gbslo2_unc2', 'gbnca', 'gbirk']
    total_params = np.stack(sort_param, axis=0)
    total_distance = np.array(sort_distance)

    # plot the histogram of parameter distribution
    plt.figure(figsize=(30, 30))
    for param_idx, param_name in enumerate(params_name):
        plt.subplot(4, 5, param_idx + 1)
        if log_setting[param_idx]:
            plt.hist(np.log(total_params[:, param_idx]), bins=50)
        else:
            plt.hist(total_params[:, param_idx], bins=50)
        plt.title(param_name)
    plt.savefig(path.join(work_directory, cell_name + '_hist.jpg'))
    plt.close()

    # plot the distribution of the distance related to paramters
    plt.figure(figsize=(30, 30))
    for param_idx, param_name in enumerate(params_name):
        plt.subplot(4, 5, param_idx + 1)
        if log_setting[param_idx]:
            plt.scatter(np.log(total_params[:, param_idx]), total_distance, alpha=0.1)
        else:
            plt.scatter(total_params[:, param_idx], total_distance, alpha=0.1)
        plt.title(param_name)
    plt.savefig(path.join(work_directory, cell_name + '_scatter.jpg'))
    plt.close()
