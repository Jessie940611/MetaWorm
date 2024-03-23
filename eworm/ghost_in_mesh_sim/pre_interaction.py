"""This file is to interact with GhostInMesh
neural_model_output: to generate neural model output
train_neuron_muscle_map: to train CNN which transfer from motor neuron voltage to muscle signal
"""
import os
import pickle
import torch
import torch.nn as nn
from matplotlib import pyplot as plt
import numpy as np
from shutil import copyfile
from eworm.utils import func, data_factory
from eworm.network import *
from neuron import h
from tqdm import trange


class CNN(nn.Module):
    """motor neuron voltage mapping to muscle cell signal"""
    def __init__(self):
        super(CNN, self).__init__()
        self.conv1 = nn.Conv1d(
                in_channels=80,              # input height
                out_channels=96,             # n_filters
                kernel_size=21,              # filter size
        ) # output shape (batch_size, 96, 300)

    def forward(self, neural_voltage):
        """forward propagation"""
        output = self.conv1(neural_voltage)
        return output


class CNN2(nn.Module):
    """motor neuron voltage mapping to muscle cell signal"""
    def __init__(self, in_channel):
        super(CNN2, self).__init__()
        self.conv1 = nn.Conv1d(
                in_channels=in_channel,      # input height
                out_channels=1,              # n_filters
                kernel_size=21,              # filter size
        ) # output shape (batch_size, 1, 300)

    def forward(self, neural_voltage):
        """forward propagation"""
        output = self.conv1(neural_voltage)
        return output


def neural_model_output(config):
    """generate neural model output, input voltage"""
    group_name = config["group_name"]
    config_path = config["config_path"]
    abs_circuit_path = config["abs_circuit_path"]
    sim_config = config["sim_config"]
    loc_path = config["loc_path"]
    input_cell_name = config["input_cell_name"]
    output_cell_name = config["output_cell_name"]

    group_path = os.path.join("output", group_name)
    os.makedirs(group_path, exist_ok=True)
    config = func.load_json(config_path)["config"]
    abs_circuit = pickle.load(open(abs_circuit_path, 'rb'))
    copyfile(config_path, os.path.join(group_path, f"{group_name}_config.json"))
    copyfile(abs_circuit_path, os.path.join(group_path, f"{group_name}_abscircuit.pkl"))
    circuit = transform.abstract2detailed(abs_circuit, config, load_hoc=True, rec_voltage=True)
    # make input connections
    circuit.input_connections = []
    for cn in input_cell_name:
        input_conn = detailed_circuit.Connection(None, circuit.cell(cell_name=cn).segments[0], 'syn', 10)
        circuit.add_connection(input_conn)
    # simulate circuit
    input_traces = data_factory.ghost_in_mesh_data_factory(len(input_cell_name), sim_config["tstop"], sim_config["dt"], loc_path)
    output_traces = circuit.simulation(sim_config, input_traces, input_cell_name, output_cell_name)
    # save the motor neuron voltage
    neural_voltage_path = os.path.join(group_path, f"{group_name}_model_output_soma.pkl")
    pickle.dump(output_traces, open(neural_voltage_path, 'wb'))
    # figure
    func.visualize_io_data(input_traces, output_traces, os.path.join(group_path, f"{group_name}_model_output.png"),
                      time_axis=np.linspace(sim_config['dt'], sim_config['tstop'], int(sim_config['tstop']/sim_config['dt'])),
                      input_label=input_cell_name,
                      output_label=output_cell_name,
                      x_label="Time (ms)", y_label="Voltage (mV)",
                      title="Circuit Search")


def add_iclamp(circuit, input_cell_names):
    circuit.iclamp = []
    for cell_name in input_cell_names:
        syn = h.IClamp(circuit.cell(cell_name=cell_name).segments[0].hoc_obj)
        syn.amp = 0.
        syn.delay = 0.
        syn.dur = 1e9
        circuit.iclamp.append(syn)


