import os
import os.path as path
import numpy as np
import torch
import numpy.random as nr
from scipy import signal
import matplotlib.pyplot as plt
import _pickle as pickle
import re
from eworm.utils import func
from scipy import interpolate


# weight normalization
"""
Three different weight form ( sample ---- input ---- meta )
1. sample: original form data, generated directly from weight config
2. input: model input form of data, normalize(sample) ---> input
3. meta: data form used for receiving gradient, sigmoid(meta) ---> input
"""


def sigmoid(x, pkg):
    return 1/(1+pkg.exp(-x))


def inv_sigmoid(x, pkg):
    return -pkg.log((1/x)-1)


def extract_bool(categories, connection_category, package_name):
    assert package_name == 'np' or package_name == 'torch'
    assert connection_category in ("syn", "gj")
    data_form = "np.array" if package_name == "np" else "torch.tensor"
    return eval(data_form)([category == connection_category for category in categories])


def get_pkg(package_name):
    assert package_name == 'np' or package_name == 'torch'
    if package_name == 'np':
        pkg = np
    else:
        pkg = torch
    return pkg


def input2meta(weights, categories, package='torch'):
    pkg = get_pkg(package)
    syn_sign, gj_sign = extract_bool(categories, "syn", package), extract_bool(categories, "gj", package)
    meta_weights = pkg.arctanh(weights * syn_sign) + inv_sigmoid(weights * gj_sign + 0.5 * syn_sign, pkg)
    return meta_weights


def sample2input(weights, categories, weight_config, package='torch'):
    pkg = get_pkg(package)
    syn_sign, gj_sign = extract_bool(categories, "syn", package), extract_bool(categories, "gj", package)
    low = pkg.log(syn_sign * weight_config['syn'][0] + gj_sign * weight_config['gj'][0])
    high = pkg.log(syn_sign * weight_config['syn'][1] + gj_sign * weight_config['gj'][1])
    input_weights = pkg.sign(weights) * (pkg.log(pkg.abs(weights)) - low) / (high - low)
    return input_weights


def meta2input(weights, categories, package='torch'):
    pkg = get_pkg(package)
    syn_sign, gj_sign = extract_bool(categories, "syn", package), extract_bool(categories, "gj", package)
    input_weights = pkg.tanh(weights * syn_sign) + sigmoid(weights * gj_sign, pkg) - 0.5 * syn_sign
    return input_weights


def input2sample(weights, categories, weight_config, package='torch'):
    pkg = get_pkg(package)
    syn_sign, gj_sign = extract_bool(categories, "syn", package), extract_bool(categories, "gj", package)
    low = pkg.log(syn_sign * weight_config['syn'][0] + gj_sign * weight_config['gj'][0])
    high = pkg.log(syn_sign * weight_config['syn'][1] + gj_sign * weight_config['gj'][1])
    sample_weights = pkg.sign(weights) * pkg.exp(pkg.abs(weights) * (high - low) + low)
    return sample_weights


def sample2meta(weights, categories, weight_config, package='torch'):
    input_weights = sample2input(weights, categories, weight_config, package)
    meta_weights = input2meta(input_weights, categories, package)
    return meta_weights


def meta2sample(weights, categories, weight_config, package='torch'):
    input_weights = meta2input(weights, categories, package)
    sample_weights = input2sample(input_weights, categories, weight_config, package)
    return sample_weights

# data generation


def multi_dim_interpolate(x, xp, fp):
    assert (x.ndim == 1 == xp.ndim) and (len(xp) == fp.shape[-1])
    output_trace = np.zeros((*fp.shape[:-1], len(x)))
    for trace_index in np.ndindex(fp.shape[:-1]):
        output_trace[trace_index] = np.interp(x, xp, fp[trace_index])
    return output_trace


