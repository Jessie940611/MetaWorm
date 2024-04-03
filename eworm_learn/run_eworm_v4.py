import os
import pickle
import copy
import numpy as np
import cupy as cp
from neuron import h
from network_learn_v4 import Network
from utils import *
from scipy import signal, interpolate
import matplotlib.pyplot as plt
import time
import json


OUTPUT_PATH = './trial10'
PREFIX = 'eworm'
SUFFIX = 'v4'
TARGET_MODE = 'corr'      # one of 'corr' or 'traces'
RUN_MODE = 'test'        # 'test' or 'train'

config_file = os.path.join(OUTPUT_PATH, "000_circuit_search_config.json")
connection_file = os.path.join(OUTPUT_PATH, "sample_#0_circuit_old.pkl")
config = func.load_json(config_file)
sim_config = config["sim_config"]

dt = sim_config["dt"]           # time step
v_r = sim_config["v_init"]      # resting potential
ngpu = 8                        # number of GPUs used
K_max_t = 80                    # K time window
K_filename = os.path.join(OUTPUT_PATH, 'K_eworm_v4.npz')   # transfer impedance file name
K_nblock = 136                  # number of diagnal sub-blocks of K
tstop = sim_config["tstop"]     # simulation time
epochs = 100                    # max training epochs
alpha_w0 = 1e-5                 # initial learning rate for w
alpha_x0 = 3e-2                 # initial learning rate for x
alpha_d = 1/125	                # learning rate decay constant
w_gap_max = None
w_gap_min = 1e-9
w_syn_max = None
w_syn_min = -2
random_seed = 42
PERCISE = True
ADAM_W = True
ADAM_X = True
PRINT_TIMESTEP = True


def cal_corrcoef_dLtdv(output_vs, target_corrcoef, lr_start, lr_end):
    output_corrcoef = np.corrcoef(output_vs[:, lr_start: lr_end])
    dLdcorr = np.array(-(target_corrcoef - output_corrcoef))  # shape (N_output, N_output)
    dLdcorr[np.isnan(dLdcorr)] = 0.
    N_output = output_vs.shape[0]
    dcorrdxt = np.zeros((N_output, N_output, lr_end - lr_start))
    for i in range(N_output):
        x = output_vs[i, lr_start: lr_end]
        mean_x = np.mean(x)
        std_x = np.std(x)
        for j in range(N_output):
            y = output_vs[j, lr_start: lr_end]
            mean_y = np.mean(y)
            std_y = np.std(y)
            cov_xy = np.mean((x - mean_x) * (y - mean_y))
            dcov_xydxt = ((y - mean_y) - np.mean(y - mean_y)) / (lr_end - lr_start)
            dstd_xdxt = ((x - mean_x) - np.mean(x - mean_x)) / (std_x * (lr_end - lr_start))
            dcorrxydxt = (dcov_xydxt * std_x * std_y - cov_xy * (dstd_xdxt * std_y)) / (std_x * std_y) ** 2
            dcorrdxt[i, j, :] = dcorrxydxt
    dLtdv = np.sum(dLdcorr[:, :, np.newaxis] * dcorrdxt, axis=1)   # shape (N_output, lr_end - lr_start)
    return dLtdv

def normalize(data):
    return (data-min(data))/(max(data)-min(data))

def smooth(y, box_pts):
    box = np.ones(box_pts)/box_pts
    y_smooth = np.convolve(y, box, mode='same')
    return y_smooth

def Ca2V(ca_data):
    """
    ca_data could be shape (N_output, tstep)
    """
    n_output, tstep = ca_data.shape
    smooth_window1 = 20
    smooth_window2 = 8
    conv_window = 200
    dt = 0.01 # (s)
    ts = np.arange(0, conv_window*dt, dt)
    flt = np.exp(-ts/1) # (s/s)
    voltage_data = np.zeros((n_output, tstep-conv_window+1-smooth_window2), dtype=np.float32)
    for i in range(n_output):
        smooth_ca_data = smooth(normalize(ca_data[i,:]), smooth_window1)/2
        deconv_smooth_ca_data, _ = signal.deconvolve(smooth_ca_data, flt)
        smooth_deconv_smooth_ca_data = smooth(deconv_smooth_ca_data, smooth_window2)
        voltage_data[i,:] = smooth_deconv_smooth_ca_data[smooth_window2:]
    # plt.figure(figsize=(24,4))
    # plt.subplot(2,1,1)
    # for i in range(3):
    #     plt.plot(ca_data[i])
    # plt.xlim(0,3110)
    # plt.subplot(2,1,2)
    # for i in range(3):
    #     plt.plot(voltage_data[i])
    # plt.xlim(0,3110)
    # plt.savefig(os.path.join(OUTPUT_PATH, 'ca2v.png'))
    # exit()
    return voltage_data