def neural_model_output_current(config, plot=True):
    """generate neural model output, input current"""
    group_name = config["group_name"]
    config_path = config["config_path"]
    abs_circuit_path = config["abs_circuit_path"]
    sim_config = config["sim_config"]
    loc_path = config["loc_path"]
    input_cell_name = config["input_cell_name"]
    output_cell_name = config["output_cell_name"]

    group_path = os.path.join("output", group_name)
    os.makedirs(group_path, exist_ok=True)
    config = func.load_json(config_path)["config"]
    abs_circuit = pickle.load(open(abs_circuit_path, 'rb'))
    copyfile(config_path, os.path.join(group_path, f"{group_name}_config.json"))
    copyfile(abs_circuit_path, os.path.join(group_path, f"{group_name}_abscircuit.pkl"))
    circuit = transform.abstract2detailed(abs_circuit, config, load_hoc=True, rec_voltage=True)
    
    # make input connections
    input_traces = data_factory.ghost_in_mesh_data_factory(len(input_cell_name), tstop=sim_config['tstop'], dt=sim_config['dt'], loc_path=loc_path)/1000
    add_iclamp(circuit, input_cell_name)

    if plot:
        plt.figure(figsize=(12,3), dpi=200)
        for v, cn in zip(input_traces, input_cell_name):
            plt.plot(np.arange(0, 10000, 5/3)/1000, v, label=cn)
        plt.xticks(np.arange(0, 10, 0.9))
        plt.legend(ncol=2)
        plt.xlabel("Time (s)")
        plt.ylabel("Voltage (mV)")
        plt.tight_layout()
        plt.savefig(os.path.join(group_path, f"{group_name}_input.png"))

    # simulation
    trace_length = int(sim_config['tstop'] / sim_config['dt'])
    h.dt, h.tstop = sim_config['dt'], sim_config['tstop']
    h.steps_per_ms = int(1 / sim_config['dt'])
    h.secondorder = sim_config['secondorder']
    h.finitialize(sim_config['v_init'])
    assert input_traces.shape == (len(input_cell_name), trace_length)
    for time_step in trange(trace_length):
        for i,_ in enumerate(input_cell_name):
            circuit.iclamp[i].amp = input_traces[i][time_step]
        h.fadvance()
    
    output_traces = []
    for cn in output_cell_name:
        output_traces.append(circuit.cell(cell_name=cn).segments[0].voltage)
    output_traces = np.array(output_traces)[:,:trace_length]
    print(output_traces.shape)
    # save the motor neuron voltage
    neural_voltage_path = os.path.join(group_path, f"{group_name}_model_output_soma.pkl")
    pickle.dump(output_traces, open(neural_voltage_path, 'wb'))
    # figure
    func.visualize_io_data(input_traces, output_traces, os.path.join(group_path, f"{group_name}_model_output.png"),
                      time_axis=np.linspace(sim_config['dt'], sim_config['tstop'], int(sim_config['tstop']/sim_config['dt'])),
                      input_label=input_cell_name,
                      output_label=output_cell_name,
                      x_label="Time (ms)", y_label="Voltage (mV)",
                      title="Circuit Search")