def squeeze_trace(trace, squeeze_ratio=10):
    """
    :param trace: with shape (..., trace_len)
    :param squeeze_ratio: float or int
    :return: trace after squeeze with shape (..., trace_len/squeeze_ratio)
        if squeeze_ratio > 1: squeeze len will be ceil to int divided by 5
    """
    if squeeze_ratio >= 1:
        squeeze_ratio = int(squeeze_ratio)
        squeeze_len = int(trace.shape[-1] // squeeze_ratio)
        trace = trace[..., :squeeze_len*squeeze_ratio]
        trace = np.mean(trace.reshape(*trace.shape[:-1], squeeze_len, squeeze_ratio), axis=-1)
        while trace.shape[-1] < int(np.ceil(trace.shape[-1]/5))*5:
            trace = np.concatenate([trace, trace[..., -1:]], axis=-1)
    else:
        prolong_ratio = int(1 / squeeze_ratio)
        prolong_len = trace.shape[-1] * prolong_ratio
        x = np.arange(prolong_len)
        xp = np.arange(trace.shape[-1]) * prolong_ratio
        trace = multi_dim_interpolate(x, xp, trace)
    return trace

def load_location(path):
    """get target worm's crawling trace"""
    target_location = np.array(func.load_json(path)["Frames"])
    _loc = np.array([0,0,0], dtype=np.float)
    # col 4,5,6 - velocity of basic head point
    # col 7,8,9 - displacement of real head point to basic head point
    delta_time = 1/30
    location = np.zeros((len(target_location), 3), dtype=np.float64)
    for i, line in enumerate(target_location):
        _loc += line[3:6] * delta_time
        location[i] = _loc + line[6:9]
    return location


def delta_concentration_3d(loc, source_loc, motion_scale):
    """get deltaconcentration along the crawling trace"""
    concentration = np.zeros((len(loc),), dtype=np.float32)
    for i, loc in enumerate(loc):
        concentration[i] = np.linalg.norm(loc - source_loc) * -1
    delta_concentration = concentration[1:] - concentration[:-1]
    delta_concentration = np.insert(delta_concentration, 0, [delta_concentration[0]])
    delta_concentration = np.repeat(delta_concentration, motion_scale)  # repeat
    delta_concentration = (delta_concentration + 0.005) * 100 / 0.035 - 80  # normalize
    return delta_concentration


def c302_data_factory(num, tstop, dt, pause_prob, initial_pause, noise_amp, smooth_sigma, seed=None):
    """
    Generating simulation traces through concatenating of spike block from c302 data, separated by random length
    resting phase.
    :param num: #trace to generate
    :param tstop: total simulation time (ms)
    :param dt: simulation time step (ms)
    :param pause_prob: probability to briefly pause between each block of spike, instead of long stop
    :param initial_pause: the pause phase padding at beginning (ms)
    :param noise_amp: noise amplitude (mV)
    :param smooth_sigma: sigma for gaussian smooth
    :param seed: random seed
    """
    # preparation
    if seed is not None:
        nr.seed(seed)
    c302dt = 0.005
    transfer_ratio, len_sim, len_init = int(dt / c302dt), int(tstop / dt), int(initial_pause / dt)
    # spike_blocks shape: (37, 160000) --> (37, 160000/transfer_ratio)
    with open(path.join(path.dirname(__file__), '..', 'components', 'c302_data', 'c302_cycle.dat'), 'rb') as f:
        spike_blocks = pickle.load(f)[2:]
    spike_blocks = squeeze_trace(spike_blocks, transfer_ratio)
    len_block = spike_blocks.shape[1]
    gaussian_window = signal.windows.gaussian(int(1+7*smooth_sigma), std=smooth_sigma)
    gaussian_window = gaussian_window/sum(gaussian_window)
    voltage_traces = []
    for _ in range(num):
        token, len_counter = spike_blocks[np.random.randint(0, len(spike_blocks))], len_init + 0
        # batch = [np.zeros(len_init) + token[0]]
        batch = [np.zeros(len_init) - 40]
        while len_counter < len_sim:
            if np.random.rand() < pause_prob:
                pause_len = nr.randint(1, int(len_block/6))
            else:
                pause_len = nr.randint(int(1.5*len_block), int(5*len_block))
            batch.append(np.zeros(pause_len) + token[0])
            batch.append(token)
            len_counter += pause_len + len_block
        batch = np.concatenate(batch, axis=0)[:len_sim]
        batch += signal.convolve(nr.uniform(-1, 1, batch.shape)*noise_amp, gaussian_window, mode='same')
        voltage_traces.append(batch)
    return np.stack(voltage_traces, axis=0)


def interpolate_data_factory(num, tstop, dt, window_duration, gap_duration, feature_probs, volt_ranges,
                             noise_settings, uniform=False, reverse=0, seed=None):
    """
    Generating simulation traces through interpolating on uniform sampled mark point.
    resting phase.
    :param num: #trace to generate
    :param tstop: total simulation time (ms)
    :param window_duration: random window time (ms)
    :param gap_duration: least gap between windows (ms)
    :param feature_probs: probability dictionary of appearance of three features of random window:
        (stage_phase_prob, plateau_phase_prob, hyper_polarization_prob)
        stage_phase: before reaching the peak, keep a medium voltage for a while. Otherwise reaching peak in one rise.
        plateau_phase: after reaching the peak, keep high voltage for a while. Otherwise start decline right after peak.
        hyper-polarization: after decline to resting level, hyper-polarize to very low voltage.
    :param volt_ranges: dictionary of voltage range of four phase
        (resting_range, peak_range, hyper_range, stage_range)
    :param dt: simulation time step (ms)
    :param noise_settings: tuple of (noise1:(noise_amp, smooth_sigma), noise2:(noise_amp, smooth_sigma), ...)
    :param seed: random seed
    :param uniform: all neuron get the same input
    """
    # preparation
    if seed is not None:
        nr.seed(seed)
    len_sim, len_window, len_gap = int(tstop/dt), int(window_duration/dt), int(gap_duration/dt)
    window_num = int(np.ceil(tstop/window_duration))
    resting_range, peak_range, hyper_range, stage_range = volt_ranges["resting_range"], volt_ranges["peak_range"], \
                                                          volt_ranges["hyper_range"], volt_ranges["stage_range"]
    gaussian_windows = []
    for noise_setting in noise_settings:
        gaussian_window = signal.windows.gaussian(int(1 + 7 * noise_setting[1]), std=noise_setting[1])[np.newaxis, ]
        gaussian_windows.append(noise_setting[0] * gaussian_window / np.sum(gaussian_window))
    voltage_traces = []
    sample_num = num
    if uniform:
        num = 1
    for window_index in range(window_num):
        active_prob = (np.exp(nr.uniform(0, np.log(num+2)))-1) / num
        batch = []
        for trace_index in range(num):
            active_flag = nr.rand() < active_prob
            # sample endpoints index and voltage
            start_index = nr.randint(int(len_gap/2), int(len_window/2) - int(len_gap/2))
            end_index = nr.randint(int(len_window/2) + int(len_gap/2), len_window-int(len_gap/2))
            previous_v = voltage_traces[-1][trace_index][-1] if window_index > 0 else nr.uniform(*resting_range)
            start_v, end_v, post_v = nr.uniform(*resting_range, size=3)
            # sample inter key points index and voltage
            stage_flag = nr.rand() < feature_probs["stage_phase_prob"]
            plateau_flag = nr.rand() < feature_probs["plateau_phase_prob"]
            hyper_flag = nr.rand() < feature_probs["hyper_polarization_prob"]
            if active_flag:
                inter_indices = nr.choice(np.arange(start_index+int(len_gap/2), end_index-int(len_gap/2)),
                                          size=2+2*int(stage_flag)+int(plateau_flag)+int(hyper_flag), replace=False)
                index_list = (0, start_index, *np.sort(inter_indices), end_index, len_window)
            else:
                index_list = (0, start_index, end_index, len_window)
            v_list = [previous_v, start_v]
            if active_flag:
                if stage_flag:
                    v_list += [nr.uniform(*stage_range), nr.uniform(*stage_range)]
                v_list += [nr.uniform(*peak_range), nr.uniform(*peak_range)]
                if plateau_flag:
                    v_list += [nr.uniform(*peak_range)]
                if hyper_flag:
                    v_list +=[nr.uniform(*hyper_range)]
            v_list += [end_v, post_v]
            # make batch
            batch.append(np.concatenate([
                np.linspace(v_list[tok_ind], v_list[tok_ind+1], num=index_list[tok_ind+1]-index_list[tok_ind])
                for tok_ind in range(len(v_list)-1)]))
        voltage_traces.append(np.stack(batch, axis=0))
    voltage_traces = np.concatenate(voltage_traces, axis=-1)
    for gaussian_window in gaussian_windows:
        voltage_traces += signal.convolve(nr.uniform(-1, 1, voltage_traces.shape), gaussian_window, mode='same')
    if uniform:
        if not reverse:
            voltage_traces = np.tile(voltage_traces[nr.randint(0, num),:], (sample_num, 1))
        else:
            voltage_traces1 = np.tile(voltage_traces[nr.randint(0, num),:], (int(reverse), 1))
            voltage_traces2 = np.tile(-voltage_traces[nr.randint(0, num),:]-(-np.min(voltage_traces[nr.randint(0, num),:])-np.max(voltage_traces[nr.randint(0, num),:])), (sample_num-int(reverse), 1))
            voltage_traces = np.concatenate((voltage_traces1, voltage_traces2), axis=0)
    return voltage_traces[:sample_num, :len_sim]


def random_data_factory(num, tstop, dt, window_range, volt_range, noise_settings,
                        reverse_noise_setting=None, seed=None):
    """
    Generating simulation traces through interpolating on uniform sampled mark point.
    resting phase.
    :param num: #trace to generate
    :param tstop: total simulation time (ms)
    :param window_range: tuple of range of window length
    :param volt_range: tuple of range of random sample voltage distribution
    :param dt: simulation time step (ms)
    :param reverse_noise_setting: tuple of (probability of reverse noise, amplitude for reverse_noise, squeeze_ratio)
    :param noise_settings: tuple of (noise1:(noise_amp, smooth_sigma), noise2:(noise_amp, smooth_sigma), ...)
    :param seed: random seed
    """
    # preparation
    if seed is not None:
        nr.seed(seed)
    window_range = (int(window_range[0]/dt), int(window_range[1]/dt))
    (rvs_prob, rvs_amp, sqz_ratio) = reverse_noise_setting if reverse_noise_setting is not None else (0, 0, 0)
    len_sim, sqr_len = int(tstop / dt), np.sqrt(window_range[0]*window_range[1])
    gaussian_windows = []
    for noise_setting in noise_settings:
        gaussian_window = signal.windows.gaussian(int(1 + 7 * noise_setting[1]), std=noise_setting[1])[np.newaxis,]
        gaussian_windows.append(noise_setting[0] * gaussian_window / np.sum(gaussian_window))
    # voltage_traces, prob, delta = [], translation_config["prob"], translation_config["delta"]
    voltage_traces = []
    for _ in range(num):
        previous_volt = nr.uniform(*volt_range)
        len_cnt, batch = 0, []
        while len_cnt < len_sim:
            window_len = np.exp(nr.uniform(np.log(window_range[0]+sqr_len), np.log(window_range[1]+sqr_len)))-sqr_len
            next_volt = nr.uniform(*volt_range)
            batch.append(np.linspace(previous_volt, next_volt, int(window_len)))
            if reverse_noise_setting is not None and nr.rand() < rvs_prob:
                rvs_noise = squeeze_trace(nr.uniform(-1, 1, int(np.ceil(window_len * sqz_ratio))), sqz_ratio) * rvs_amp
                batch[-1] += rvs_noise[:int(window_len)]
            len_cnt += int(window_len)
            previous_volt = next_volt
        voltage_traces.append(np.concatenate(batch, axis=0)[:len_sim])
    voltage_traces = np.stack(voltage_traces, axis=0)
    for gaussian_window in gaussian_windows:
        voltage_traces += signal.convolve(nr.uniform(-1, 1, voltage_traces.shape), gaussian_window, mode='same')
    return voltage_traces


def cell2015_pc1_data_factory(num, tstop, dt, volt_range):
    """
    Generating simulation traces through interpolating on cell2015-pc1 data.
    :param num: #trace to generate
    :param tstop: total simulation time (ms)
    :param dt: simulation time step (ms)
    :param volt_range: tuple of range of random sample voltage distribution
    """
    pc1_data = pickle.load(open(path.join(path.dirname(__file__), '..', 'components', 'cell2015_data', 'pc1.dat'), 'rb'))
    pc1_time = pc1_data[:,0] * 1000 # (s) * 1000 = (ms)
    pc1_data = pc1_data[:,1]
    pc1_data = (pc1_data - min(pc1_data)) / (max(pc1_data) - min(pc1_data)) * (volt_range[1] - volt_range[0]) + volt_range[0]
    target_time_step = np.arange(start=0, stop=tstop, step=dt, dtype=np.float32)
    target_data = np.expand_dims(np.interp(target_time_step, pc1_time, pc1_data), axis=0)
    voltage_traces = np.tile(target_data, (num,1))
    return voltage_traces


def ghost_in_mesh_data_factory(num, tstop, dt, loc_path):
    """only used in ghost in mesh"""
    # dt = 5/3, tstop = 10000
    assert tstop <= 10000
    location = load_location(loc_path)
    delta_concentration = delta_concentration_3d(location, location[-1], motion_scale=1)
    xold = np.linspace(0, 10000, len(delta_concentration))
    yold = delta_concentration
    f = interpolate.interp1d(xold, yold, kind=5)
    trace_length = int(tstop / dt)
    xnew = np.linspace(0, tstop, trace_length)
    ynew = f(xnew)
    voltage_traces = np.tile(ynew, (num, 1))
    return voltage_traces


def ghost_in_mesh_data_factory_standard(num, tstop, dt, loc_path):
    """only used in ghost in mesh"""
    # dt = 5/3, tstop = 10000
    location = load_location(loc_path)
    delta_concentration = delta_concentration_3d(location, location[-1], motion_scale=1)   # 300 points, 30000 ms
    one_cycle = delta_concentration[141:162]
    xold = np.linspace(0, (len(one_cycle)-1)*100, len(one_cycle))   # ms
    yold = one_cycle  # 21 points, 20 * 100 ms
    xnew = np.linspace(0, (len(one_cycle)-1)*100-dt, round((len(one_cycle)-1)*100/dt))   # ms
    f = interpolate.interp1d(xold, yold, kind='previous')
    ynew = f(xnew)
    trace_length = int(tstop / dt)
    if np.mod(trace_length, len(xnew)) == 0:
        one_trace = np.tile(ynew, int(trace_length/len(ynew)))
    else:
        one_trace = np.tile(ynew, np.ceil(trace_length/len(ynew)))[:(-len(ynew)+np.mod(trace_length,len(ynew)))]
    voltage_traces = np.tile(one_trace, (num, 1))
    return voltage_traces


if __name__ == "__main__":
    nr.seed(42)
    default_c302_data_factory_config = {
        "factory_name": "c302_data_factory",
        "args": {
            'pause_prob': 0.2,
            'initial_pause': 500,
            'noise_amp': 80,
            'smooth_sigma': 60}}
    default_interpolate_data_factory_config = {
    "factory_name": "interpolate_data_factory",
    "args": {
      "window_duration": 400,
      "gap_duration": 100,
      "feature_probs": {
        "stage_phase_prob": 0.5,
        "plateau_phase_prob": 0.5,
        "hyper_polarization_prob": 0.5
      },
      "volt_ranges": {
        "resting_range": [
          -70,
          -70
        ],
        "peak_range": [
          20,
          40
        ],
        "hyper_range": [
          -70,
          -70
        ],
        "stage_range": [
          20,
          40
        ]
      },
      "noise_settings": [
        [
          200,
          10
        ],
        [
          0,
          0
        ]
      ]
    }
  }
    default_random_data_factory_config = {
        "factory_name": "random_data_factory",
        "args": {
            "window_range": (100, 1000),
            "volt_range": (-90, -50),
            "noise_settings": ((40, 5), (180, 60)),
            "reverse_noise_setting": (0.1, 30, 1/8)}}
    default_cell2015_pc1_data_factory_config = {
        "factory_name": "cell2015_pc1_data_factory",
        "args": {
            "volt_range": (-75, 40)}}
    default_ghost_in_mesh_data_factory_config = {
        "factory_name": "ghost_in_mesh_data_factory_standard",
        "args": {
            "loc_path": path.join(path.dirname(__file__), '..',
                                  "ghost_in_mesh_sim", "data", "state",
                                  "worm_states_300_220428-152601.json")
        }}
    data_config = default_interpolate_data_factory_config
    sim_config = {"dt": 5/3, "tstop": 10000}
    task_name = "sample"
    sample_num = 4
    sample_traces = eval(data_config['factory_name'])(
        num=sample_num, tstop=sim_config['tstop'], dt=sim_config['dt'], **data_config['args'])
    file_name = f"{data_config['factory_name']}_{task_name} "#\
                # + "__".join(["_".join(map(str, noise_setting))
                #              for noise_setting in data_config["args"]["noise_settings"]])

    working_directory = path.join(path.dirname(__file__), "data_factory_samples")
    os.makedirs(working_directory, exist_ok=True)
    plt.figure(figsize=(20, 3*sample_num))
    plt.subplot(sample_num + 1, 1, 1)
    for trace in sample_traces:
        plt.plot(np.linspace(0, sim_config['tstop'], len(trace))/1000, trace, alpha=0.4)
    plt.title(file_name)
    for trace_id, trace in enumerate(sample_traces):
        plt.subplot(sample_num+1, 1,  trace_id+2)
        plot_trace = squeeze_trace(trace, 8)
        plt.plot(np.linspace(0, sim_config['tstop'], len(plot_trace))/1000, plot_trace)
        plt.axhline(y=-50, color='r', alpha=0.4, linestyle='--')
        plt.axhline(y=0, color='r', alpha=0.4, linestyle='--')
        plt.xlabel("Time(s)")
        plt.ylabel("Voltage(mV)")
        plt.ylim(-100, 50)
    plt.tight_layout()
    plt.savefig(path.join(working_directory, f"{file_name}.jpg"))
    plt.close()
