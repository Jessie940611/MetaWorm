import sys
sys.path.append('/home/brains/worm_simulation/neural_model/')
from network.init import *
from network.input import *
from network.simulate import *
from network.export import *
from utils.args import *
from utils.jsonfunc import *
import numpy as np
import os
from neuron import h
from cnn import CNN, CNN2
import torch
import torch.nn as nn


class WormInEnv(object):
    def __init__(self, config) -> None:
        '''
        config: a dictionary
        '''
        self.config = config
        self.worm = None
        self.cnn = None
        self.neuron_num = 0
        self.target = config["food_location"]
        self.head_loc = config["start_location"]
        self.con = self._concentration(self.head_loc)
        self.motor_neuron_map = None
        self._define_worm_model()
        self._define_cnn_model()


    def _define_worm_model(self):
        '''initialize Celegans model and neuron simulation'''
        config_file = self.config["config_file"]
        connection_file = self.config["connection_file"]
        config = load_json(config_file)
        set_h_before(config)
        circuit = pickle.load(open(connection_file, "rb"))
        self.worm = init_network(config, circuit)
        self.neuron_num = len(self.worm.cells)
        set_h_after(self.config["neuron_sim_dt"], self.config["tstop"], self.config["v_init"])


    def _define_cnn_model(self):
        '''initialize CNN which convert motor neuron voltage to muscle signal'''
        self.cnn = CNN2()
        self.cnn.load_state_dict(torch.load(self.config["cnn_model_file"]))
        motor_neuron_volt = np.zeros([1, len(self.config["output_neuron"]), 21])   # kernel size
        self.motor_neuron_map = torch.as_tensor(torch.from_numpy(motor_neuron_volt)).to(torch.float32)


    def _concentration(self, loc):
        '''calculate atttration concentration according to head location'''
        return np.linalg.norm(self.target - loc) * -1


    def interact_with_env_one_time_step(self, next_head_loc):
        ''' calculate muscle signal
        data flow: head_loc -> concentration -> motor neuron voltage -> muscle signal
        input: next_head_loc, array (3,)
        output: muscle_signal, array (96,1)
        '''
        
        con_next = np.linalg.norm(self.target - next_head_loc) * -1
        delta_con = con_next - self.con
        
        # run neuron simulation one time step
        for conn in self.worm.input_connections:
            conn.connection.vpre = delta_con
        h.fadvance()

        # calculate motor neuron voltage
        motor_neuron_volt = np.zeros((1, len(self.config["output_neuron"]), 1), dtype=np.float32)
        for i, cn in enumerate(self.config["output_neuron"]):
            cell = self.worm.name_cell_dict[cn]
            motor_neuron_volt[0][i] = cell.cell.Soma(0.5).v
        self.head_loc = next_head_loc
        self.con = con_next
        self.motor_neuron_map = torch.cat((
            self.motor_neuron_map[:,:, 1:], 
            torch.as_tensor(torch.from_numpy(motor_neuron_volt)).to(torch.float32)
            ), 2)
        # calculate muscle signal
        muscle_signal = (self.cnn(self.motor_neuron_map).data.numpy() + 80)/ 100

        return muscle_signal


if __name__ == "__main__":
    config = {
        "config_file": "/home/brains/worm_simulation/neural_model/config_all.json",
        "connection_file": "/home/brains/worm_simulation/neural_model/wang_sim/data/220428_106_circuit_6.pkl",
        "cnn_model_file": "/home/brains/worm_simulation/neural_model/wang_sim/data/cnn_model.pt",
        "output_neuron": ["RIML", "RIMR", "RMEL", "RMER", "RMED", "RMEV", "RMDDL", "RMDDR", "RMDL", "RMDR", "RMDVL", "RMDVR", "RIVL", "RIVR", "SMDDL", "SMDDR", "SMDVL", "SMDVR", "SMBDL", "SMBDR", "SMBVL", "SMBVR", "DA01", "DA02", "DA03", "DA04", "DA05", "DA06", "DA07", "DA08", "DA09", "DB01", "DB02", "DB03", "DB04", "DB05", "DB06", "DB07", "DD01", "DD02", "DD03", "DD04", "DD05", "DD06", "VA01", "VA02", "VA03", "VA04", "VA05", "VA06", "VA07", "VA08", "VA09", "VA10", "VA11", "VA12", "VB01", "VB02", "VB03", "VB04", "VB05", "VB06", "VB07", "VB08", "VB09", "VB10", "VB11", "VD01", "VD02", "VD03", "VD04", "VD05", "VD06", "VD07", "VD08", "VD09", "VD10", "VD11", "VD12", "VD13"],
        "neuron_sim_dt": 5/3,
        "motion_sim_dt": 100/3,
        "tstop": 10000,
        "v_init": -78,
        "food_location": np.array([10,10,10]),
        "start_location": np.array([0,0,0])
    }

    wie = WormInEnv(config)
    
    # random head location, which should be export from environment simulation
    test_head_loc = np.random.uniform(low=0, high=10, size=(3,10))

    # for each time step, calculate muscle signal according to head location
    for i in range(test_head_loc.shape[1]):
        muscle_signal = wie.interact_with_env_one_time_step(test_head_loc[:,i])
