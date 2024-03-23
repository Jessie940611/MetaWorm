import interact
import numpy as np
import os

"""
class WormInEnv(object):
    def __init__(self):
        print("WormInEnv init ", os.getcwd())
        self.f = open("data/mengdi/eworm.muscle-220428-152601.txt", "r")

        interact.add_worm(np.array([2,0,0], dtype=np.float32), np.array([-2,0,0],dtype=np.float32))
        # interact.add_worm(np.array([0,2,0], dtype=np.float32), np.array([0,-2,0],dtype=np.float32))
        # interact.add_worm(np.array([0,0,2], dtype=np.float32), np.array([0,0,-2],dtype=np.float32))
        interact.add_food(np.array([0,1.5,0], dtype=np.float32))

        # interact.restore_worm(0, "data/mesh/worm1.json")
        # interact.restore_worm(1, "data/mesh/worm2.json")
        # interact.restore_worm(2, "data/mesh/worm3.json")

        self.count = 0

    def step_init(self):
        print("step init worm in env")

        loc = interact.get_head_location(0)
        print("head location", loc)

        line = self.f.readline()
        dataline = [float(x) for x in line.split()]
        datanp = np.array(dataline, dtype=np.float32)
        # print(datanp)
        interact.set_muscle_signals(datanp, 0)
        # interact.set_muscle_signals(datanp, 1)
        # interact.set_muscle_signals(datanp, 2)
        # skip 7 lines copied data
        for i in range(7):
            self.f.readline()
        self.count += 1

        # if self.count == 10:
        #     interact.save_worm(0, "data/mesh/worm1.json")
        # elif self.count == 20:
        #     interact.save_worm(1, "data/mesh/worm2.json")
        # elif self.count == 30:
        #     interact.save_worm(2, "data/mesh/worm3.json")

    def step(self):
        print("step worm in env")

        loc = interact.get_head_location(0)
        print("head location", loc)

        line = self.f.readline()
        dataline = [float(x) for x in line.split()]
        datanp = np.array(dataline, dtype=np.float32)
        # print(datanp)
        interact.set_muscle_signals(datanp, 0)
        # interact.set_muscle_signals(datanp, 1)
        # interact.set_muscle_signals(datanp, 2)

        # skip 7 lines copied data
        for i in range(7):
            self.f.readline()
"""


'''
import interact
import numpy as np
import os

class WormInEnv(object):
    def __init__(self):
        print("WormInEnv init ", os.getcwd())
        self.f = open("data/mengdi/eworm.muscle-220428-152601.txt", "r")

    def step(self):
        print("step worm in env")

        loc = interact.get_head_location()
        print("head location", loc)

        line = self.f.readline()
        dataline = [float(x) for x in line.split()]
        datanp = np.array(dataline, dtype=np.float32)
        print("datanp", datanp, datanp.shape)
        interact.set_muscle_signals(datanp)

        # skip 7 lines copied data
        for i in range(7):
            self.f.readline()
'''



import sys
sys.path.append('/home/brains/worm_simulation/neural_model/')
from network.init import *
from network.input import *
from network.simulate import set_h_before, set_h_after
from network.export import *
from utils.args import *
from utils.jsonfunc import *
import numpy as np
import math
import os
from neuron import h
from wang_sim.cnn import CNN2
import torch
import torch.nn as nn
from matplotlib import pyplot as plt
import pickle
from WormNN import WormNN

def normalize_rgb(c):
    return c / 255.0