def train_neuron_muscle_map(config):
    """train CNN which transfer from motor neuron voltage to muscle signal"""
    group_name = config["group_name"]
    muscle_path = config["muscle_path"]
    muscle_sim_config = config["muscle_sim_config"]

    group_path = os.path.join("output", group_name)
    if not os.path.exists(group_path):
        os.makedirs(group_path)
    # load motor neuron voltage
    neural_voltage_path = os.path.join(group_path, f"{group_name}_model_output_soma.pkl")
    neuron_voltage = np.array(pickle.load(open(neural_voltage_path, "rb"))) # (80, 6000)
    neuron_voltage = np.mean(neuron_voltage.reshape((80, 300, 20)), axis=2)
    # load target muscle signal
    muscle_signal = np.transpose(np.array(func.load_json(muscle_path)["Frames"]))
    # figure
    plt.figure(figsize=(10,3))
    for i in range(muscle_signal.shape[0]):
        plt.plot(np.linspace(muscle_sim_config["dt"], muscle_sim_config["tstop"], int(muscle_sim_config["tstop"]/muscle_sim_config["dt"])), muscle_signal[i,:])
    plt.xlabel("time (s)")
    plt.ylabel("target muscle signal")
    plt.ylim(-0.02,0.82)
    plt.tight_layout()
    plt.savefig(os.path.join(group_path, f"{group_name}_target_muscle_signal.png"))

    muscle_signal = muscle_signal * 100 - 80
    # training neural voltage to muscle signal mapping
    neuron_voltage = neuron_voltage[:,20:]
    muscle_signal = muscle_signal[:,20:]
    EPOCH = 501
    LR = 0.01
    cnn = CNN()
    optimizer = torch.optim.Adam(cnn.parameters(), lr=LR)
    loss_func = nn.MSELoss()
    b_x = torch.tensor(torch.from_numpy(neuron_voltage)).to(torch.float32).unsqueeze(0).unsqueeze(0)
    pad = nn.ReflectionPad2d(padding=(10, 10, 0, 0))
    b_x = torch.squeeze(pad(b_x), dim=0)
    b_y = torch.tensor(torch.from_numpy(muscle_signal)).to(torch.float32)
    loss_arr = np.zeros([EPOCH,], dtype=np.float)
    for epoch in range(EPOCH):
        output = cnn(b_x)
        loss = loss_func(output, b_y)
        optimizer.zero_grad()      # clear gradients for this training step
        loss.backward()            # backpropagation, compute gradients
        optimizer.step()           # apply gradients
        loss_arr[epoch] = loss.data.numpy()
        if epoch % 50 == 0:
            print(f'Epoch: {epoch} | train loss: {loss.data.numpy():.4f}')
    model_path = os.path.join(group_path, f"{group_name}_cnn_model.pt")
    torch.save(cnn.state_dict(), model_path)
    # figure
    nrow, ncol = 8, 12
    plt.figure(figsize=(nrow*2, ncol*2))
    localw = cnn.conv1.weight.cpu().clone()
    cnt = 0
    for i in range(nrow):
        for _ in range(ncol):
            plt.subplot(nrow, ncol, cnt+1)
            plt.title(str(cnt))
            plt.imshow(localw[cnt,:,:].detach(), cmap='viridis')
            cnt += 1
    plt.tight_layout()
    plt.savefig(os.path.join(group_path, f"{group_name}_cnn_kernel.png"))
    plt.figure()
    plt.plot(loss_arr)
    plt.savefig(os.path.join(group_path, f"{group_name}_cnn_loss.png"))

    # show prediction
    prediction = (cnn(b_x).data.numpy() + 80)/ 100
    # figure
    plt.figure(figsize=(10,3))
    for i in prediction[0]:
        # plt.plot(np.linspace(muscle_sim_config["dt"], muscle_sim_config["tstop"], int(muscle_sim_config["tstop"]/muscle_sim_config["dt"])), i)
        plt.plot(i)
    plt.ylim(-0.02,0.82)
    plt.xlabel("time (s)")
    plt.ylabel("predicted muscle signal")
    plt.tight_layout()
    plt.savefig(os.path.join(group_path, f"{group_name}_cnn_prediction.png"))

    prediction = np.squeeze(np.transpose(prediction))
    # save prediction (seq: DR0-24, VR0-24, DL0-24, VL0-24)
    with open(os.path.join(group_path, f"{group_name:s}_eworm.muscle-220428-152601.txt"), "w") as file:
        for i in range(prediction.shape[0]):
            for _ in range(8):
                for j in range(prediction.shape[1]):
                    if j == 0:
                        file.write(f"{prediction[i][j]:.10f}")
                    else:
                        file.write(f"{ prediction[i][j]:.10f}")
                file.write("\n")


