import os
import pymp
import time
import argparse
import h5py as h5
import numpy as np
from tqdm import tqdm
import os.path as path
from functools import reduce
from neuron import h, load_mechanisms
from utils.utils import squeeze_trace, params_sample, visualize_traces
from single_nrn_tune.neuron_model import WormNeuron
from single_nrn_tune.feature import extract_features


def nrn_init(cell_name):
    mechanism_dir = path.join(path.dirname(__file__), 'data', 'mechanism')
    model_dir = path.join(path.dirname(__file__), 'data', 'model', cell_name + ".hoc")
    h.load_file(model_dir)
    h.load_file('stdrun.hoc')
    load_mechanisms(mechanism_dir)


def syn_exp(cell_name, clamp_type, trunc_len, sqz):
    """
    Get experimental data from clamp recording
    :param cell_name:
    :param clamp_type: either current clamp ('Current') or voltage clamp ('Voltage')
    :param trunc_len:
    :param sqz: squeeze ratio
    :return: dynamic_data, feature_data

    """
    work_directory = path.join(path.dirname(__file__), 'data', 'clamp_data')
    assert clamp_type in ('Current', 'Voltage')

    # read experimental data
    with open(path.join(work_directory, cell_name + '_' + clamp_type + '_Clamp_Trace.txt'), 'r') as f:
        trace_data = []
        for line in f.readlines()[3:3 + trunc_len]:
            records = line.split('\t')
            trace_data.append(np.array([float(record) for record in records]))
        trace_data = np.stack(trace_data, axis=-1)

    # experimental data process
    if clamp_type == 'Current':
        # V --> mV
        dynamic_data = trace_data[1:] * 1000
    elif clamp_type == 'Voltage':
        # A --> nA
        dynamic_data = trace_data[1:] * 1e9
    else:
        raise NotImplementedError
    dynamic_data = squeeze_trace(dynamic_data, sqz)
    if cell_name == 'AWAL':
        dynamic_data[4, 252:750] = dynamic_data[4, 750:751]
        dynamic_data[4, 750:1400] = dynamic_data[4, 750:751]
    dynamic_data = np.stack((dynamic_data[0], dynamic_data[4], dynamic_data[5], dynamic_data[10]), axis=0)
    feature_data = extract_features(dynamic_data)

    return dynamic_data, feature_data


def construct(cell_name, task_id,
              data_num, file_storage, thread_num,
              param_range, log_setting, **sti_info):
    """
    Construct dataset from generating traces with random sampled neuron parameters
    :param cell_name:
    :param task_id: id for running task
    :param data_num: total #data to construct
    :param file_storage: maximum storage number in each h5 file
    :param thread_num: #parallel thread
    :param param_range: with shape (2, params_num)
    :param log_setting: boolean list/array with shape (params_num)
    :param sti_info: stimulus information for simulation:
            "dt", "tstop", "v_init", "clamp", "start_time", "end_time", "clamp_type", "sqz"
    :return: None

    """
    work_directory = path.join(path.dirname(__file__), 'data', 'syn_data', task_id)
    os.makedirs(work_directory, exist_ok=True)
    file_num = int(np.ceil(data_num / file_storage))
    params_num = param_range.shape[1]
    seq_num, seq_len = len(sti_info['clamp']), int(sti_info['tstop']/(sti_info['dt']*sti_info['sqz']))
    feature_num = len(extract_features(np.zeros((seq_num, seq_len))))
    dynamic_exp, feature_exp = syn_exp(cell_name, sti_info['clamp_type'],
                                       int(sti_info['tstop']/sti_info['dt']), sti_info['sqz'])

    for file_idx in range(file_num):
        num = min(file_storage, data_num)
        cnt = 0
        param_arr = np.zeros((num, params_num), dtype=np.float32)
        dynamic_arr = np.zeros((num, seq_num, seq_len), dtype=np.float32)
        feature_arr = np.zeros((num, feature_num), dtype=np.float32)

        while cnt < num:
            start_time = time.time()
            sample_num = file_storage
            params = params_sample(param_range, sample_num, log_setting)

            dynamic_sample = pymp.shared.array([sample_num, seq_num, seq_len], dtype='float32')
            feature_sample = pymp.shared.array([sample_num, feature_num], dtype='float32')
            with pymp.Parallel(thread_num) as p:
                for idx in tqdm(p.range(sample_num)) if p.thread_num == 0 else p.range(sample_num):
                    dynamic_sample[idx], feature_sample[idx], _ = simulation(cell_name, params[idx], False, **sti_info)

            print('It takes {}s to generate dynamic and feature'.format(time.time() - start_time))
            start_time = time.time()

            # data cleaning
            """
            #####Example#####
            Here we clean data through following criterion:
            check the consistency of several time stamp between constructed dynamic and experimental dynamic
            
            Alternative conditions could be some investigation on the feature data, like:
            conditions = [feature_sample[..., 0] < 1, feature_sample[..., 7] > -1, 
                          np.mean(feature_sample[..., 9:15]) < 256,]
            """
            conditions = [
                *[np.mean(np.abs(np.mean(dynamic_sample[:, :, t_:t_+50], axis=-1) -
                                 np.mean(dynamic_exp[np.newaxis, :, t_:t_+50], axis=-1)), axis=-1) < 15
                  for t_ in (100, 160, 252, 269, 400, 500, 600, 750, 1000, 1350, 1400, 1600)]]

            purified_idx = np.argwhere(reduce(np.logical_and, conditions))[:(num - cnt), 0]
            purified_num = len(purified_idx)
            param_arr[cnt:cnt + purified_num] = params[purified_idx]
            dynamic_arr[cnt:cnt + purified_num] = dynamic_sample[purified_idx]
            feature_arr[cnt:cnt + purified_num] = feature_sample[purified_idx]
            cnt += purified_num
            print(cnt)
            print('It takes {}s to append array'.format(time.time() - start_time))
        data_num -= num

        # visualize and save constructed file
        visualize_traces(dynamic_arr, path.join(work_directory, cell_name+'_'+str(file_idx)+'.jpg'))

        start_time = time.time()
        with h5.File(path.join(work_directory, cell_name+'_'+str(file_idx)+'.h5'), 'w') as file:
            file.create_dataset('dynamic_data', data=dynamic_arr)
            file.create_dataset('param_data', data=param_arr)
            file.create_dataset('feature_data', data=feature_arr)
            file.create_dataset('param_range', data=param_range)
            file.create_dataset('log_setting', data=log_setting)

        print('It takes {} to save the data no.{}'.format(time.time() - start_time, file_idx))


