import numpy as np
import math
import os
from neuron import h
import torch
import pickle
from eworm.ghost_in_mesh_sim.pre_interaction import CNN, CNN2
from eworm.network.detailed_circuit import DetailedCircuit
from eworm.utils import *
from eworm.network import *


class WormNeuralNetwork(object):
    """class for a worm's nervous system

    Attributes:
        config: dictionary, configuration of simulation
        head_location: array of float32, shape (3,), head location of this worm
        target_location: array of float32, shape (3,), target location (food) of this worm
        concentration: float, the concentration that this worm senses
        delta_concentration_history: array of float32, shape (history_len,), delta concentration history
        worm: DetailedCircuit class, the worm's nervous system
        cnn: CNN, which can transfer motor neuron voltage to muscle signal
        motor_neuron_map: tensor, shape (1, len(output_neuron), history_len), save motor neuron voltage
    """
    def __init__(self, start_location, target_location, config, history_len=11) -> None:
        self.config = config
        self.head_location = start_location
        self.target_location = target_location
        self.concentration = self._get_concentration(self.head_location)
        self.delta_concentration_history = np.zeros((history_len,), dtype=np.float32)
        self.worm = self._define_worm_model()
        if self.config["proprioception"]:
            self.add_proprioceptual_synapse()
        if self.config.get("detail_control", "fc") == "fc":
            self._add_iclamp()
            self.wout = self._load_reservori_wout()
        elif "cnn" in self.config.get("detail_control", "fc"):
            self.cnn, self.motor_neuron_map = self._define_cnn_model()
        self.min_ms = 0
        self.max_ms = 0


    def _define_worm_model(self):
        '''initialize Celegans model and neuron simulation'''
        config_file = self.config["config_file"]
        config = func.load_json(config_file)["config"]
        connection_file = self.config["abs_circuit_file"]
        abs_circuit = pickle.load(open(connection_file, "rb"))
        np.random.seed(1)
        # abstract2detailed(abs_circuit, config)
        # abstract2point(abs_circuit, config)
        # abstract2detailed_shuffle_location(abs_circuit, config)
        # abstract2detailed_shuffle_weight(abs_circuit, config, shuffle_target="syn")
        # abstract2detailed_shuffle_weight(abs_circuit, config, shuffle_target="gj")
        # abstract2detailed_remove(abs_circuit, config, remove_target="syn")
        # abstract2detailed_remove(abs_circuit, config, remove_target="gj")
        circuit = transform.abstract2detailed_remove(abs_circuit, config, remove_target="gj")
        self.neuron_num = len(circuit.cells)
        self.nseg = 0
        for cell in circuit.cells:
            self.nseg += len(cell.segments)
        return circuit
        
    def _add_iclamp(self):
        input_cell_names = self.config["input_neuron"]
        self.worm.iclamp = []
        for cell_name in input_cell_names:
            syn = h.IClamp(self.worm.cell(cell_name=cell_name).segments[0].hoc_obj)
            syn.amp = 0.
            syn.delay = 0.
            syn.dur = 1e9
            self.worm.iclamp.append(syn)
    
    def _load_reservori_wout(self):
        wout_path = self.config["reservoir_wout"]
        wout = pickle.load(open(wout_path, "rb"))
        return wout

    def _define_cnn_model(self):
        '''initialize CNN which convert motor neuron voltage to muscle signal and the motor neuron map'''
        if self.config.get("detail_control", "fc") == "one_cnn":
            cnn = CNN()
            cnn.load_state_dict(torch.load(self.config["cnn_model_file"]))
            motor_neuron_volt = np.zeros([1, len(self.config["output_neuron"]), 21])   # kernel size
            motor_neuron_map = torch.as_tensor(torch.from_numpy(motor_neuron_volt)).to(torch.float32)
        elif self.config.get("detail_control", "fc") == "multi_cnn" and self.config.get("neuron_muscle_matrix", None):
                self.linked_neuron_id = {}
                cnn = []
                motor_neuron_map = []
                neuron_muscle_matrix, _, _ = func.read_excel(file_name=self.config["neuron_muscle_matrix"],
                                                             sheet_name="Sheet3")
                for m_i in range(96):
                    self.linked_neuron_id[m_i] = []
                    for n_i in range(len(self.config["output_neuron"])):
                        if len(str(neuron_muscle_matrix.cell_value(n_i + 1, m_i + 1))) > 0:
                            self.linked_neuron_id[m_i].append(n_i)
                    _cnn = CNN2(in_channel=len(self.linked_neuron_id[m_i]))
                    model_name = self.config["cnn_model_file"].replace(".pt",f"_{m_i}.pt")
                    _cnn.load_state_dict(torch.load(model_name))
                    _motor_neuron_volt = np.zeros([1, len(self.linked_neuron_id[m_i]), 21])   # kernel size
                    cnn.append(_cnn)
                    motor_neuron_map.append(torch.as_tensor(torch.from_numpy(_motor_neuron_volt)).to(torch.float32))
        else:
            raise ValueError
        return cnn, motor_neuron_map


    def _get_concentration(self, location):
        '''calculate atttration concentration according to head location'''
        return np.linalg.norm(self.target_location - location) * -1


    def normalize_delta_concentration(self, concentration_next):
        """normalize network input (i.e. delta concentration)"""
        self.min_ms = np.min(self.delta_concentration_history)
        self.max_ms = np.max(self.delta_concentration_history)
        delta_concentration = concentration_next - self.concentration
        delta_concentration = delta_concentration
        delta_concentration_scale = (delta_concentration - self.min_ms) * 100 / (self.max_ms - self.min_ms) - 80
        if "cnn" in self.config.get("detail_control", "fc"):
            if math.isnan(delta_concentration_scale):
                delta_concentration_scale = -80
            else:
                delta_concentration_scale = np.clip(delta_concentration_scale, -80, 20)
        elif self.config.get("detail_control", "fc") == "fc":
            delta_concentration_scale = delta_concentration_scale / 1000
            if math.isnan(delta_concentration_scale):
                delta_concentration_scale = -0.08
            else:
                # delta_concentration_scale = 1.0/(1+np.exp(-(delta_concentration_scale+0.015)*200))/10-0.08  # do not work
                delta_concentration_scale = np.clip(delta_concentration_scale, -0.08, 0.02)
        self.delta_concentration_history = np.concatenate((self.delta_concentration_history[1:], np.array([delta_concentration])))
        self.concentration = concentration_next
        return delta_concentration, delta_concentration_scale


    def save_neuron_muscle(self, id):
        """save neuron and muscle state to file"""
        nm_state = {}
        nm_state['delta_con_history'] = self.delta_concentration_history
        nm_state['motor_neuron_map'] = self.motor_neuron_map.tolist()
        nm_state['muscle_signal'] = self.muscle_signal
        pickle.dump(nm_state, open(os.path.join(os.path.dirname(__file__), 'output', 'sim', f'neuron_muscle_{id:d}.pkl'), 'wb'))
    

    def restore_state(self, id):
        """restore neuron and muscle state to file"""
        # all locations are local locations
        nm_state = pickle.load(open(os.path.join(os.path.dirname(__file__), 'output', 'sim', f'neuron_muscle_{id:d}.pkl'), 'rb'))
        self.delta_concentration_history = nm_state['delta_con_history']
        self.muscle_signal = nm_state['muscle_signal']
        if self.config.get("detail_control", "fc") == "one_cnn":
            self.motor_neuron_map = torch.Tensor(nm_state['motor_neuron_map'])
        elif self.config.get("detail_control", "fc") == "multi_cnn":
            for m_i in range(96):
                linked_neuron = self.linked_neuron_id[m_i]
                self.motor_neuron_map[m_i] = torch.Tensor(nm_state['motor_neuron_map'])[:,linked_neuron,:]
        elif self.config.get("detail_control", "fc") == "fc":
            self.motor_neuron_map = nm_state['motor_neuron_map']
        else:
            return


    def location_to_network_input(self, head_locationation):
        """tranfer from local head location to network input"""
        # get next head location
        next_head_location = head_locationation
        self.head_location = next_head_location
        # calculate concentration gradient
        concentration_next = self._get_concentration(next_head_location)
        _, delta_concentration_scale = self.normalize_delta_concentration(concentration_next)
        return delta_concentration_scale


    def motor_neuron_volt_to_muscle_sig(self, mnv):
        """tranfer motor neuron voltage to muscle signal"""
        if self.config.get("detail_control", "fc") == "one_cnn":
            motor_neuron_volt = np.zeros((1, len(self.config["output_neuron"]), 1), dtype=np.float32)
            motor_neuron_volt[0,:,0] = mnv
            # calculate muscle signal
            self.motor_neuron_map = torch.cat((
                self.motor_neuron_map[:,:, 1:], 
                torch.as_tensor(torch.from_numpy(motor_neuron_volt)).to(torch.float32)
                ), 2)
            self.muscle_signal = np.clip(np.squeeze((self.cnn(self.motor_neuron_map).data.numpy() + 80)/ 100), 0, 0.8)
        elif self.config.get("detail_control", "fc") == "multi_cnn":
            self.muscle_signal = np.zeros((96,), dtype=np.float32) # xxx
            for m_i in range(96):
                linked_neruon = self.linked_neuron_id[m_i]
                motor_neuron_volt = np.zeros((1, len(linked_neruon), 1), dtype=np.float32)
                motor_neuron_volt[0,:,0] = mnv[linked_neruon]
                self.motor_neuron_map[m_i] = torch.cat((
                    self.motor_neuron_map[m_i][:,:, 1:], 
                    torch.as_tensor(torch.from_numpy(motor_neuron_volt)).to(torch.float32)
                    ), 2)
                self.muscle_signal[m_i] = np.clip(np.squeeze((self.cnn[m_i](self.motor_neuron_map[m_i]).data.numpy() + 80)/ 100), 0, 0.8)
        elif self.config.get("detail_control", "fc") == "fc":
            self.muscle_signal = np.clip((np.squeeze(mnv @ self.wout)+80)/100, 0, 0.8)  #np.squeeze(mnv @ self.wout)
            # print("mnv:", mnv)
            # print("self.wout", self.wout)
            # print("@", mnv @ self.wout)
        return self.muscle_signal
    

    def get_neuron_voltage(self):
        voltage = np.zeros((self.nseg,), dtype=np.float32)
        cnt = 0
        for cell in self.worm.cells:
            for seg in cell.segments:
                voltage[cnt] = seg.hoc_obj.v
                cnt += 1
        return voltage


    def add_proprioceptual_synapse(self):
        """proprioception is the signal from muscle to motor neuron
           before giving motor neuron proprioceptive input, synapse
           should be put on motor neuron
        """
        # load neuron-muscle connection: 
        # muscle seq: DR0-24, VR0-24, DL0-24, VL0-24
        # neuron seq: no seq, you should scan
        config_file = func.load_json(self.config['config_file'])
        matrix_path = config_file['dir_info']['neuron_muscle_con']
        muscle_neuron_matrix, nrow, ncol = func.read_excel(file_name=os.path.join(os.path.dirname(__file__), "..", matrix_path))
        # add proprioceptual synapse
        self.worm.proprioceptive_connections = []
        for j in range(1,ncol):
            cell_name = muscle_neuron_matrix.cell_value(0, j)
            cell = self.worm.cell(cell_name=cell_name)
            for i in range(1, nrow):
                conn = muscle_neuron_matrix.cell_value(i, j)
                if len(str(conn)) != 0:
                    new_connection = detailed_circuit.Connection(None, cell.segment(0), 'syn', 1)
                    self.worm.proprioceptive_connections.append((cell_name, i-1, new_connection))