def train_neuron_muscle_detail_map(config):
    """train CNNs for each muscle cell.
       CNNs transfer from motor neuron voltage to muscle signal."""
    group_name = config["group_name"]
    muscle_path = config["muscle_path"]
    muscle_sim_config = config["muscle_sim_config"]

    group_path = os.path.join("output", group_name)
    assert os.path.exists(group_path)
    # load motor neuron voltage
    neural_voltage_path = os.path.join(group_path, f"{group_name}_model_output_soma.pkl")
    neuron_voltage = np.array(pickle.load(open(neural_voltage_path, "rb")))
    neuron_voltage = np.mean(neuron_voltage.reshape((80, 300, 20)), axis=2)
    # load target muscle signal
    muscle_signal = np.transpose(np.array(func.load_json(muscle_path)["Frames"]))
    # figure
    plt.figure(figsize=(10,3))
    for i in range(muscle_signal.shape[0]):
        plt.plot(np.linspace(muscle_sim_config["dt"], muscle_sim_config["tstop"], int(muscle_sim_config["tstop"]/muscle_sim_config["dt"])), muscle_signal[i,:])
    plt.xlabel("time (s)")
    plt.ylabel("target muscle signal")
    plt.ylim(-0.02,0.82)
    plt.tight_layout()
    plt.savefig(os.path.join(group_path, f"{group_name}_target_muscle_signal.png"))

    muscle_signal = muscle_signal * 100 - 80
    # training neural voltage to muscle signal mapping

    neuron_muscle_matrix, _, _ = func.read_excel(file_name=config["neuron_muscle_matrix"],
                                               sheet_name="Sheet3")
    EPOCH = 1501
    LR = 0.001
    for m_i in range(96):
        linked_neuron_id = []
        for n_i in range(len(config["output_cell_name"])):
            if len(str(neuron_muscle_matrix.cell_value(n_i + 1, m_i + 1))) > 0:
                linked_neuron_id.append(n_i)
        cnn = CNN2(in_channel=len(linked_neuron_id))
        optimizer = torch.optim.Adam(cnn.parameters(), lr=LR)
        loss_func = nn.MSELoss()
        b_x = torch.tensor(torch.from_numpy(neuron_voltage[linked_neuron_id])).to(torch.float32).unsqueeze(0).unsqueeze(0)
        pad = nn.ReflectionPad2d(padding=(10, 10, 0, 0))
        b_x = torch.squeeze(pad(b_x), dim=0)
        b_y = torch.tensor(torch.from_numpy(np.expand_dims(muscle_signal[m_i], axis=0))).to(torch.float32)
        loss_arr = np.zeros([EPOCH,], dtype=np.float)
        for epoch in range(EPOCH):
            output = cnn(b_x)
            loss = loss_func(output, b_y)
            optimizer.zero_grad()      # clear gradients for this training step
            loss.backward()            # backpropagation, compute gradients
            optimizer.step()           # apply gradients
            loss_arr[epoch] = loss.data.numpy()
            # if epoch % 50 == 0:
            #     print(f'Epoch: {epoch} | train loss: {loss.data.numpy():.4f}')
        print(f"muscle no.{m_i}, loss decreased from {loss_arr[0]:.2f} to {loss_arr[-1]:.2f}", end='\r')
        model_path = os.path.join(group_path, f"{group_name}_cnn_model_{m_i}.pt")
        torch.save(cnn.state_dict(), model_path)
        # plt.figure()
        # plt.plot(loss_arr)
        # plt.savefig(os.path.join(group_path, f"{group_name}_cnn_loss_{m_i}.png"))

        # show prediction
        prediction = (cnn(b_x).data.numpy() + 80)/ 100
        # figure
        plt.figure(figsize=(10,2))
        x = np.linspace(muscle_sim_config["dt"], muscle_sim_config["tstop"], int(muscle_sim_config["tstop"]/muscle_sim_config["dt"]))
        plt.plot(x, prediction[0][0], label="prediction")
        plt.plot(x, (muscle_signal[m_i]+80)/100, label="ground truth")
        plt.ylim(-0.02,0.82)
        plt.title(f"muscle no.{m_i}")
        plt.legend()
        plt.xlabel("time (s)")
        plt.ylabel("predicted muscle signal")
        plt.tight_layout()
        plt.savefig(os.path.join(group_path, f"{group_name}_cnn_prediction_{m_i}.png"))


