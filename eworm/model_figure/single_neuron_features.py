from eworm.utils import *
import numpy as np
import matplotlib.pyplot as plt
import pickle
from neuron import h, load_mechanisms
import colorsys
import os
import os.path as path

h.load_file('stdrun.hoc')
working_directory = path.join(path.dirname('__file__'), "figs")

electrophysiology_simulation_config = {
    "AWAL": {
        "expr_ori_file_name": "AWAL_Current_Clamp_Trace.txt",
        "expr_file_name": "AWAL_electrophysiology_trace.pkl",
        "expr_x_range": [42, 17491],
        "expr_y_range": [],
        "siml_file_name": "AWAL_simulation_trace.pkl",
        "siml_x_range": [50100, 84998],
        "unit": 1e3,
        "xlim": (0, 7),
        "ylim": (),
        "mechanism_dir": os.path.join(path.dirname(__file__), '..', 'components', 'mechanism_AWA'),
        "sim_config": {"dt": 0.2, "tstop": 20000, "v_init": -73, "secondorder": 0},
        "stim_config": {"type": 'iclamp', "clamp_range": np.arange(-2, 17, 2) * 1e-3, "delay": 11000, "dur": 5000}
    },
    "AWCL": {
        "expr_ori_file_name": "AWCL_Voltage_Clamp_Trace.txt",
        "expr_file_name": "AWCL_electrophysiology_trace.pkl",
        "expr_x_range": [21, 332],
        "expr_y_range": [],
        "siml_file_name": "AWCL_simulation_trace.pkl",
        "siml_x_range": [4942, 5564],
        "unit": 1e9,
        "xlim": (0, 0.12),
        "ylim": (-0.50, 0.75),
        "mechanism_dir": os.path.join(path.dirname(__file__), '..', 'components', 'mechanism'),
        "sim_config": {"dt": 0.2, "tstop": 1600, "v_init": -60, "secondorder": 0},
        "stim_config": {"type": 'seclamp', "clamp_range": np.arange(-10, 111, 20), "delay": 1000, "dur": 100}
    },
    "AIYL": {
        "expr_ori_file_name": "AIYL_Current_Clamp_Trace.txt",
        "expr_file_name": "AIYL_electrophysiology_trace.pkl",
        "expr_x_range": [42, 17491],
        "expr_y_range": [],
        "siml_file_name": "AIYL_simulation_trace.pkl",
        "siml_x_range": [50100, 84998],
        "unit": 1e3,
        "xlim": (0, 7),
        "ylim": (),
        "mechanism_dir": os.path.join(path.dirname(__file__), '..', 'components', 'mechanism'),
        "sim_config": {"dt": 0.2, "tstop": 20000, "v_init": -45, "secondorder": 0},
        "stim_config": {"type": 'iclamp', "clamp_range": np.arange(-15, 36, 5) * 1e-3, "delay": 11000, "dur": 5000}
    },
    "RIML": {
        "expr_ori_file_name": "RIML_Current_Clamp_Trace.txt",
        "expr_file_name": "RIML_electrophysiology_trace.pkl",
        "expr_x_range": [42, 17491],
        "expr_y_range": [1, 10],
        "siml_file_name": "RIML_simulation_trace.pkl",
        "siml_x_range": [50100, 84998],
        "unit": 1e3,
        "xlim": (0, 7),
        "ylim": (),
        "mechanism_dir": os.path.join(path.dirname(__file__), '..', 'components', 'mechanism'),
        "sim_config": {"dt": 0.2, "tstop": 20000, "v_init": -39.3, "secondorder": 0},
        "stim_config": {"type": 'iclamp', "clamp_range": np.arange(-15, 26, 5) * 1e-3, "delay": 11000, "dur": 5000}
    },
    "VD05": {
        "expr_ori_file_name": "VD05_Voltage_Clamp_Trace.txt",
        "expr_file_name": "VD05_electrophysiology_trace.pkl",
        "expr_x_range": [127, 3527],
        "expr_y_range": [],
        "siml_file_name": "VD05_simulation_trace.pkl",
        "siml_x_range": [4500, 11300],
        "unit": 1e9,
        "xlim": (0, 1.4),
        "ylim": (-0.05, 0.3),
        "mechanism_dir": os.path.join(path.dirname(__file__), '..', 'components', 'mechanism'),
        "sim_config": {"dt": 0.2, "tstop": 2600, "v_init": -60, "secondorder": 0},
        "stim_config": {"type": 'seclamp', "clamp_range": np.arange(-10, 41, 10), "delay": 1000, "dur": 1210}
    },
    "AVAL": {
        "expr_ori_file_name": "AVAL_Current_Clamp_Trace.txt",
        "expr_file_name": "AVAL_electrophysiology_trace.pkl",
        "expr_x_range": [42, 17491],
        "expr_y_range": [],
        "siml_file_name": "AVAL_simulation_trace.pkl",
        "siml_x_range": [50060, 84958],
        "unit": 1e3,
        "xlim": (0, 6.5),
        "ylim": (),
        "mechanism_dir": os.path.join(path.dirname(__file__), '..', 'components', 'mechanism'),
        "sim_config": {"dt": 0.2, "tstop": 20000, "v_init": -30, "secondorder": 0},
        "stim_config": {"type": 'iclamp', "clamp_range": np.arange(-20, 21, 5) * 1e-3, "delay": 11000, "dur": 5015}
    }
}