color_list = [
    np.array([0.9, 0.9, 0.9, 0.6], dtype = np.float32),
    normalize_rgb(np.array([192,162,199,102], dtype=np.float32)),
    normalize_rgb(np.array([190,237,168,102], dtype=np.float32)),
    normalize_rgb(np.array([255,180,236,102], dtype=np.float32)),
    normalize_rgb(np.array([118,224,189,102], dtype=np.float32)),
    normalize_rgb(np.array([231,149,156,102], dtype=np.float32)),
    normalize_rgb(np.array([175,255,244,102], dtype=np.float32)),
    normalize_rgb(np.array([255,191,165,102], dtype=np.float32)),
    normalize_rgb(np.array([164,185,255,102], dtype=np.float32)),
    normalize_rgb(np.array([237,227,150,102], dtype=np.float32)),
    normalize_rgb(np.array([88,186,189,102], dtype=np.float32)),
    normalize_rgb(np.array([193,169,106,102], dtype=np.float32)),
    normalize_rgb(np.array([114,187,146,102], dtype=np.float32)),
    normalize_rgb(np.array([255,249,206,102], dtype=np.float32)),
    normalize_rgb(np.array([185,208,184,102], dtype=np.float32)),
]
class WormInEnv0(object):
    def __init__(self) -> None:
        '''
        config: a dictionary
        '''
        self.config = {
        "config_file": "/home/brains/worm_simulation/neural_model/config_all.json",
        "connection_file": "/home/brains/worm_simulation/neural_model/wang_sim/data/220527_106_circuit_121.pkl",
        "cnn_model_file": "/home/brains/worm_simulation/neural_model/wang_sim/data/newloc_cnn_model.pt",
        "output_neuron": ["RIML", "RIMR", "RMEL", "RMER", "RMED", "RMEV", "RMDDL", "RMDDR", "RMDL", "RMDR", "RMDVL", "RMDVR", "RIVL", "RIVR", "SMDDL", "SMDDR", "SMDVL", "SMDVR", "SMBDL", "SMBDR", "SMBVL", "SMBVR", "DA01", "DA02", "DA03", "DA04", "DA05", "DA06", "DA07", "DA08", "DA09", "DB01", "DB02", "DB03", "DB04", "DB05", "DB06", "DB07", "DD01", "DD02", "DD03", "DD04", "DD05", "DD06", "VA01", "VA02", "VA03", "VA04", "VA05", "VA06", "VA07", "VA08", "VA09", "VA10", "VA11", "VA12", "VB01", "VB02", "VB03", "VB04", "VB05", "VB06", "VB07", "VB08", "VB09", "VB10", "VB11", "VD01", "VD02", "VD03", "VD04", "VD05", "VD06", "VD07", "VD08", "VD09", "VD10", "VD11", "VD12", "VD13"],
        "neuron_sim_dt": 5/3,
        "motion_sim_dt": 100/3,
        "tstop": 10000,
        "v_init": -78
        }

        config_file = self.config["config_file"]
        config = load_json(config_file)
        set_h_before(config)
        self.worms = []

        # add food 
        # global
        food_loc_global = np.array([0, 0, 0], dtype=np.float32)
        interact.add_food(food_loc_global)

        # record  200 time step
        # start local [-0.02428353 -0.13023555 -0.47388658] global [-0.02428353,0.13023555,0.47388658]
        # orientation [0., 0., 1.]
        # target local [-0.3783243, 1.0820273, 1.7552527] global [-0.3783243, 1.0820273, 1.7552527]  
        # terminal local [-0.32623067 -1.0665209  -1.78049   ] global [-0.32623067  1.0665209   1.78049   ]



        start_loc = [ # global
            np.array([0.24710286, -0.7391443, -1.451586], dtype=np.float32)-np.array([-0.1894,0.3286,0.2970], dtype=np.float32),
            np.array([ 0.5456, -2.6517,  0.9685], dtype=np.float32)-np.array([-0.0581,0.7034,-0.4729], dtype=np.float32),
            np.array([0., 1., 0.], dtype=np.float32)-np.array([-1.0233,-1.1293,0.6143], dtype=np.float32),
            np.array([-0.21249998,  0.24670002, -0.3761    ],dtype=np.float32)-np.array([-0.0586,-0.0605,0.4150], dtype=np.float32),
            np.array([-0.18789999, -0.71519995,  0.1523    ],dtype=np.float32)-np.array([0.3153,0.1367,0.1495], dtype=np.float32)-np.array([-0.3676,-0.0606,0.0384],dtype=np.float32),
            np.array([-0.0456,-1.1517,0.9685],dtype=np.float32)-np.array([0.2742,-1.4208,2.8152], dtype=np.float32)-np.array([0.1868,-0.1434,-0.7496], dtype=np.float32),

            # np.array([0.24710286, -0.7391443, -0.451586], dtype=np.float32)-np.array([-0.1894,0.3286,0.2970], dtype=np.float32),
            # np.array([ 0.5456, -1.6517,  0.9685], dtype=np.float32)-np.array([-0.0581,0.7034,-0.4729], dtype=np.float32),
            # np.array([0., 0.2, 0.], dtype=np.float32)-np.array([-1.0233,-1.1293,0.6143], dtype=np.float32),
            # np.array([-0.21249998,  0.24670002, 1.4761    ],dtype=np.float32)-np.array([-0.0586,-0.0605,0.4150], dtype=np.float32),
            # np.array([-0.18789999, 0.81519995,  0.5523    ],dtype=np.float32)-np.array([0.3153,0.1367,0.1495], dtype=np.float32)-np.array([-0.3676,-0.0606,0.0384],dtype=np.float32),
            # np.array([-0.0456,-1.1517,0.9685],dtype=np.float32)-np.array([0.2742,-1.4208,2.8152], dtype=np.float32)-np.array([0.1868,-0.1434,-0.7496], dtype=np.float32),
        ] 
        orientation = [
            np.array([0., 0., 1.], dtype=np.float32),
            np.array([0., 1., 0.], dtype=np.float32),
            np.array([-1., 0., 0.], dtype=np.float32),
            np.array([0.3, 0.3, 0.3], dtype=np.float32),
            np.array([0.1, -0.7, 0.2], dtype=np.float32),
            np.array([0.4, 0.6, -0.1], dtype=np.float32),
        ]
        target_loc = [ # global
            np.array([-0.10693792,  0.21264744, -0.1702199], dtype=np.float32)-np.array([-0.1894,0.3286,0.2970], dtype=np.float32),
            np.array([-0.9544    , -4.1517    , -0.03149998], dtype=np.float32)-np.array([-0.0581,0.7034,-0.4729], dtype=np.float32),
            np.array([-0.3783243, 1.0820273, 1.7552527], dtype=np.float32)-np.array([-1.0233,-1.1293,0.6143], dtype=np.float32),
            np.array([ 1.1109622 ,  0.79784745, -1.0197198 ], dtype=np.float32)-np.array([-0.0586,-0.0605,0.4150], dtype=np.float32),
            np.array([-0.19669999, -4.9186    ,  2.1893    ], dtype=np.float32)-np.array([0.3153,0.1367,0.1495], dtype=np.float32)-np.array([-0.3676,-0.0606,0.0384],dtype=np.float32),
            np.array([-1.4304,-0.3385,0.2734],dtype=np.float32)-np.array([0.2742,-1.4208,2.8152], dtype=np.float32)-np.array([0.1868,-0.1434,-0.7496],dtype=np.float32),
        ]
        

        assert len(start_loc) == len(orientation) and len(start_loc) == len(target_loc) 
        self.worm_num = len(start_loc)

        # interact.set_fog_color(np.array([90 / 255.0, 199.0 / 255.0, 248.0 / 255.0, 1.0], dtype=np.float32))
        # interact.set_floor_color(np.array([16 / 255.0, 35 / 255.0, 79 / 255.0, 1.0], dtype=np.float32))
        # interact.set_fog_params(1, np.array([0.06,4,8], dtype=np.float32))
        
        # interact.set_fog_color(np.array([137 / 255.0, 204.0 / 255.0, 217.0 / 255.0, 1.0], dtype=np.float32))
        # interact.set_floor_color(np.array([44 / 255.0, 91 / 255.0, 103 / 255.0, 1.0], dtype=np.float32))

        interact.set_fog_color(np.array([87 / 255.0, 154.0 / 255.0, 137.0 / 255.0, 1.0], dtype=np.float32))
        interact.set_floor_color(np.array([44 / 255.0, 91 / 255.0, 80 / 255.0, 1.0], dtype=np.float32))
        interact.set_fog_params(1, np.array([0.06,4,8], dtype=np.float32))

        # interact.set_fog_color(np.array([0 / 255.0, 0.0 / 255.0, 0.0 / 255.0, 1.0], dtype=np.float32))
        # interact.set_floor_color(np.array([0 / 255.0, 0 / 255.0, 0 / 255.0, 1.0], dtype=np.float32))
        # interact.set_fog_params(1, np.array([0.06,4,8], dtype=np.float32))

        # add worms
        for i in range(self.worm_num):
            interact.add_worm(start_loc[i], orientation[i], color_list[0])
            setattr(self, "worm_%d_start_loc_local"%i, interact.world_to_worm_local(i, start_loc[i]))
            setattr(self, "worm_%d_target_loc_local"%i, interact.world_to_worm_local(i, target_loc[i])) 
            setattr(self, "worm_%d"%i, WormNN(eval("self.worm_%d_start_loc_local"%i), eval("self.worm_%d_target_loc_local"%i)))
            self.worms.append(eval("self.worm_%d"%i))
        

        # restore worm state
        self.restore_state(0)
        self.restore_state(1)
        self.restore_state(2)
        self.restore_state(3)
        self.restore_state(4)
        self.restore_state(5)



        set_h_after(self.config["neuron_sim_dt"], self.config["tstop"], self.config["v_init"])
        
        self.step_count = 0  # count time step
        self.f = open("data/mengdi/eworm.muscle.gt-220428-152601.txt", "r")


    def control_muscle(self):
        delta_con_scale_list = []
        dt_scale = int(600/30)
        # get next head location
        for i in range(self.worm_num):
            setattr(self, "next_head_loc_%d"%i, interact.get_head_location(i))
            setattr(self, "delta_con_scale_%d"%i, eval("self.worm_%i.location_to_network_input(self.next_head_loc_0)"%i))
            delta_con_scale_list.append(eval("self.delta_con_scale_%d"%i))
            setattr(self, "mnv_%d"%i, np.zeros((len(self.config["output_neuron"]), ), dtype=np.float32))
        
        # calculate motor neuron voltage
        for _ in range(dt_scale):
            mnv = self.simulate_neural_model_one_time_step(delta_con_scale_list)
            for i in range(self.worm_num):
                temp = eval("self.mnv_%d"%i) + mnv[i]
                self.__setattr__("mnv_%d"%i, temp)
        
        muscle_signal_list = []
        for i in range(self.worm_num):
            self.__setattr__("mnv_%d"%i, eval("self.mnv_%d"%i) / dt_scale)
            _m = eval("self.worm_%d.motor_neuron_volt_to_muscle_sig(self.mnv_%i)"%(i,i))
            muscle_signal_list.append(_m)

        return muscle_signal_list

    

    def step_init(self):
        self.step_count += 1

        muscle_signal_list = self.control_muscle()
        
        # use muscle signal from file
        line = self.f.readline()
        dataline = [float(x) for x in line.split()]
        datanp = np.array(dataline, dtype=np.float32)

        for i in range(self.worm_num):
            interact.set_muscle_signals(datanp, i)

        # skip 7 lines copied data
        for _ in range(7):
            self.f.readline()
        
        print("time step: %d"%self.step_count, "local", interact.get_head_location(0), 
              "world", interact.worm_local_to_world(0, interact.get_head_location(0)))



    def step(self):  # 30Hz
        self.step_count += 1

        muscle_signal_list = self.control_muscle()
       
        for i in range(self.worm_num):
            interact.set_muscle_signals(muscle_signal_list[i], i)

        for i in range(self.worm_num):
            L = interact.get_head_location(i)
            G = interact.worm_local_to_world(i, L)
            print("ts:%d, worm_%d: L:[%.4f,%.4f,%.4f], G:[%.4f,%.4f,%.4f]"
                    %(self.step_count,i,L[0],L[1],L[2],G[0],G[1],G[2]))
        
        # if self.step_count == 111:
        #     self.save_state(3,111)
        # elif self.step_count == 222:
        #     self.save_state(4,222)
        # elif self.step_count == 158:
        #     self.save_state(5,158)

        # if self.step_count ==300:
        #     base_path = "/home/brains/worm_simulation/interact/MetaWorm0509-interact-0511/MetaWorm0509/data/mesh/group_6worm/"
        #     export_morphology_and_synapse(self.worm_0.worm, base_path+"group_6worm.swc", base_path+"group_6worm.snp")
        #     for i in range(self.worm_num):
        #         export_voltage(eval("self.worm_%d.worm"%i), base_path+"group_6worm_%d.vtg"%i)

        


    def save_state(self, save_id, step_count):
        save_id = int(save_id)
        interact.save_worm(save_id, "data/mesh/mesh_%d.json"%save_id)
        eval("self.worm_%d.save_neuron_muscle(%d)"%(save_id, save_id))
        print("save worm_%d at ts %d"%(save_id, step_count))
    

    def restore_state(self, restore_id):
        restore_id = int(restore_id)
        interact.restore_worm(restore_id, "data/mesh/mesh_%d.json"%restore_id)
        eval("self.worm_%d.restore_state(%d)"%(restore_id, restore_id))
        print("restore worm_%d state"%restore_id)
        



    def simulate_neural_model_one_time_step(self, delta_con_scale_list):  # 600Hz
        ''' calculate motor neuron voltage '''
        # delta_con_scale_list: [delta_con_scale_0, delta_con_scale_1, ...]
        # run neuron simulation one time step

        for wormnn, delta_con_scale in zip(self.worms, delta_con_scale_list):
            for conn in wormnn.worm.input_connections:
                conn.connection.vpre = delta_con_scale
        h.fadvance()

        # record soma voltage
        motor_neuron_volt_list = []
        for wormnn in self.worms:
            motor_neuron_volt = np.zeros((len(self.config["output_neuron"]), ), dtype=np.float32)
            for i, cn in enumerate(self.config["output_neuron"]):
                cell = wormnn.worm.name_cell_dict[cn]
                motor_neuron_volt[i] = cell.cell.Soma(0.5).v
            motor_neuron_volt_list.append(motor_neuron_volt)

        return motor_neuron_volt_list