def train_neuron_muscle_reservoir(config, plot=True):
    """train fc layer which transfer from motor neuron voltage to muscle signal"""
    group_name = config["group_name"]
    muscle_path = config["muscle_path"]
    muscle_sim_config = config["muscle_sim_config"]

    group_path = os.path.join("output", group_name)
    if not os.path.exists(group_path):
        os.makedirs(group_path)
    # load motor neuron voltage
    neural_voltage_path = os.path.join(group_path, f"{group_name}_model_output_soma.pkl")
    neuron_voltage = np.array(pickle.load(open(neural_voltage_path, "rb"))) # (80,6000)
    neuron_voltage = np.mean(neuron_voltage.reshape((80, 300, 20)), axis=2) # (80,300)
    # load target muscle signal
    muscle_signal = np.transpose(np.array(func.load_json(muscle_path)["Frames"])) # (96,300)
    # figure
    if plot:
        # neural voltage
        import seaborn
        output_neuron_name = config["output_cell_name"]
        time_trace = []
        for t in np.arange(0, 10000, 1000/30):
            if np.mod(t,900) < 0.01:
                time_trace.append(f'{t/1000:.1f}')
            else:
                time_trace.append('')
        plt.figure(figsize=(8,12), dpi=200)
        seaborn.heatmap(neuron_voltage, xticklabels=time_trace, cmap='jet', vmin=-70, vmax=-20)
        plt.yticks([*range(len(output_neuron_name))], output_neuron_name, rotation ='horizontal')
        plt.xlabel('Time (s)')
        plt.tight_layout()
        plt.savefig(os.path.join(group_path, f"{group_name}_neural_voltage_heatmap.png"))
        plt.figure(figsize=(12,6), dpi=200)
        for v, cn in zip(neuron_voltage, output_neuron_name):
            plt.plot(np.arange(0, 10000, 1000/30)/1000, v, label=cn)
        plt.xticks(np.arange(0, 10, 0.9))
        plt.legend(ncol=5)
        plt.xlabel("Time (s)")
        plt.ylabel("Voltage (mV)")
        plt.savefig(os.path.join(group_path, f"{group_name}_neural_voltage.png"))

        # muscle cell voltage
        muscle_name = ["DR01","DR02","DR03","DR04","DR05","DR06","DR07","DR08","DR09","DR10","DR11","DR12","DR13","DR14","DR15","DR16","DR17","DR18","DR19","DR20","DR21","DR22","DR23","DR24","VR01","VR02","VR03","VR04","VR05","VR06","VR07","VR08","VR09","VR10","VR11","VR12","VR13","VR14","VR15","VR16","VR17","VR18","VR19","VR20","VR21","VR22","VR23","VR24","DL01","DL02","DL03","DL04","DL05","DL06","DL07","DL08","DL09","DL10","DL11","DL12","DL13","DL14","DL15","DL16","DL17","DL18","DL19","DL20","DL21","DL22","DL23","DL24","VL01","VL02","VL03","VL04","VL05","VL06","VL07","VL08","VL09","VL10","VL11","VL12","VL13","VL14","VL15","VL16","VL17","VL18","VL19","VL20","VL21","VL22","VL23","VL24"]
        plt.figure(figsize=(8,16), dpi=200)
        seaborn.heatmap(muscle_signal * 100 - 80, xticklabels=time_trace, cmap='jet', vmin=-70, vmax=-20)
        plt.yticks([*range(len(muscle_name))], muscle_name, rotation ='horizontal')
        plt.xlabel('Time (s)')
        plt.tight_layout()
        plt.savefig(os.path.join(group_path, f"{group_name}_target_muscle_heatmap.png"))
        plt.figure(figsize=(12,4), dpi=200)
        for v, cn in zip(muscle_signal * 100 - 80, muscle_name):
            plt.plot(np.arange(0, 10000, 1000/30)/1000, v, label=cn)
        plt.xticks(np.arange(0, 10, 0.9))
        # plt.legend(ncol=5)
        plt.ylim(-90, 10)
        plt.xlabel("Time (s)")
        plt.ylabel("Voltage (mV)")
        plt.savefig(os.path.join(group_path, f"{group_name}_target_muscle_voltage.png"))


    muscle_signal = muscle_signal * 100 - 80
    # training readout map (neural voltage to muscle signal)
    # https://github.com/stevenabreu7/handson_reservoir/blob/main/handson_tutorial.ipynb
    alpha = 1e-3
    T_washout = 20
    x_train = neuron_voltage[:,T_washout:].T # (300,80)
    y_train = muscle_signal[:,T_washout:].T #(300,96)
    state_corr = x_train.T @ x_train
    cross_corr = x_train.T @ y_train
    w_out = np.linalg.inv((state_corr + alpha * np.eye(x_train.shape[1]))) @ cross_corr
    print(f"w_out shape: {w_out.shape}")

    # prediction
    prediction = (neuron_voltage.T @ w_out).T
    print(f"prediction shape: {prediction.shape}")

    # figure
    if plot:
        plt.figure(dpi=200)
        seaborn.heatmap(w_out, cmap = 'jet')
        plt.savefig(os.path.join(group_path, f"{group_name}_rsv_wout.png"))
        plt.figure(figsize=(8,16), dpi=200)
        seaborn.heatmap(prediction, xticklabels=time_trace, cmap='jet', vmin=-70, vmax=-20)
        plt.yticks([*range(len(muscle_name))], muscle_name, rotation ='horizontal')
        plt.xlabel('Time (s)')
        plt.tight_layout()
        plt.savefig(os.path.join(group_path, f"{group_name}_rsv_prediction_heatmap.png"))
        plt.figure(figsize=(12,4), dpi=200)
        for v, cn in zip(prediction, muscle_name):
            plt.plot(np.arange(0, 10000, 1000/30)/1000, v, label=cn)
        plt.xticks(np.arange(0, 10, 0.9))
        # plt.xlim(0.8,10.3)
        plt.ylim(-90, 10)
        # plt.legend(ncol=5)
        plt.xlabel("Time (s)")
        plt.ylabel("Voltage (mV)")
        plt.savefig(os.path.join(group_path, f"{group_name}_rsv_prediction_voltage.png"))

    prediction = np.squeeze(np.transpose(prediction))
    # save prediction (seq: DR0-24, VR0-24, DL0-24, VL0-24)
    with open(os.path.join(group_path, f"{group_name:s}_eworm.muscle-220428-152601.txt"), "w") as file:
        for i in range(prediction.shape[0]):
            for _ in range(8):
                for j in range(prediction.shape[1]):
                    if j == 0:
                        file.write(f"{prediction[i][j]:.10f}")
                    else:
                        file.write(f"{ prediction[i][j]:.10f}")
                file.write("\n")