def simulation(cell_name, params, record_ions, **sti_info):
    """
    :param cell_name:
    :param params:
    :param record_ions: Boolean, record ion channels current for further analysis
    :param sti_info: stimulus information for simulation:
            "dt", "tstop", "v_init", "clamp", "start_time", "end_time", "clamp_type", "sqz"
    :return: dynamic_data, feature_data, currents_data

    """
    clamp_range = np.array(sti_info['clamp'])
    seq_num, seq_len = len(sti_info['clamp']), int(sti_info['tstop']/(sti_info['dt']*sti_info['sqz']))
    dynamic_data = np.zeros((seq_num, seq_len), dtype=np.float32)
    ion_data = []
    currents_data = {}
    neuron = WormNeuron(cell_name, params)

    for i, amp in enumerate(clamp_range):
        if sti_info['clamp_type'] == 'Voltage':
            neuron.add_constant_voltage_stimulus(sti_info['start_time'], sti_info['end_time'], sti_info['tstop'], amp)
        elif sti_info['clamp_type'] == 'Current':
            neuron.add_constant_current_stimulus(sti_info['start_time'], sti_info['end_time'], amp)
        else:
            raise NotImplementedError
        dynamic_data[i, :] = squeeze_trace(neuron.run(sti_info['dt'], sti_info['tstop'], sti_info['v_init'],
                                                      ions=record_ions), squeeze_ratio=sti_info['sqz'])
        if record_ions:
            ion_data.append(neuron.ion)

    if record_ions:
        for key_ in ion_data[0].keys():
            currents_data[key_] = np.stack((squeeze_trace(ion_data[i][key_], squeeze_ratio=sti_info['sqz'])
                                            for i in range(len(clamp_range))), axis=0)
    feature_data = extract_features(dynamic_data)

    return dynamic_data, feature_data, currents_data


parser = argparse.ArgumentParser()
parser.add_argument('--num', default=10000, type=int,
                    help='num of data to synthesize')
parser.add_argument('--threads', default=32, type=int,
                    help='thread number')
parser.add_argument('--cell', default='AIYL', type=str,
                    help='cell name')
parser.add_argument('--id', default='Happy', type=str,
                    help='name your process')

args = parser.parse_args()

if __name__ == '__main__':
    print(args)
    params_range = np.array([
        [10,    0.1,    1e-6,   -40.,
         1e-5,  1e-5,   1e-5,   1e-5,   1e-5,
         1e-5,  1e-5,   1e-5,   1e-5,   1e-5,
         1e-5,  1e-5,   1e-5,   1e-6,   2e-3,],
        [400,   10,     5e-4,   -100.,
         1.,    1.,     1.e-1,  1.e-2,  1.e-1,
         1.e-2, 1.e-1,  1.e-1,  1.e-1,  1.,
         1.,    1.,     1.,     5e-4,   8e-3],])

    log_setting = np.array(
        [False, True, True, False,
         True, True, True, True, True,
         True, True, True, True, True,
         True, True, True, True, False]
    )
    cell_name = args.cell
    data_num, file_storage = args.num, 10000
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

    nrn_init(cell_name)
    construct(
        cell_name, args.id,
        data_num, file_storage, args.threads,
        params_range, log_setting,
        **sti_info)