def interpolate_data(src_time, src_data, dst_tstop, dst_dt):
    assert src_time[-1] >= dst_tstop and len(src_data)
    f = interpolate.interp1d(src_time[:src_data.shape[1]], src_data, kind="quadratic")
    dst_time = np.arange(0, dst_tstop+dst_dt, dst_dt)
    interpolated_data = f(dst_time)
    return interpolated_data

def map_data(data, map_range):
    ret = (data-np.min(data))/(np.max(data)-np.min(data))*(map_range[1] - map_range[0]) + map_range[0]
    return ret


def train(net: Network, output_names, input_is, target):
    net.set_outputs(output_names)

    lr_start = 0
    lr_end = int(tstop / dt)

    x = np.copy(input_is)

    # Adam params
    beta_1 = 0.9
    beta_2 = 0.999
    epsilon = 1e-9
    if ADAM_W:
        adam_m_w = 0.
        adam_v_w = 0.
        beta_1_t_w = 1.
        beta_2_t_w = 1.
    if ADAM_X:
        adam_m_x = 0.
        adam_v_x = 0.
        beta_1_t_x = 1.
        beta_2_t_x = 1.

    opt_epoch = -1
    opt_w = cp.asnumpy(net.w)
    opt_x = np.copy(x)
    opt_mean_error = 1e100
    if ADAM_W:
        opt_adam_params_w = copy.deepcopy((adam_m_w, adam_v_w, beta_1_t_w, beta_2_t_w))
    if ADAM_X:
        opt_adam_params_x = copy.deepcopy((adam_m_x, adam_v_x, beta_1_t_x, beta_2_t_x))
    alpha_multiplier = 0.3

    train_error = []
    for epoch in range(epochs):
        start_time = time.time()
        alpha_w = alpha_w0 / (1 + alpha_d * epoch)
        alpha_x = alpha_x0 / (1 + alpha_d * epoch)

        net._reset_lr_records()

        h.t = 0
        h.tstop = tstop
        h.secondorder = 0

        h.finitialize(v_r)
        h.fcurrent()

        # stimulation
        tstep = 0
        pre_time = time.time()
        while h.t < h.tstop:
            if PRINT_TIMESTEP:
                now_time = time.time()
                print(f'epoch: {epoch}, t: {h.t}, used: {now_time-pre_time:.3f}s', end='\r')
                pre_time = now_time
            for ind, id in enumerate(net.input_ids):
                net.input_synlist[id].amp = x[ind, tstep]
                # net.input_synlist[id].vpre = x[ind, tstep]
            h.fadvance()
            tstep += 1
            net.update_dvdw(tstep, percise=PERCISE)
        print('')

        output_vs = []
        for cn in output_names:
            id = net.cells_id_dic[cn]
            output_vs.append(np.array(net.output_vlist[id]))
        output_vs = np.array(output_vs)     # shape (N_output, tstep)

        if epoch == 0:
            np.save(os.path.join(OUTPUT_PATH, f'v_initial_{PREFIX}_{SUFFIX}.npy'), np.array(output_vs))
        else:
            np.save(os.path.join(OUTPUT_PATH, f'v_final_{PREFIX}_{SUFFIX}.npy'), np.array(output_vs))

        if TARGET_MODE == 'corr':
            output_corrcoef = np.corrcoef(output_vs[:, lr_start: lr_end])
            mean_error = np.abs(target - output_corrcoef)
            mean_error[np.isnan(mean_error)] = 0.
            mean_error = 0.5 * np.mean(mean_error ** 2)
            dLtdv = cal_corrcoef_dLtdv(output_vs, target, lr_start, lr_end)
            output_vs_l2loss = map_data((output_vs[:, lr_start: lr_end]-(-35)), [np.min(dLtdv), np.max(dLtdv)])
            dLtdv -= output_vs_l2loss
        elif TARGET_MODE == 'traces':
            l2_loss = 0.5 * np.mean(np.abs(target - output_vs) ** 2, axis=0)
            mean_error = np.mean(l2_loss[lr_start: lr_end])
            dLtdv = np.array(-(target - output_vs) / net.N_output)
            dLtdv = dLtdv[:, lr_start: lr_end]
        else:
            raise ValueError("Target mode must be 'corr' / 'traces'")
        train_error.append(mean_error)
        logger.info(f'epoch: {epoch}, mean error: {mean_error:.5g}')

        if mean_error < opt_mean_error:
            opt_epoch = epoch
            opt_w = cp.asnumpy(net.w).copy()
            opt_x = np.copy(x)
            opt_mean_error = mean_error
            if ADAM_W:
                opt_adam_params_w = copy.deepcopy((adam_m_w, adam_v_w, beta_1_t_w, beta_2_t_w))
            if ADAM_X:
                opt_adam_params_x = copy.deepcopy((adam_m_x, adam_v_x, beta_1_t_x, beta_2_t_x))
            np.save(os.path.join(OUTPUT_PATH, f'weights_optimal_{PREFIX}_{SUFFIX}.npy'), opt_w)
            np.save(os.path.join(OUTPUT_PATH, f'x_optimal_{PREFIX}_{SUFFIX}.npy'), opt_x)
            logger.info('optimal weights & x saved')
        elif epoch - opt_epoch >= 3:
            # retreat
            logger.info(f'no improvement since epoch {opt_epoch} for 10 epochs, restore weights & x, learning rate *= 0.3')
            net.set_weights(opt_w)
            x = np.copy(opt_x)
            if ADAM_W:
                adam_m_w, adam_v_w, beta_1_t_w, beta_2_t_w = copy.deepcopy(opt_adam_params_w)
            if ADAM_X:
                adam_m_x, adam_v_x, beta_1_t_x, beta_2_t_x = copy.deepcopy(opt_adam_params_x)
            opt_epoch = epoch
            alpha_multiplier *= 0.8
            continue
        alpha_w *= alpha_multiplier
        alpha_x *= alpha_multiplier

        dw, dx = net.get_dw_dx(dLtdv, lr_start, lr_end)

        if ADAM_W:
            adam_m_w = beta_1 * adam_m_w + (1. - beta_1) * dw
            adam_v_w = beta_2 * adam_v_w + (1. - beta_2) * dw * dw
            beta_1_t_w = beta_1_t_w * beta_1
            beta_2_t_w = beta_2_t_w * beta_2
            m_hat_w = adam_m_w / (1. - beta_1_t_w)
            v_hat_w = adam_v_w / (1. - beta_2_t_w)
            dw = m_hat_w / (np.sqrt(v_hat_w) + epsilon)
        if ADAM_X:
            adam_m_x = beta_1 * adam_m_x + (1. - beta_1) * dx
            adam_v_x = beta_2 * adam_v_x + (1. - beta_2) * dx * dx
            beta_1_t_x = beta_1_t_x * beta_1
            beta_2_t_x = beta_2_t_x * beta_2
            m_hat_x = adam_m_x / (1. - beta_1_t_x)
            v_hat_x = adam_v_x / (1. - beta_2_t_x)
            dx = m_hat_x / (np.sqrt(v_hat_x) + epsilon)
        dw *= alpha_w
        dx *= alpha_x

        tmp_w = cp.asnumpy(net.w)
        logger.info(f'w gap max: {np.max(tmp_w[net.pgap]):.5g}, min: {np.min(tmp_w[net.pgap]):.5g}')
        logger.info(f'w syn max: {np.max(tmp_w[net.psyn]):.5g}, min: {np.min(tmp_w[net.psyn]):.5g}')
        logger.info(f'x max: {np.max(x):.5g}, min: {np.min(x):.5g}')
        logger.info(f'dw gap max: {np.max(dw[net.pgap]):.5g}, min: {np.min(dw[net.pgap]):.5g}')
        logger.info(f'dw syn max: {np.max(dw[net.psyn]):.5g}, min: {np.min(dw[net.psyn]):.5g}')
        logger.info(f'dx max: {np.max(dx):.5g}, min: {np.min(dx):.5g}')

        net.update_weights(dw)
        x[lr_start: lr_end] += dx - 1.5*x[:,lr_start: lr_end]

        net.save_weights(path=os.path.join(OUTPUT_PATH, f'weights_train_{PREFIX}_{SUFFIX}.npy'))
        np.save(os.path.join(OUTPUT_PATH, f'x_train_{PREFIX}_{SUFFIX}.npy'), x)
        logger.info('weights & x saved')
        logger.info(f'time cost: {time.time()-start_time}')

    return train_error