def plateau_value(traces):
    return np.mean(traces[:, int(traces.shape[-1] * 4 / 7):int(traces.shape[-1] * 5.9 / 7)], axis=-1)


def gradient(traces, time_series):
    dt = (time_series[-1] - time_series[0])/(traces.shape[-1]-1)
    gradient_result = (traces[:, 1:] - traces[:, :-1]) / np.expand_dims(dt, axis=-1)
    return gradient_result


def get_param_dict(traces):
    normal_trace = np.mean(np.abs(traces-traces[0, 0]), axis=0)
    normal_trace = (normal_trace-np.mean(normal_trace))/np.mean(np.abs(normal_trace-np.mean(normal_trace))**2)
    normal_trace = np.abs(normal_trace - normal_trace[0])
    half_len = int(len(normal_trace)/2)
    init_timestep = int(np.argwhere(normal_trace[:half_len] < 0.01)[-1])
    end_timestep = int(np.argwhere(normal_trace[half_len:] > 0.01)[-1])+half_len
    return init_timestep, end_timestep-init_timestep


def peak_down(gradient_traces, init_timestep, duration_timestep):
    smooth_window = np.hamming(60)

    down_positions = []
    peak_values = []
    for trace in gradient_traces:
        intp_trace = np.interp(np.linspace(0, 1, len(trace)*10), np.linspace(0, 1, len(trace)), trace)
        intp_trace = np.convolve(intp_trace, smooth_window, 'same')
        range_ends = np.argwhere(trace[init_timestep:int(init_timestep+duration_timestep/30)] <= 0)
        if len(range_ends) == 0:
            range_end = int(init_timestep+duration_timestep/30)
        else:
            range_end = range_ends[0] + init_timestep
        peak_pos = init_timestep*10+np.argmax(np.abs(intp_trace[int(init_timestep)*10:int(range_end+1)*10]))
        peak_values.append(abs(intp_trace[peak_pos]))
        if peak_values[-1] < 50:
            down_positions.append(init_timestep)
        else:
            down_pos = int(np.argwhere(np.abs(intp_trace[peak_pos:]) <= 150)[0])
            down_positions.append(int((peak_pos+down_pos)/10))
    return np.array(down_positions)


