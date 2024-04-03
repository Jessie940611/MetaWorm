import os
import neuron
import argparse
import numpy as np
from neuron import h
import os.path as path
import matplotlib.pyplot as plt


class ChannelTester(object):
    """Test dynamics of ion channel model"""
    def __init__(self, channel_name, g):
        super().__init__()
        self.channel_name = channel_name
        self.g = g
        self.soma = self.define_neuron()
        self.v_rec, self.t_rec, self.i_rec, self.cai_rec = self.define_recorder()

    def define_neuron(self):
        """Define neuron's morphology, passive and active properties"""
        soma = h.Section(name='soma')
        # area = L * pi * d
        soma.L = 1  # (um)
        soma.diam = 1 / np.pi  # (um)
        mod_path = path.join(path.dirname(__file__), '..', 'components', 'mechanism')
        neuron.load_mechanisms(mod_path)
        
        soma.insert(self.channel_name)
        if self.channel_name in ['kcnl']:
            soma.insert('cainternm')
            soma.insert('egl19')
            soma.insert('unc2')
            soma.insert('cca1')
        for seg in soma:
            if self.channel_name == 'shl1':
                seg.shl1.gbshl1 = self.g
            elif self.channel_name == 'kvs1':
                seg.kvs1.gbkvs1 = self.g
            elif self.channel_name == 'shk1':
                seg.shk1.gbshk1 = self.g
            elif self.channel_name == 'kqt3':
                seg.kqt3.gbkqt3 = self.g
            elif self.channel_name == 'irk':
                seg.irk.gbirk = self.g
            elif self.channel_name == 'egl36':
                seg.egl36.gbegl36 = self.g
            elif self.channel_name == 'egl2':
                seg.egl2.gbegl2 = self.g
            elif self.channel_name == 'nca':
                seg.nca.gbnca = self.g
            elif self.channel_name == 'egl19':
                seg.egl19.gbegl19 = self.g
            elif self.channel_name == 'unc2':
                seg.unc2.gbunc2 = self.g
            elif self.channel_name == 'cca1':
                seg.cca1.gbcca1 = self.g
            elif self.channel_name == 'slo1_unc2':
                seg.slo1_unc2.gbslo1 = self.g
            elif self.channel_name == 'slo1_egl19':
                seg.slo1_egl19.gbslo1 = self.g
            elif self.channel_name == 'slo2_unc2':
                seg.slo2_unc2.gbslo2 = self.g
            elif self.channel_name == 'slo2_egl19':
                seg.slo2_egl19.gbslo2 = self.g
            elif self.channel_name == 'kcnl':
                seg.kcnl.gbkcnl = self.g
                seg.egl19.gbegl19 = 1  # (nS)
                seg.unc2.gbunc2 = 1  # (nS)
                seg.cca1.gbcca1 = 1  # (nS)
                seg.cainternm.vcell = 31.16  # (um^3) AWC vcell
        return soma

    def define_recorder(self):
        """Define recorder of neural voltage, current and time"""
        v_rec, t_rec, i_rec, cai_rec = h.Vector(), h.Vector(), h.Vector(), h.Vector()
        v_rec.record(self.soma(0.5)._ref_v)
        t_rec.record(h._ref_t)
        if self.channel_name in ['nca']:
            i_rec.record(self.soma(0.5)._ref_ina)
        elif self.channel_name in ['shl1', 'kvs1', 'shk1', 'kqt3', 'irk', 'egl36', 'egl2',
                                   'slo1_unc2', 'slo1_egl19', 'slo2_unc2', 'slo2_egl19', 'kcnl']:
            i_rec.record(self.soma(0.5)._ref_ik)
        elif self.channel_name in ['egl19', 'unc2', 'cca1']:
            i_rec.record(self.soma(0.5)._ref_ica)
        if self.channel_name in ['kcnl']:
            cai_rec.record(self.soma(0.5)._ref_cai)
        return v_rec, t_rec, i_rec, cai_rec

    def run_simulation(self, sim_protocol, save_dir):
        """Simulate model and visualize it"""
        sim_time, step_values = sim_protocol['sim_time'], sim_protocol['step_values']
        h.load_file('stdrun.hoc')
        h.tstop = np.sum(sim_time)
        h.dt = 0.025
        h.secondorder = 0
        h.v_init = -80
        n_time_step = int(h.tstop/h.dt + 1)
        v_list = np.zeros([len(step_values), n_time_step])
        i_list = np.zeros([len(step_values), n_time_step])
        cai_list = np.zeros([len(step_values), n_time_step])
        for step_index, step_value in enumerate(step_values):
            stimulus = h.VClamp(self.soma(0.5))
            stimulus.dur[0] = sim_time[0]
            stimulus.amp[0] = -80
            stimulus.dur[1] = sim_time[1]
            stimulus.amp[1] = step_value
            stimulus.dur[2] = sim_time[2]
            stimulus.amp[2] = -80
            h.finitialize(-80)
            h.run()
            v_list[step_index] = self.v_rec.as_numpy()
            i_list[step_index] = self.i_rec.as_numpy()
            if self.channel_name in ['kcnl']:
                cai_list[step_index] = self.cai_rec.as_numpy()
        # Visualization
        plt.figure(figsize=(5, 2.3))
        plt.subplot(1, 2, 1)
        for step_index in range(len(step_values)):
            c_l = step_index / (len(step_values) + 1)
            plt.plot(self.t_rec.as_numpy(), v_list[step_index],
                     alpha=0.8, label=str(step_index), color=(c_l, c_l, c_l))
        plt.xlabel('Time (ms)')
        plt.xticks(sim_protocol['xticks'])
        plt.ylabel('Voltage (mV)')
        plt.title('Voltage Clamp')
        plt.subplot(1, 2, 2)
        for step_index in range(len(step_values)):
            c_l = step_index / (len(step_values) + 1)
            plt.plot(self.t_rec.as_numpy(), i_list[step_index],
                     alpha=0.8, label=str(step_index), color=(c_l, c_l, c_l))
        plt.xlabel('Time (ms)')
        plt.ylabel('Current (pA)')
        plt.title(self.channel_name.upper())
        plt.xticks(sim_protocol['xticks'])
        plt.yticks(sim_protocol['yticks'])
        plt.tight_layout()
        plt.savefig(path.join(save_dir, f'{self.channel_name}.png'),
                    dpi=300, bbox_inches='tight')
        if self.channel_name in ['kcnl']:
            plt.figure(figsize=(5, 2.3))
            plt.subplot(1, 2, 1)
            for step_index in range(len(step_values)):
                c_l = step_index / (len(step_values) + 1)
                plt.plot(self.t_rec.as_numpy(), v_list[step_index],
                         alpha=0.8, label=str(step_index), color=(c_l, c_l, c_l))
            plt.xlabel('Time (ms)')
            plt.xticks(sim_protocol['xticks'])
            plt.ylabel('Voltage (mV)')
            plt.title('Voltage Clamp')
            plt.subplot(1, 2, 2)
            for step_index in range(len(step_values)):
                c_l = step_index / (len(step_values) + 1)
                plt.plot(self.t_rec.as_numpy(), cai_list[step_index],
                         alpha=0.8, label=str(step_index), color=(c_l, c_l, c_l))
            plt.xlabel('Time (ms)')
            plt.ylabel('[Ca2+] (uM/um2)')
            plt.title(self.channel_name.upper())
            plt.xticks(sim_protocol['xticks'])
            plt.tight_layout()
            plt.savefig(path.join(save_dir, f'{self.channel_name}_cai.png'),
                        dpi=300, bbox_inches='tight')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--channel_id', type=int, default=0, help='Channel Index to test')
    parser.add_argument('--save_name', default='channel test result', help='result recording name')
    args = parser.parse_args()
    channel_indices = ('shl1', 'kvs1', 'shk1', 'kqt3', 'irk',
                       'egl36', 'egl2', 'nca', 'egl19', 'unc2',
                       'cca1', 'slo1_unc2', 'slo1_egl19', 'slo2_unc2', 'slo2_egl19',
                       'kcnl')
    sim_protocol_database = {
        'shl1': {'sim_time': (100, 600, 100),
                 'step_values': np.linspace(start=-120, stop=40, num=17),
                 'g': 3., 'xticks': [0, 300, 600], 'yticks': [0, 40, 80, 120],
                 },
        'kvs1': {'sim_time': (100, 600, 100),
                 'step_values': np.linspace(start=-120, stop=40, num=17),
                 'g': 3, 'xticks': [0, 300, 600], 'yticks': [0, 40, 80, 120],
                 },
        'shk1': {'sim_time': (100, 600, 100),
                 'step_values': np.linspace(start=-120, stop=40, num=17),
                 'g': 1.1, 'xticks': [0, 300, 600], 'yticks': [0, 40, 80, 120],
                 },
        'kqt3': {'sim_time': (200, 6000, 200),
                 'step_values': np.linspace(start=-120, stop=40, num=17),
                 'g': 15., 'xticks': [0, 3000, 6000], 'yticks': [0, 400, 800],
                 },
        'irk': {'sim_time': (100, 600, 100),
                'step_values': np.linspace(start=-120, stop=40, num=17),
                'g': 10, 'xticks': [0, 300, 600], 'yticks': [-400, -200, 0, 200],
                },
        'egl36': {'sim_time': (200, 6000, 200),
                  'step_values': np.linspace(start=-120, stop=40, num=17),
                  'g': 50, 'xticks': [0, 3000, 6000],
                  'yticks': [0, 500, 1000, 1500, 2000, 2500],
                  },
        'egl2': {'sim_time': (200, 6000, 200),
                 'step_values': np.linspace(start=-120, stop=40, num=17),
                 'g': 20, 'xticks': [0, 3000, 6000],
                 'yticks': [0, 500, 1000, 1500, 2000, 2500],
                 },
        'nca': {'sim_time': (100, 600, 100),
                'step_values': np.linspace(start=-120, stop=40, num=17),
                'g': 2, 'xticks': [0, 300, 600], 'yticks': [0, -200, -400],
                },
        'egl19': {'sim_time': (100, 200, 100),
                  'step_values': np.linspace(start=-80, stop=40, num=13),
                  'g': 200, 'xticks': [0, 100, 300], 'yticks': [0, -7500, -15000, -22500, -30000],
                  },
        'unc2': {'sim_time': (100, 200, 100),
                 'step_values': np.linspace(start=-80, stop=40, num=13),
                 'g': 23, 'xticks': [0, 100, 300], 'yticks': [0, -500, -1000, -1500],
                 },
        'cca1': {'sim_time': (100, 200, 100),
                 'step_values': np.linspace(start=-80, stop=40, num=13),
                 'g': 25, 'xticks': [0, 100, 300], 'yticks': [0, -500, -1000, -1500],
                 },
        'slo1_unc2': {'sim_time': (20, 660, 20),
                      'step_values': np.linspace(start=-80, stop=40, num=13),
                      'g': 15, 'xticks': [0, 200, 400, 600], 'yticks': [0, 200, 400],
                      },
        'slo1_egl19': {'sim_time': (20, 660, 20),
                       'step_values': np.linspace(start=-80, stop=40, num=13),
                       'g': 15, 'xticks': [0, 200, 400, 600], 'yticks': [0, 200, 400],
                       },
        'slo2_unc2': {'sim_time': (20, 660, 20),
                      'step_values': np.linspace(start=-80, stop=40, num=13),
                      'g': 15, 'xticks': [0, 200, 400, 600], 'yticks': [0, 100, 200],
                      },
        'slo2_egl19': {'sim_time': (20, 660, 20),
                       'step_values': np.linspace(start=-80, stop=40, num=13),
                       'g': 15, 'xticks': [0, 200, 400, 600], 'yticks': [0, 100, 200],
                       },
        'kcnl': {'sim_time': (20, 660, 20),
                 'step_values': np.linspace(start=-80, stop=40, num=13),
                 'g': 1.5, 'xticks': [0, 200, 400, 600], 'yticks': [0, 20, 40, 60, 80],
                 }
    }
    work_directory = path.join(path.dirname(__file__), args.save_name)
    os.makedirs(work_directory, exist_ok=True)
    channel_name = channel_indices[args.channel_id]
    ct = ChannelTester(channel_name, sim_protocol_database[channel_name]['g'])
    ct.run_simulation(sim_protocol_database[channel_name], work_directory)