def test(net: Network, output_names):
    opt_w = np.load(os.path.join(OUTPUT_PATH, f'weights_optimal_{PREFIX}_{SUFFIX}.npy'))
    opt_x = np.load(os.path.join(OUTPUT_PATH, f'x_optimal_{PREFIX}_{SUFFIX}.npy'))
    net.set_weights(opt_w)
    synlist = {}
    syn_cnt = 0
    for i in net.synlist.keys():
        synlist[i] = {}
        for j in net.synlist[i].keys():
            syninfo_list = net.synlist[i][j]
            synlist[i][j] = []
            for syninfo in syninfo_list:
                try:
                    syninfo.syn.Vth
                    synlist[i][j].append([syninfo.id, syninfo.point, 'syn', syninfo.syn.w * 1e4, syninfo.p])
                except:
                    synlist[i][j].append([syninfo.id, syninfo.point, 'gj', syninfo.syn.w, syninfo.p])
                syn_cnt += 1
    print("synapse cnt:", syn_cnt)
    pickle.dump(synlist, open(os.path.join(OUTPUT_PATH, f"net_synlist.pkl"), "wb"))
    print("saved network ")
    net.set_outputs(output_names)
    x = np.copy(opt_x)
    net.input_vs = x

    h.t = 0
    h.tstop = tstop
    h.secondorder = 0
    h.finitialize(v_r)
    h.fcurrent()

    # stimulation
    tstep = 0
    print("start", net.cells["AWAL"].Soma(0.5).v)
    while h.t < h.tstop:
        if PRINT_TIMESTEP:
            print(f't: {h.t}/{h.tstop}', end='\r')
        for ind, cid in enumerate(net.input_ids):
            net.input_synlist[cid].amp = x[ind, tstep]
            # net.input_synlist[id].vpre = x[ind, tstep]
        h.fadvance()
        tstep += 1
    print('')

    output_vs = []
    for cn in output_names:
        id = net.cells_id_dic[cn]
        output_vs.append(np.array(net.output_vlist[id]))
    output_vs = np.array(output_vs)     # shape (N_output, tstep)
    return output_vs