def original_voltage_analysis(config, neuron_name):
    sim_data = pickle.load(
        open(os.path.join(path.dirname(__file__), "single_neuron", "simulation_data", config["siml_file_name"]), "rb"))
    elec_data = pickle.load(
        open(os.path.join(path.dirname(__file__), "single_neuron", "electrophysiology_data", config["expr_file_name"]), "rb"))
    simulation_traces, electrophysiology_traces = np.array(sim_data["voltage"]), np.array(elec_data["voltage"])
    simulation_time, electrophysiology_time = np.linspace(0, sim_data["time"][0][-1], simulation_traces.shape[-1]), np.linspace(0, elec_data["time"][-1], electrophysiology_traces.shape[-1])
    simulation_gradient, electrophysiology_gradient = gradient(simulation_traces, simulation_time), gradient(electrophysiology_traces, electrophysiology_time)
    print(simulation_traces.shape, electrophysiology_traces.shape)
    print(simulation_time.shape, electrophysiology_time.shape)
    init_time = config['stim_config']['delay']/1000 - config['siml_x_range'][0]*config['sim_config']['dt']/1000
    duration_time = config['stim_config']['dur']/1000
    init_sim_timestep, dur_sim_timestep = get_param_dict(simulation_traces)
    init_ele_timestep, dur_ele_timestep = get_param_dict(electrophysiology_traces)
    simulation_downs = peak_down(simulation_gradient, init_sim_timestep, dur_sim_timestep)
    electrophysiology_downs = peak_down(electrophysiology_gradient, init_ele_timestep, dur_ele_timestep)

    stim_axis = config["stim_config"]["clamp_range"]
    # fig, ax = plt.subplots(1, 6, figsize=(18, 2), dpi=200)

    # for trace_id in range(simulation_traces.shape[0]):
    #     ax[0].plot(simulation_time, simulation_traces[trace_id], color='k', label="model", lw=0.6, alpha=0.6)
    #     ax[0].plot(electrophysiology_time, electrophysiology_traces[trace_id], color='r', label="experiment", lw=0.6, alpha=0.6)

    # # iv
    # simulation_plateau_value = plateau_value(simulation_traces)
    # electrophysiology_plateau_value = plateau_value(electrophysiology_traces)
    # ax[1].plot(stim_axis, electrophysiology_plateau_value, color='r', marker='o', label='experiment')
    # ax[1].plot(stim_axis, simulation_plateau_value, color='k', marker='*', label='model')
    # if config["stim_config"]['type'] == 'iclamp':
    #     ax[1].set_xlabel("Current (pA)")
    #     ax[1].set_ylabel("Voltage (mV)")
    # elif config["stim_config"]['type'] == 'seclamp':
    #     ax[1].set_xlabel("Voltage (mV)")
    #     ax[1].set_ylabel("Current (pA)")

    # ax[1].spines['top'].set_visible(False)
    # ax[1].spines['right'].set_visible(False)

    # for trace_id in range(simulation_traces.shape[0]):
    #     ax[2].plot(simulation_time[:-1], simulation_gradient[trace_id], color='k', label="model", lw=0.5, alpha=0.6)
    #     ax[2].plot(electrophysiology_time[:-1], electrophysiology_gradient[trace_id], color='r', label="experiment", lw=0.5, alpha=0.6)

    # for trace_id in range(simulation_traces.shape[0]):
    #     ax[3].plot(simulation_time, simulation_traces[trace_id], color='k', label="model", lw=0.5, alpha=0.6)
    #     ax[3].plot(electrophysiology_time, electrophysiology_traces[trace_id], color='r', label="experiment", lw=0.5, alpha=0.6)
    #     ax[3].scatter(simulation_downs[trace_id]*simulation_time[-1]/simulation_traces.shape[-1],
    #                   simulation_traces[trace_id][simulation_downs[trace_id]], c='k', s=100, lw=0.2, marker="|")
    #     ax[3].scatter(electrophysiology_downs[trace_id]*electrophysiology_time[-1]/electrophysiology_traces.shape[-1],
    #                   electrophysiology_traces[trace_id][electrophysiology_downs[trace_id]], c='r', s=100, lw=0.2, marker="|")

    # ax[3].set_xlim(init_time-duration_time/40, init_time+duration_time/30)

    # offset = init_ele_timestep*electrophysiology_time[-1]/electrophysiology_traces.shape[-1]
    # simulation_downs_time = (simulation_downs)*simulation_time[-1]/simulation_traces.shape[-1] - offset
    # electrophysiology_downs_time = (electrophysiology_downs)*electrophysiology_time[-1]/electrophysiology_traces.shape[-1] - offset
    # ax[4].plot(stim_axis, electrophysiology_downs_time, color='r', marker='o', label='experiment')
    # ax[4].plot(stim_axis, simulation_downs_time, color='k', marker='*', label='model')
    # if config["stim_config"]['type'] == 'iclamp':
    #     ax[4].set_xlabel("Current (pA)")
    #     ax[4].set_ylabel("Time (s)")
    # elif config["stim_config"]['type'] == 'seclamp':
    #     ax[4].set_xlabel("Voltage (mV)")
    #     ax[4].set_ylabel("Time (s)")
    # # ax[4].set_ylim(0, duration_time/30)

    # ax[4].spines['top'].set_visible(False)
    # ax[4].spines['right'].set_visible(False)

    # ax[5].plot(stim_axis, [simulation_traces[trace_id, simulation_downs[trace_id]] for trace_id in range(simulation_traces.shape[0])],
    #            color='k', marker='*', label='model')
    # ax[5].plot(stim_axis, [electrophysiology_traces[trace_id, electrophysiology_downs[trace_id]] for trace_id in range(electrophysiology_traces.shape[0])],
    #            color='r', marker='o', label='experiment')
    # if config["stim_config"]['type'] == 'iclamp':
    #     ax[5].set_xlabel("Current (pA)")
    #     ax[5].set_ylabel("Voltage (mV)")
    # elif config["stim_config"]['type'] == 'seclamp':
    #     ax[5].set_xlabel("Voltage (mV)")
    #     ax[5].set_ylabel("Current (pA)")

    # ax[5].spines['top'].set_visible(False)
    # ax[5].spines['right'].set_visible(False)
    # plt.tight_layout()
    # plt.savefig(os.path.join(path.dirname(__file__), "single_neuron", "figure", f"{neuron_name}_original_voltage_analysis.png"))
    # plt.close()
    fig, ax = plt.subplots(figsize=(2,2), dpi=200)
    if config["stim_config"]['type'] == 'iclamp':
        ax.plot(stim_axis*1e3, [electrophysiology_traces[trace_id, electrophysiology_downs[trace_id]] for trace_id in range(electrophysiology_traces.shape[0])],
               color='r', marker='o', label='experiment')
        ax.plot(stim_axis*1e3, [simulation_traces[trace_id, simulation_downs[trace_id]] for trace_id in range(simulation_traces.shape[0])],
               color='k', marker='*', label='model')
        ax.set_xlabel("Current (pA)")
        ax.set_ylabel("Voltage (mV)")
    elif config["stim_config"]['type'] == 'seclamp':
        ax.plot(stim_axis, [electrophysiology_traces[trace_id, electrophysiology_downs[trace_id]]*1e3 for trace_id in range(electrophysiology_traces.shape[0])],
               color='r', marker='o', label='experiment')
        ax.plot(stim_axis, [simulation_traces[trace_id, simulation_downs[trace_id]]*1e3 for trace_id in range(simulation_traces.shape[0])],
               color='k', marker='*', label='model')
        ax.set_xlabel("Voltage (mV)")
        ax.set_ylabel("Current (pA)")
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    plt.tight_layout()
    plt.savefig(os.path.join(path.dirname(__file__), "single_neuron", "figure", f"{neuron_name}_last_col.png"))
    plt.close()


# neuron_name_list = ["AVAL", "AWAL", "AWCL"]
neuron_name_list = ["AIYL", "AVAL", "AWCL", "RIML", "VD05"]
# neuron_name_list = ["AVAL"]
for neuron_name in neuron_name_list:
    original_voltage_analysis(electrophysiology_simulation_config[neuron_name], neuron_name)
    # exit()
    # plot_fitting_result(electrophysiology_simulation_config[neuron_name], neuron_name)
    # plot_i_v_curve(electrophysiology_simulation_config[neuron_name], neuron_name)

