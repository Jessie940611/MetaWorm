import os
import sys
# sys.path.append('/home/brains/worm_simulation/interact/MetaWorm0509-interact-0511/MetaWorm0509/interact/interact.so')
sys.path.append('/home/brains/worm_simulation/interact/worm-simulation/build2/')
import interact
import numpy as np
import os
from neuron import h, load_mechanisms
from eworm.utils import *
from eworm.ghost_in_mesh_sim.worm_neural_network import WormNeuralNetwork
from matplotlib import pyplot as plt
import pickle


def normalize_rgb(c):
    return c / 255.0


class WormInEnv(object):
    """class for an online interaction between worm and GhostInMesh

    Attributes:
        config: dictionary, configuration of simulation
        worms: list of WormNeuralNetwork class, [worm_0, worm_1, ...]
        worm_num: int, worm number
        step_count: int, step counter, start from 0
        init_muscle_file: initial muscle file to teach body how to move
    """
    def __init__(self, color_list=None, config=None, food_location=None,
                 start_location=None, orientation=None, target_location=None) -> None:
        print(sys.path)
        print("aaaa")
        if color_list is None:
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
        group_name = "video_online"
        if config is None:
            config = {
                "config_file": os.path.join(os.path.dirname(__file__), "data", "tuned", group_name, f"{group_name}_config.json"),
                "abs_circuit_file": os.path.join(os.path.dirname(__file__), "data", "tuned", group_name, f"{group_name}_abscircuit.pkl"),
                "cnn_model_file": os.path.join(os.path.dirname(__file__), "data", "tuned", group_name, f"{group_name}_cnn_model.pt"),
                # "init_muscle_file": os.path.join(os.path.dirname(__file__), "output", "test", "gt_eworm.muscle-220428-152601.txt"),
                "init_muscle_file": "/home/brains/worm_simulation/interact/worm-simulation/build/test.npy",
                "input_neuron": ["AWAL","AWAR","AWCL","AWCR","ASKL","ASKR","ALNL","ALNR","PLML","PHAL","PHAR","URYDL","URYDR","URYVL","URYVR"],
                "output_neuron": ["RIML", "RIMR", "RMEL", "RMER", "RMED", "RMEV", "RMDDL", "RMDDR", "RMDL", "RMDR", "RMDVL", "RMDVR", "RIVL", "RIVR", "SMDDL", "SMDDR", "SMDVL", "SMDVR", "SMBDL", "SMBDR", "SMBVL", "SMBVR", "DA01", "DA02", "DA03", "DA04", "DA05", "DA06", "DA07", "DA08", "DA09", "DB01", "DB02", "DB03", "DB04", "DB05", "DB06", "DB07", "DD01", "DD02", "DD03", "DD04", "DD05", "DD06", "VA01", "VA02", "VA03", "VA04", "VA05", "VA06", "VA07", "VA08", "VA09", "VA10", "VA11", "VA12", "VB01", "VB02", "VB03", "VB04", "VB05", "VB06", "VB07", "VB08", "VB09", "VB10", "VB11", "VD01", "VD02", "VD03", "VD04", "VD05", "VD06", "VD07", "VD08", "VD09", "VD10", "VD11", "VD12", "VD13"],
                "neuron_sim_dt": 5/3,
                "motion_sim_dt": 100, # (10Hz)
                "tstop": 10000000,
                "v_init": -65,
                "secondorder": 0,
                "proprioception": False, # default False
                "detail_control": "fc",  # "one_cnn", "multi_cnn", "fc"
                "neuron_muscle_matrix": os.path.join(os.path.dirname(__file__), "..", "components", "param", "connection", "neuron_muscle.xlsx"),
                "reservoir_wout": os.path.join(os.path.dirname(__file__), "data", "tuned", group_name, f"{group_name}_wout.pkl"),
                "interaction_mode": "online",
                "export_data": True  # default False
            }
        if food_location is None:
            food_location = np.array([1.8275,-0.0276,-0.3082], dtype=np.float32)  #np.array([0, 0, 0], dtype=np.float32) # global
        if start_location is None:
            start_location = [ # global
                # np.array([1.,1.,1.], dtype=np.float32)*np.random.uniform(low=2/np.sqrt(3), high=6/np.sqrt(3))
                np.array([0.43650287, -1.0677443, -1.7485859], dtype=np.float32),
            ]
        if orientation is None:
            orientation = [
                # np.array(np.random.randn(3), dtype=np.float32)
                np.array([0., 0., 1.], dtype=np.float32),
            ]
            orientation = orientation/np.linalg.norm(orientation)
        if target_location is None:
            target_location = [ # global
                np.array([0, 0, 0], dtype=np.float32),
            ]
        self.config = config
        self.state_config = None
        self.standard_input_signal = data_factory.ghost_in_mesh_data_factory_standard(len(self.config["input_neuron"]), self.config["tstop"], self.config["neuron_sim_dt"], os.path.join("data", "state", "worm_states_300_220428-152601.json"))/1000
        interact.add_food(food_location)
        assert len(start_location) == len(orientation) and len(start_location) == len(target_location)
        interact.set_fog_color(np.array([87 / 255.0, 154.0 / 255.0, 137.0 / 255.0, 1.0], dtype=np.float32))
        interact.set_floor_color(np.array([44 / 255.0, 91 / 255.0, 80 / 255.0, 1.0], dtype=np.float32))
        interact.set_fog_params(1, np.array([0.06,4,8], dtype=np.float32))

        # add worms
        self.worms = []
        for i,_ in enumerate(start_location):
            interact.add_worm(start_location[i], orientation[i], color_list[0])
            setattr(self, f"worm_{i:d}_start_location_local", interact.world_to_worm_local(i, start_location[i]))
            setattr(self, f"worm_{i:d}_target_location_local", interact.world_to_worm_local(i, target_location[i]))
            print(start_location[i], interact.world_to_worm_local(i, start_location[i]))
            print(target_location[i], interact.world_to_worm_local(i, target_location[i]))
            setattr(self, f"worm_{i:d}", WormNeuralNetwork(eval(f"self.worm_{i:d}_start_location_local"), eval(f"self.worm_{i:d}_target_location_local"), self.config))
            self.worms.append(eval(f"self.worm_{i:d}"))
            for connection in eval(f"self.worm_{i:d}.worm.input_connections"):
                connection.hoc_obj.vpre = config["v_init"]
        self.worm_num = len(self.worms)
        # restore worm state
        # for i in range(self.worm_num):
        #     self.restore_state(i)
        self.step_count = 0  # count time step
        self.muscle_signal_list = [np.zeros((96,)) for _ in range(self.worm_num)]
        # self.init_muscle_file = open(self.config["init_muscle_file"], "r")
        self.init_muscle_file = np.load(self.config["init_muscle_file"])

        h.dt, h.tstop = config['neuron_sim_dt'], config['tstop']
        # h.steps_per_ms = int(1 / sim_config['dt'])
        h.secondorder = config['secondorder']
        h.finitialize(config['v_init'])

        self.test_m = []
        self.test_n = []
        self.test_i = []
        self.test_mn = []
        self.test_cn = []
        self.test_local_head_loc = []
        self.test_world_head_loc = []

        self.rel_x =  np.array([], dtype=np.float64)
        self.rel_y =  np.array([], dtype=np.float64)
        self.rel_z =  np.array([], dtype=np.float64)
        self.rel_vx = np.array([], dtype=np.float64)
        self.rel_vy = np.array([], dtype=np.float64)
        self.rel_vz = np.array([], dtype=np.float64)

    def control_muscle(self):
        """get head location, calculate motor neuron voltage and tranfer it to muscle signal"""
        delta_concentration_normalized_list = []
        dt_scale = int(self.config["motion_sim_dt"]/self.config["neuron_sim_dt"])
        # get next head location and the corresponding input stimulus
        for i in range(self.worm_num):
            setattr(self, f"next_head_location_{i:d}", interact.get_head_location(i))
            if self.config.get("interaction_mode", "online") == "online":
                setattr(self, f"delta_concentration_normalized_{i:d}", eval(f"self.worm_{i:d}.location_to_network_input(self.next_head_location_{i:d})"))
            elif self.config.get("interaction_mode", "online") == "offline":
                setattr(self, f"delta_concentration_normalized_{i:d}", self.standard_input_signal[0,self.step_count*dt_scale])
            delta_concentration_normalized_list.append(eval(f"self.delta_concentration_normalized_{i:d}"))
            setattr(self, f"mnv_{i:d}", np.zeros((len(self.config["output_neuron"]), ), dtype=np.float32))
            setattr(self, f"nv_{i:d}", np.zeros((eval(f"self.worm_{i:d}.nseg"), ), dtype=np.float32))
        
        # # test figure
        self.test_i.append(self.delta_concentration_normalized_0)
        # plt.figure(figsize=(12,3))
        # plt.plot(self.test_i, '*-')
        # plt.savefig("test_i.png")
        # plt.close('all')
        # with open("forfigure_input.txt", "a") as file:
        #     file.write(f"{self.delta_concentration_normalized_0:.6f}\n")

        self.test_cn.append([self.worm_0.min_ms, self.worm_0.max_ms])
        self.test_local_head_loc.append(interact.get_head_location(0))
        self.test_world_head_loc.append(interact.worm_local_to_world(0, interact.get_head_location(0)))

        # calculate motor neuron voltage
        for _ in range(dt_scale):
            mnv = self.simulate_neural_model_one_time_step(delta_concentration_normalized_list)
            for i in range(self.worm_num):
                self.__setattr__(f"mnv_{i:d}", eval(f"self.mnv_{i:d}") + mnv[i])
                self.__setattr__(f"nv_{i:d}", eval(f"self.nv_{i:d}") + eval(f"self.worm_{i:d}.get_neuron_voltage()"))
        # calculate muscle signal
        muscle_signal_list = []
        for i in range(self.worm_num):
            self.__setattr__(f"mnv_{i:d}", eval(f"self.mnv_{i:d}") / dt_scale)
            _m = eval(f"self.worm_{i:d}.motor_neuron_volt_to_muscle_sig(self.mnv_{i:d})")
            muscle_signal_list.append(_m)
            self.__setattr__(f"nv_{i:d}", eval(f"self.nv_{i:d}") / dt_scale)
        # with open("forfigure_motor_neuron.txt", "a") as file:
        #     for n in self.mnv_0:
        #         file.write(f"{n:.6f},")
        #     file.write("\n")
        # with open("forfigure_neuron.txt", "a") as file:
        #     for n in self.nv_0:
        #         file.write(f"{n:.6f},")
        #     file.write("\n")
        # with open("forfigure_muscle.txt", "a") as file:
        #     for m in muscle_signal_list[0]:
        #         file.write(f"{m:.6f},")
        #     file.write("\n")
        return muscle_signal_list

    def step_init(self):
        """move one step initially:
        to initialize the movement, target muscle signals teach body how to move, 30Hz"""
        # print("init", self.step_count)
        self.step_count += 1
        # use muscle signal from file
        # line = self.init_muscle_file.readline()
        # datanp = np.array([float(x) for x in line.split()], dtype=np.float32)
        datanp = np.array(self.init_muscle_file[self.step_count-1], dtype=np.float32)
        self.muscle_signal_list = self.control_muscle() 
        for i in range(self.worm_num):
            interact.set_muscle_signals(datanp, i)
            interact.set_neuron_voltages(eval(f"self.nv_{i:d}"))
        # skip 7 lines copied data
        # for _ in range(7):
        #     self.init_muscle_file.readline()
        print(f"init ts: {self.step_count:d}", "local", interact.get_head_location(0), "world", interact.worm_local_to_world(0, interact.get_head_location(0)))

        # # test figure
        # self.test_m.append(self.muscle_signal_list[0])
        # # self.test_n.append(self.nv_0)
        # self.test_mn.append(self.mnv_0)
        # plt.figure(figsize=(12,9))
        # plt.subplot(3,1,1)
        # tmp_m = np.array(self.test_m)
        # for i in range(96):
        #     plt.plot(tmp_m[:,i])
        # # plt.ylim(-80, 20)
        # plt.ylim(-0.2,1)
        # # plt.subplot(3,1,2)
        # # tmp_n = np.array(self.test_n)
        # # for i in range(self.worm_0.nseg):
        # #     plt.plot(tmp_n[:,i])
        # plt.subplot(3,1,3)
        # tmp_mn = np.array(self.test_mn)
        # for i in range(len(self.config["output_neuron"])):
        #     plt.plot(tmp_mn[:,i])
        # plt.savefig("test_m_n.png")
        # plt.close('all')


    def step(self):
        """move one step according to the stimulus in the environment"""
        print("step", self.step_count)
        self.step_count += 1
        self.muscle_signal_list = self.control_muscle()
        for i in range(self.worm_num):
            # muscle_data, self.state_config = muscle_playground.algorithm(interact, i, self.state_config, show_direction=True)
            # interact.set_muscle_signals(np.array(muscle_data[:, 0], dtype=np.float32), i)
            interact.set_muscle_signals(np.array(self.muscle_signal_list[i], dtype=np.float32), i)
            interact.set_neuron_voltages(eval(f"self.nv_{i:d}"))
            relative_loc_vel = np.asarray(interact.get_tracking_location(i))

        relative_loc = np.reshape(relative_loc_vel[:51], (17, 3)).transpose()
        relative_vel = np.reshape(relative_loc_vel[51:], (17, 3)).transpose()
        if self.step_count == 1:
            self.rel_x = np.expand_dims(relative_loc[0], axis=0)
            self.rel_y = np.expand_dims(relative_loc[1], axis=0)
            self.rel_z = np.expand_dims(relative_loc[2], axis=0)
            self.rel_vx = np.expand_dims(relative_vel[0], axis=0)
            self.rel_vy = np.expand_dims(relative_vel[1], axis=0)
            self.rel_vz = np.expand_dims(relative_vel[2], axis=0)
        else:
            self.rel_x  = np.append(self.rel_x,  np.expand_dims(relative_loc[0], axis=0), axis=0)
            self.rel_y  = np.append(self.rel_y,  np.expand_dims(relative_loc[1], axis=0), axis=0)
            self.rel_z  = np.append(self.rel_z,  np.expand_dims(relative_loc[2], axis=0), axis=0)
            self.rel_vx = np.append(self.rel_vx, np.expand_dims(relative_vel[0], axis=0), axis=0)
            self.rel_vy = np.append(self.rel_vy, np.expand_dims(relative_vel[1], axis=0), axis=0)
            self.rel_vz = np.append(self.rel_vz, np.expand_dims(relative_vel[2], axis=0), axis=0)

        # # test relative location
        # _, ax_pos = plt.subplots(3,1,figsize=(13,7))
        # yRange = 0.5
        # myPlot.FunctionsPos(ax_pos[0], self.rel_x.transpose(), -1.0, 1.0, -yRange, yRange, title="Position X")
        # myPlot.FunctionsPos(ax_pos[1], self.rel_y.transpose(), -1.0, 1.0, -yRange, yRange, title="Position Y")
        # myPlot.FunctionsPos(ax_pos[2], self.rel_z.transpose(), -1.0, 1.0, -yRange, yRange, title="Position Z")
        # plt.tight_layout()
        # plt.savefig("pos.png")

        # _, ax_vel = plt.subplots(3,1,figsize=(13,7))
        # myPlot.FunctionsVel(ax_vel[0], self.rel_vx.transpose(), 0, self.step_count, -yRange, yRange, title="Velocity X")
        # myPlot.FunctionsVel(ax_vel[1], self.rel_vy.transpose(), 0, self.step_count, -yRange, yRange, title="Velocity Y")
        # myPlot.FunctionsVel(ax_vel[2], self.rel_vz.transpose(), 0, self.step_count, -yRange, yRange, title="Velocity Z")
        # plt.tight_layout()
        # plt.savefig("vel.png")
        # plt.close('all')

        # # test muscle, neuron, motor neuron value
        self.test_m.append(self.muscle_signal_list[0])
        self.test_n.append(self.nv_0)
        # self.test_mn.append(self.mnv_0)
        # plt.figure(figsize=(12,9))
        # plt.subplot(3,1,1)
        # tmp_m = np.array(self.test_m)
        # for i in range(96):
        #     plt.plot(tmp_m[:,i])
        # # plt.ylim(-80, 20)
        # plt.ylim(-0.2,1)
        # # plt.subplot(3,1,2)
        # # tmp_n = np.array(self.test_n)
        # # for i in range(self.worm_0.nseg):
        # #     plt.plot(tmp_n[:,i])
        # plt.subplot(3,1,3)
        # tmp_mn = np.array(self.test_mn)
        # for i in range(len(self.config["output_neuron"])):
        #     plt.plot(tmp_mn[:,i])
        # plt.savefig("test_m_n.png")
        # plt.close('all')

        if self.config.get("export_data", False) and self.step_count == 800:
            pickle.dump({
                "sensory_input": self.test_i,
                "normalization_value": self.test_cn,
                "neural_voltage": self.test_n,
                "muscle_activation": self.test_m,
                "behavior_value": [self.rel_x, self.rel_y, self.rel_z, self.rel_vx, self.rel_vy, self.rel_vz],
                "local_head_location": self.test_local_head_loc,
                "world_head_location": self.test_world_head_loc
            },open("remove_syn.pkl", "wb"))


        for i in range(self.worm_num):
            head_local_location = interact.get_head_location(i)
            head_global_location = interact.worm_local_to_world(i, head_local_location)
            # print(f"step ts:{self.step_count}, worm_{i:d}: Local:[{head_local_location[0]:.4f},{head_local_location[1]:.4f},{head_local_location[2]:.4f}], Global:[{head_global_location[0]:.4f},{head_global_location[1]:.4f},{head_global_location[2]:.4f}]")
        # if self.step_count == 111:
        #     self.save_state(3,111)
        # elif self.step_count == 222:
        #     self.save_state(4,222)
        # elif self.step_count == 158:
        #     self.save_state(5,158)

    def step_with_auto_screen_shot(self):
        """move one step according to the stimulus in the environment"""
        self.step_count += 1
        self.muscle_signal_list = self.control_muscle()
        for i in range(self.worm_num):
            interact.set_muscle_signals(self.muscle_signal_list[i], i)
            interact.set_neuron_voltages_and_screen_shot(eval(f"self.nv_{i:d}"))

    def save_state(self, save_id, step_count):
        """save motion and neural state of one time step"""
        save_id = int(save_id)
        interact.save_worm(save_id, os.path.join(os.path.dirname(__file__), 'output', 'sim', f"mesh_{save_id:d}.json"))
        eval(f"self.worm_{save_id}.save_neuron_muscle({save_id})")
        print(f"save worm_{save_id} at time step {step_count}")
    
    def restore_state(self, restore_id):
        """restore motion and neural state of one time step"""
        restore_id = int(restore_id)
        interact.restore_worm(restore_id, os.path.join(os.path.dirname(__file__), 'output', 'sim', f"mesh_{restore_id:d}.json"))
        eval(f"self.worm_{restore_id:d}.restore_state({restore_id:d})")
        print(f"restore worm_{restore_id:d} state")
        
    def simulate_neural_model_one_time_step(self, delta_concentration_normalized_list):
        ''' calculate motor neuron voltage, 600Hz'''
        # delta_concentration_normalized_list: [x0, x1, ...]
        # run neuron simulation one time step
        if "cnn" in self.config.get("detail_control", "fc"):
            for wormnn, dcn in zip(self.worms, delta_concentration_normalized_list):
                for conn in wormnn.worm.input_connections:
                    conn.hoc_obj.vpre = dcn
            if self.config["proprioception"]:
                # add proprioceptive input
                for wormnn, mss in zip(self.worms, self.muscle_signal_list):
                    for (_, muscle_index, conn) in wormnn.worm.proprioceptive_connections:
                        conn.hoc_obj.vpre = mss[muscle_index] * 80
        elif self.config.get("detail_control", "fc") == "fc":
            for wormnn, dcn in zip(self.worms, delta_concentration_normalized_list):
                for i,_ in enumerate(self.config["input_neuron"]):
                    wormnn.worm.iclamp[i].amp = dcn
        h.fadvance()
        # record soma voltage
        motor_neuron_volt_list = []
        for wormnn in self.worms:
            motor_neuron_volt = np.zeros((len(self.config["output_neuron"]), ), dtype=np.float32)
            for i, cn in enumerate(self.config["output_neuron"]):
                cell = wormnn.worm.cell(cell_name=cn)
                motor_neuron_volt[i] = cell.hoc_obj.Soma(0.5).v
            motor_neuron_volt_list.append(motor_neuron_volt)
        return motor_neuron_volt_list