if __name__ == "__main__":
    import logging
    import sys

    logger = logging.getLogger()
    logger.setLevel(logging.INFO)
    formatter = logging.Formatter("%(asctime)s - %(filename)s[line:%(lineno)d] - %(levelname)s: %(message)s")

    sh = logging.StreamHandler(sys.stdout)
    sh.setLevel(logging.INFO)
    sh.setFormatter(formatter)
    logger.addHandler(sh)

    fh = logging.FileHandler(os.path.join(OUTPUT_PATH, f'log_{PREFIX}_{SUFFIX}.txt'))
    fh.setLevel(logging.INFO)
    fh.setFormatter(formatter)
    logger.addHandler(fh)

    logger.info('\n###################################################################################################\n')

    np.random.seed(random_seed)

    # setup h
    h.load_file('stdrun.hoc')
    h.dt = dt

    input_names = config["search_config"]["input_cell_names"]
    print("input names:", input_names)

    # data_config = config["data_config"]
    # input_is = getattr(data_factory, data_config['factory_name'])(num=len(input_names),
    #                                                               tstop=tstop,
    #                                                               dt=dt,
    #                                                               **data_config['args']) * 1e-3
    input_is = np.random.normal(loc=0., scale=1e-3, size=(len(input_names), int(tstop / dt))) + 0.03
    output_names_config = config["search_config"]["output_cell_names"]
    if TARGET_MODE == 'corr':
        with open(os.path.join("components", "cb2022_data", "Ca_corr_mat_cell_name.txt")) as f:
            output_names_target = f.read().split("\t")
        ca_corr = np.loadtxt(os.path.join("components", "cb2022_data", "Ca_corr_mat.txt"))
        output_names = []
        output_ids = []
        for i, cn in enumerate(output_names_target):
            if cn in output_names_config:
                output_names.append(cn)
                output_ids.append(i)
        target = ca_corr[output_ids,:][:,output_ids]
    elif TARGET_MODE == 'traces':
        # with open(os.path.join("components", "cb2022_data", "Ca_traces_cell_name.txt")) as f:
        #     tmp_names = f.read().split("\t")
        # record_ids = []
        # output_names = []
        # for id, name in enumerate(tmp_names):
        #     try:
        #         _ = int(name)
        #     except ValueError:
        #         if name in output_names_config:
        #             record_ids.append(id)
        #             output_names.append(name)
        # ca_tline = np.loadtxt(os.path.join("components", "cb2022_data", "Ca_traces_time.txt"))
        # cas = np.loadtxt(os.path.join("components", "cb2022_data", "Ca_traces.txt"))[:, record_ids].T
        # target = interpolate_data(ca_tline*1000, Ca2V(cas), tstop, dt)
        # target = map_data(target, [-80,20])
        muscle_path = os.path.join(os.path.dirname(__file__),"components", "gim_muscle_data", "worm_muscles_300_220428-152601.json")
        muscle_signal = np.transpose(np.array(json.load(open(muscle_path, 'r+'))["Frames"]))
        muscle_time = np.arange(0, 10000, 1000/30)
        target = interpolate_data(muscle_time, muscle_signal, tstop, dt)
        target = map_data(target, [-80,20])
        output_names = config["search_config"]["output_cell_names"]
    else:
        raise ValueError("Target mode must be one of 'corr' or 'traces'.")
    print("output names:", output_names)
    print("target:", target)

    net_config = config["config"]
    lr_config = {
        'v_r': v_r,
        'ngpu': ngpu,
        'K_max_t': K_max_t,
        'K_filename': K_filename,
        'K_nblock': K_nblock,
        'w_gap_max': w_gap_max,
        'w_gap_min': w_gap_min,
        'w_syn_max': w_syn_max,
        'w_syn_min': w_syn_min
    }

    eworm_net = Network(net_config, lr_config, random_seed)
    eworm_net.read_cells_neurite_connection(connection_file, input_names)

    optimal_weight_path = os.path.join(OUTPUT_PATH, f'weights_train_{PREFIX}_{SUFFIX}.npy')
    optimal_input_path = os.path.join(OUTPUT_PATH, f'x_train_{PREFIX}_{SUFFIX}.npy')
    if os.path.exists(optimal_weight_path) and os.path.exists(optimal_input_path):
        opt_w = np.load(optimal_weight_path)
        opt_x = np.load(optimal_input_path)
        eworm_net.set_weights(opt_w)
        input_is = None
        input_is = np.copy(opt_x)
        print("optimal weight and input are loaded!")


    # set initial weights
    # w_gap = np.random.uniform(low=, high=, size=(eworm_net.N,))
    # w_syn = np.random.normal(loc=0., scale=1e-5, size=(eworm_net.N,))

    ### train ###
    if RUN_MODE == 'train':
        train_error = train(eworm_net, output_names, input_is, target)
        np.save(os.path.join(OUTPUT_PATH, f'error_{PREFIX}_{SUFFIX}.npy'), train_error)

    ### test ###
    if RUN_MODE == 'test':
        output_vs = test(eworm_net, output_names)
        plt.figure(figsize=(40,6), dpi=200)
        plt.subplot(3,1,1)
        plt.title("input (simulation)")
        for input in eworm_net.input_vs:
            plt.plot(input * 1000)
        plt.xlabel('time step (dt = 0.5ms)')
        plt.ylabel('current (pA)')
        plt.subplot(3,1,2)
        plt.title("output (simulation)")
        for output in output_vs:
            plt.plot(output)
        plt.xlabel('time step (dt = 0.5ms)')
        plt.ylabel('voltage (mV)')
        if TARGET_MODE == "traces":
            plt.subplot(3,1,3)
            plt.title("output (experiment)")
            for output in target:
                plt.plot(output)
        plt.tight_layout()
        plt.savefig(os.path.join(OUTPUT_PATH, "voltage_traces.png"))

        sim_corr = np.corrcoef(output_vs)
        fig = plt.figure(figsize=(48,24), dpi=200)
        ax = fig.add_subplot(121)
        ax.set_title("Simulation", fontsize=40)
        ax.set_yticks(range(len(output_names)))
        ax.set_yticklabels(output_names, fontsize=20)
        ax.set_xticks(range(len(output_names)))
        ax.set_xticklabels(output_names, fontsize=18)
        im = ax.imshow(sim_corr, cmap=plt.cm.cool, vmin = -1, vmax = 1)
        plt.xticks(rotation=45)
        plt.rcParams['font.size'] = 30
        plt.colorbar(im, fraction=0.0452)
        plt.tight_layout()
        if TARGET_MODE == "corr":
            ax = fig.add_subplot(122)
            ax.set_title("Experiment", fontsize=40)
            ax.set_yticks(range(len(output_names)))
            ax.set_yticklabels(output_names, fontsize=20)
            ax.set_xticks(range(len(output_names)))
            ax.set_xticklabels(output_names, fontsize=18)
            im = ax.imshow(target, cmap=plt.cm.cool, vmin = -1, vmax = 1)
            plt.xticks(rotation=45)
            plt.rcParams['font.size'] = 30
            plt.colorbar(im, fraction=0.0452)
        plt.tight_layout()
        fig.savefig(os.path.join(OUTPUT_PATH, "corr_test.png"))
