from neuron import h
import neuron
import math
import numpy as np
from matplotlib import pyplot as plt

class test_cell(object):
    def __init__(self, channel_name, step_v, gbar, area, volumn, tstop) -> None:
        # channel_name = ['shl1', 'egl2' ...], a list
        # step_v = list(range(stim_start, stim_end+1, stim_step))
        # gbar = [1.8, 2.5 ...]  : in mod (nS/um^2), here area = 1um^2, then gbar*S = 1.8 nS, a list
        # tstop = 100 (ms)
        super().__init__()
        self.channel_name = channel_name
        self.step_v = step_v
        self.gbar = gbar
        self.tstop = tstop
        self.n_v_step = len(self.step_v)
        self.set_run_param()
        self.n_time_step = int(h.tstop / h.dt + 1)
        self.volumn = volumn
        self.area = area
        self.v_list = np.zeros([self.n_v_step, self.n_time_step], dtype=np.float64)
        self.i_list = np.zeros([self.n_v_step, self.n_time_step], dtype=np.float64)
        self.cai_list = np.zeros([self.n_v_step, self.n_time_step], dtype=np.float64)
        self.define_neuron()
        self.define_vector()

    def set_run_param(self):
        h.load_file('stdrun.hoc')
        h.tstop = self.tstop
        h.dt = 0.025
        h.secondorder=0
        h.v_init = -75

    def define_neuron(self):
        self.soma = h.Section(name='soma')
        # area = L * pi * d
        self.soma.L = 1  # (um)
        self.soma.diam = 1  # (um)
        self.soma.cm = 1.3  # (uF/cm2) Membrane capacitance in micro Farads / cm^2
        self.soma.insert('pas')
        for seg in self.soma:
            seg.pas.g = 0.027/self.area  # Passive conductance in S/cm2 
            seg.pas.e = -90    # Leak reversal potential mV


        neuron.load_mechanisms('/home/zhaomengdi/Project/eworm/Celegans_as_deep_net_net/chemotaxis_net/components/mechanism/')
        for cn, gb in zip(self.channel_name, self.gbar):
            self.soma.insert(cn)
            if cn in ['slo1_unc2', 'slo1_egl19', 'slo2_unc2', 'slo2_egl19']:
                self.soma.insert('cainternn')
            if cn in ['kcnl']:
                self.soma.insert('cainternm')
                self.soma.insert('egl19')
                self.soma.insert('unc2')
                self.soma.insert('cca1')
            for seg in self.soma:
                if cn == 'shl1':
                    seg.shl1.gbshl1 = gb/self.area
                elif cn == 'kvs1':
                    seg.kvs1.gbkvs1 = gb/self.area
                elif cn == 'shk1':
                    seg.shk1.gbshk1 = gb/self.area
                elif cn == 'kqt3':
                    seg.kqt3.gbkqt3 = gb/self.area
                elif cn == 'irk':
                    seg.irk.gbirk = gb/self.area
                elif cn == 'egl36':
                    seg.egl36.gbegl36 = gb/self.area
                elif cn == 'egl2':
                    seg.egl2.gbegl2 = gb/self.area
                elif cn == 'nca':
                    seg.nca.gbnca = gb/self.area
                elif cn == 'egl19':
                    seg.egl19.gbegl19 = gb/self.area
                elif cn == 'unc2':
                    seg.unc2.gbunc2 = gb/self.area
                elif cn == 'cca1':
                    seg.cca1.gbcca1 = gb/self.area
                elif cn == 'slo1_unc2':
                    seg.slo1_unc2.gbslo1 = gb/self.area
                    seg.cainternn.gbcav = gb/self.area
                    seg.cainternn.vcell = self.volumn  # (um^3)
                elif cn == 'slo1_egl19':
                    seg.slo1_egl19.gbslo1 = gb/self.area
                    seg.cainternn.gbcav = gb/self.area
                    seg.cainternn.vcell = self.volumn  # (um^3)
                elif cn == 'slo2_unc2':
                    seg.slo2_unc2.gbslo2 = gb/self.area
                    seg.cainternn.gbcav = gb/self.area
                    seg.cainternn.vcell = self.volumn  # (um^3)
                elif cn == 'slo2_egl19':
                    seg.slo2_egl19.gbslo2 = gb/self.area
                    seg.cainternn.gbcav = gb/self.area
                    seg.cainternn.vcell = self.volumn  # (um^3)
                elif cn == 'kcnl':
                    seg.kcnl.gbkcnl = gb/self.area
                    seg.cainternm.vcell = self.volumn  # (um^3) AWC vcell


    def define_stimulus(self, stim_value):
        self.stim = h.IClamp(self.soma(0.5))
        self.stim.delay = stim_value[0]
        self.stim.dur = stim_value[1] - stim_value[0]
        self.stim.amp = stim_value[2] * (10**-3)  # (nA)
        print("stim = ", stim_value)
    
    def define_vector(self):
        self.v_vec = h.Vector()        # Membrane potential vector (mV)
        self.t_vec = h.Vector()        # Time stamp vector (ms)
        self.i_vec = h.Vector()        # potassium/sodium current (mA/cm^2)
        self.cai_vec = h.Vector()      # calsium concentration (uM/um2)
        self.v_vec.record(self.soma(0.5)._ref_v)
        self.t_vec.record(h._ref_t)
        '''
        if self.channel_name in ['nca']:
            self.i_vec.record(self.soma(0.5)._ref_ina)
        elif self.channel_name in ['shl1', 'kvs1', 'shk1', 'kqt3', 'irk', 'egl36', 'egl2', 'slo1_unc2', 'slo1_egl19', 'slo2_unc2', 'slo2_egl19', 'kcnl']:
            self.i_vec.record(self.soma(0.5)._ref_ik)
        elif self.channel_name in ['egl19', 'unc2', 'cca1']:
            self.i_vec.record(self.soma(0.5)._ref_ica)
        if self.channel_name in ['kcnl']:
            self.cai_vec.record(self.soma(0.5)._ref_cai)
        '''

    def run_simulation(self):
        # record voltage and current
        for i in range(self.n_v_step):
            self.define_stimulus(self.step_v[i])
            self.define_vector()
            h.run()
            self.v_list[i,:] = self.v_vec.as_numpy()
            

    def save_voltage_current(self, xticks, yticks, figname):
        # plot voltage and current
        plt.figure(figsize=(5,2.3))
        plt.subplot(1,2,1)
        for i in range(self.n_v_step):
            c = i / (self.n_v_step + 1)
            current = np.zeros_like(self.t_vec.as_numpy())
            current[int(self.step_v[i][0]/h.dt):int(self.step_v[i][1]/h.dt)] = self.step_v[i][2]
            plt.plot(self.t_vec, current, label=str(i), color = (c,c,c))
        plt.xlabel('time (ms)')
        plt.xticks(xticks)
        plt.ylabel('current (pA)')

        plt.subplot(1,2,2)
        for i in range(self.n_v_step):
            c = i / (self.n_v_step + 1)
            plt.plot(self.t_vec, self.v_list[i], label=str(i), color = (c,c,c))
        plt.xlabel('time (ms)')
        plt.xticks(xticks)
        plt.ylabel('mV')
        #plt.yticks(yticks)
        plt.title('voltage clamp')

        plt.savefig(figname+".png")



if __name__ == '__main__':
    
    channel_name_list = ['shl1','shk1','kvs1','egl2','egl36',
                         'kqt3','egl19','unc2','cca1','slo1_egl19',
                         'slo1_unc2','slo2_egl19','slo2_unc2','nca','irk']   
    gbar_list = [2.9, 0.1, 0.8, 0.85, 0,
                 0.55, 1.55, 1, 0.7, 0.11, 
                 0.11, 0.10, 0.10, 0.06, 0.06, 0.25]
    volumn = 31.16 # (um^3)
    area = 238.16  # (um^2)
    step_v = [[100, 600, 0]]
              #[100, 600, 0],
              #[100, 600, 4],
              #[100, 600, 8],
              #[100, 600, 12],
              #[100, 600, 16],
              #[100, 600, 20]]
    
    tc = test_cell(channel_name = channel_name_list,\
                   step_v=step_v,\
                   gbar=gbar_list,\
                   area=area,\
                   volumn=volumn,\
                   tstop=700)

    tc.run_simulation()
    tc.save_voltage_current(np.arange(50,651,150), np.arange(-80,-6,15), "AWC")

    print("test %s done!"%tc.channel_name)

    #tc.advance_simulation(stim_value=-80)
    #tc.save_param()
