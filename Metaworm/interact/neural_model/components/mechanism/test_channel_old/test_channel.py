from neuron import h
import neuron
import math
import numpy as np
from matplotlib import pyplot as plt

class test_channel(object):
    def __init__(self, channel_name, step_v, gbar, stim_time) -> None:
        # channel_name = 'shl1'
        # step_v = list(range(stim_start, stim_end+1, stim_step))
        # gbar = 1.8  : in mod (nS/um^2), here area = 1um^2, then gbar*S = 1.8 nS
        # stim_time = np.array([100, 600, 100]) (ms)
        super().__init__()
        self.channel_name = channel_name
        self.step_v = step_v
        self.gbar = gbar
        self.stim_time = stim_time
        self.n_v_step = len(self.step_v)
        self.set_run_param()
        self.n_time_step = int(h.tstop / h.dt + 1)
        self.v_list = np.zeros([self.n_v_step, self.n_time_step], dtype=np.float64)
        self.i_list = np.zeros([self.n_v_step, self.n_time_step], dtype=np.float64)
        self.cai_list = np.zeros([self.n_v_step, self.n_time_step], dtype=np.float64)
        self.define_neuron()
        self.define_vector()

    def set_run_param(self):
        h.load_file('stdrun.hoc')
        h.tstop = np.sum(self.stim_time)
        h.dt = 0.025
        h.secondorder=0
        h.v_init = -80

    def define_neuron(self):
        self.soma = h.Section(name='soma')
        # area = L * pi * d
        self.soma.L = 1  # (um)
        self.soma.diam = 1 / math.pi  # (um)
        neuron.load_mechanisms('/home/zhaomengdi/Project/eworm/Celegans_as_deep_net_net/chemotaxis_net/components/mechanism/')
        self.soma.insert(self.channel_name)
        if self.channel_name in ['slo1_unc2', 'slo1_egl19', 'slo2_unc2', 'slo2_egl19']:
            self.soma.insert('cainternn')
        if self.channel_name in ['kcnl']:
            self.soma.insert('cainternm')
            self.soma.insert('egl19')
            self.soma.insert('unc2')
            self.soma.insert('cca1')
        for seg in self.soma:
            if self.channel_name == 'shl1':
                seg.shl1.gbshl1 = self.gbar
            elif self.channel_name == 'kvs1':
                seg.kvs1.gbkvs1 = self.gbar
            elif self.channel_name == 'shk1':
                seg.shk1.gbshk1 = self.gbar
            elif self.channel_name == 'kqt3':
                seg.kqt3.gbkqt3 = self.gbar
            elif self.channel_name == 'irk':
                seg.irk.gbirk = self.gbar
            elif self.channel_name == 'egl36':
                seg.egl36.gbegl36 = self.gbar
            elif self.channel_name == 'egl2':
                seg.egl2.gbegl2 = self.gbar
            elif self.channel_name == 'nca':
                seg.nca.gbnca = self.gbar
            elif self.channel_name == 'egl19':
                seg.egl19.gbegl19 = self.gbar
            elif self.channel_name == 'unc2':
                seg.unc2.gbunc2 = self.gbar
            elif self.channel_name == 'cca1':
                seg.cca1.gbcca1 = self.gbar
            elif self.channel_name == 'slo1_unc2':
                seg.slo1_unc2.gbslo1 = self.gbar
                seg.cainternn.gbcav = self.gbar
                seg.cainternn.vcell = 31.16  # (um^3) AWC vcell
            elif self.channel_name == 'slo1_egl19':
                seg.slo1_egl19.gbslo1 = self.gbar
                seg.cainternn.gbcav = self.gbar
                seg.cainternn.vcell = 31.16  # (um^3) AWC vcell
            elif self.channel_name == 'slo2_unc2':
                seg.slo2_unc2.gbslo2 = self.gbar
                seg.cainternn.gbcav = self.gbar
                seg.cainternn.vcell = 31.16  # (um^3) AWC vcell
            elif self.channel_name == 'slo2_egl19':
                seg.slo2_egl19.gbslo2 = self.gbar
                seg.cainternn.gbcav = self.gbar
                seg.cainternn.vcell = 31.16  # (um^3) AWC vcell
            elif self.channel_name == 'kcnl':
                seg.kcnl.gbkcnl = self.gbar
                seg.egl19.gbegl19 = 1  # (nS)
                seg.unc2.gbunc2 = 1  # (nS)
                seg.cca1.gbcca1 = 1  # (nS)
                seg.cainternm.vcell = 31.16  # (um^3) AWC vcell



    def define_stimulus(self, stim_value):
        self.stim = h.VClamp(self.soma(0.5))
        self.stim.dur[0] = self.stim_time[0]
        self.stim.amp[0] = -80
        self.stim.dur[1] = self.stim_time[1]
        self.stim.amp[1] = stim_value
        self.stim.dur[2] = self.stim_time[2]
        self.stim.amp[2] = -80
        print("stim = ", stim_value)
    
    def define_vector(self):
        self.v_vec = h.Vector()        # Membrane potential vector (mV)
        self.t_vec = h.Vector()        # Time stamp vector (ms)
        self.i_vec = h.Vector()        # potassium/sodium current (mA/cm^2)
        self.cai_vec = h.Vector()      # calsium concentration (uM/um2)
        self.v_vec.record(self.soma(0.5)._ref_v)
        self.t_vec.record(h._ref_t)
        if self.channel_name in ['nca']:
            self.i_vec.record(self.soma(0.5)._ref_ina)
        elif self.channel_name in ['shl1', 'kvs1', 'shk1', 'kqt3', 'irk', 'egl36', 'egl2', 'slo1_unc2', 'slo1_egl19', 'slo2_unc2', 'slo2_egl19', 'kcnl']:
            self.i_vec.record(self.soma(0.5)._ref_ik)
        elif self.channel_name in ['egl19', 'unc2', 'cca1']:
            self.i_vec.record(self.soma(0.5)._ref_ica)
        if self.channel_name in ['kcnl']:
            self.cai_vec.record(self.soma(0.5)._ref_cai)

    def run_simulation(self):
        # record voltage and current
        for i in range(self.n_v_step):
            self.define_stimulus(self.step_v[i])
            self.define_vector()
            h.run()
            self.v_list[i,:] = self.v_vec.as_numpy()
            self.i_list[i,:] = self.i_vec.as_numpy()
            if self.channel_name in ['kcnl']:
                self.cai_list[i,:] = self.cai_vec.as_numpy()

    def save_voltage_current(self, xticks, yticks):
        # plot voltage and current
        plt.figure(figsize=(5,2.3))

        plt.subplot(1,2,1)
        for i in range(self.n_v_step):
            c = i / (self.n_v_step + 1)
            plt.plot(self.t_vec, self.v_list[i], label=str(i), color = (c,c,c))
        plt.xlabel('time (ms)')
        plt.xticks(xticks)
        plt.ylabel('mV')
        plt.title('voltage clamp')

        plt.subplot(1,2,2)
        for i in range(self.n_v_step):
            c = i / (self.n_v_step + 1)
            plt.plot(self.t_vec, np.double(self.i_list[i]), color = (c,c,c)) # (pA)
        plt.xlabel('time (ms)')
        plt.ylabel('current (pA)')
        plt.title(self.channel_name.upper())
        plt.xticks(xticks)
        plt.yticks(yticks)
        plt.tight_layout()
        plt.savefig('test_%s.png'%self.channel_name, dpi=3000, bbox_inches = 'tight')

        if self.channel_name in ['kcnl']:
            print(self.cai_list)
            plt.figure(figsize=(5,2.3))

            plt.subplot(1,2,1)
            for i in range(self.n_v_step):
                c = i / (self.n_v_step + 1)
                plt.plot(self.t_vec, self.v_list[i], label=str(i), color = (c,c,c))
            plt.xlabel('time (ms)')
            plt.xticks(xticks)
            plt.ylabel('mV')
            plt.title('voltage clamp')

            plt.subplot(1,2,2)
            for i in range(self.n_v_step):
                c = i / (self.n_v_step + 1)
                plt.plot(self.t_vec, np.double(self.cai_list[i]), color = (c,c,c)) # (uM/um2)
            plt.xlabel('time (ms)')
            plt.ylabel('[Ca2+] (uM/um2)')
            plt.title(self.channel_name.upper())
            plt.xticks(xticks)
            plt.tight_layout()
            plt.savefig('test_%s_cai.png'%self.channel_name, dpi=3000, bbox_inches = 'tight')


    def advance_simulation(self, stim_value):
        # record params in ion channel
        self.define_stimulus(stim_value)
        '''
        self.minf = np.zeros(self.n_time_step)
        self.hfinf = np.zeros(self.n_time_step)
        self.hsinf = np.zeros(self.n_time_step)
        self.tm = np.zeros(self.n_time_step)
        self.thf = np.zeros(self.n_time_step)
        self.ths = np.zeros(self.n_time_step)
        '''
        self.cai = np.zeros(self.n_time_step)
        #self.hcav = np.zeros(self.n_time_step)
        cnt = 0
        while (h.t < h.tstop):
            h.fadvance()
            '''
            self.minf[cnt] = self.soma.minf_shl1
            self.hfinf[cnt] = self.soma.hfinf_shl1
            self.hsinf[cnt] = self.soma.hsinf_shl1
            self.tm[cnt] = self.soma.tm_shl1
            self.thf[cnt] = self.soma.thf_shl1
            self.ths[cnt] = self.soma.ths_shl1
            '''
            self.cai[cnt] = self.soma.cai
            #self.hcav[cnt] = self.soma.hcav_slo1
            cnt += 1
        print(cnt)

    def save_param(self):
        # plot params in ion channel
        '''
        plt.figure(figsize=(4,8))
        plt.plot(self.minf)
        plt.plot(self.hfinf)
        plt.plot(self.hsinf)
        plt.savefig('test_%s_inf.png'%self.channel_name, bbox_inches = 'tight')
        plt.figure(figsize=(8,4))
        plt.plot(self.tm)
        plt.plot(self.thf)
        plt.plot(self.ths)
        plt.savefig('test_%s_t.png'%self.channel_name, bbox_inches = 'tight')
        '''
        plt.figure(figsize=(8,4))
        plt.plot(self.cai)
        #plt.plot(self.hcav)
        plt.savefig('test_%s_s.png'%self.channel_name, bbox_inches = 'tight')



if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('channel_id', type=int, default=0, help='choose which channel to test')
    args = parser.parse_args()

    channel_id = args.channel_id
    print(channel_id)

    channel_name_list = {0: 'shl1', 
                         1: 'kvs1',
                         2: 'shk1',
                         3: 'kqt3',
                         4: 'irk',
                         5: 'egl36',
                         6: 'egl2',
                         7: 'nca',
                         8: 'egl19',
                         9: 'unc2',
                         10:'cca1',
                         11:'slo1_unc2',
                         12:'slo1_egl19',
                         13:'slo2_unc2',
                         14:'slo2_egl19',
                         15:'kcnl'}
    stim_v_start_list = [-120, -120, -120, -120, -120,\
                         -120, -120, -120, -80, -80,\
                         -80, -80, -80, -80, -80,\
                         -80]
    stim_v_end_list = [40, 40, 40, 40, 40,\
                       40, 40, 40, 40, 40,\
                       40, 40, 40, 40, 40,\
                       40]
    stim_v_step_list = [10, 10, 10, 10, 10,\
                        10, 10, 10, 10, 10,\
                        10, 10, 10, 10, 10,\
                        10]
    gbar_list = [1.8, 3, 1.1, 7.3, 10,\
                 50, 20, 2, 200, 23,\
                 25, 15, 15, 15, 15,\
                 1.5]

    stim_time1 = np.array([100, 600, 100], dtype=int)
    stim_time2 = np.array([200, 6000, 200], dtype=int)
    stim_time3 = np.array([100, 200, 100], dtype=int)
    stim_time4 = np.array([20, 660, 20], dtype=int)
    stim_time_list = [stim_time1, stim_time1, stim_time1, stim_time2, stim_time1,\
                      stim_time2, stim_time2, stim_time1, stim_time3, stim_time3,\
                      stim_time3, stim_time4, stim_time4, stim_time4, stim_time4,\
                      stim_time4]

    xtick1 = [0, 300, 600]
    xtick2 = [0, 3000, 6000]
    xtick3 = [0, 100, 300]
    xtick4 = [0, 200, 400, 600]

    ytick1 = [0, 40, 80, 120]
    ytick2 = [0, 400, 800]
    ytick3 = [-400, -200, 0, 200]
    ytick4 = [0, 500, 1000, 1500, 2000]
    ytick5 = [-400, -200, 0]
    ytick6 = [-22.5*(10**3), -7.5*(10**3), 0]
    ytick7 = [-1500, -1000, -500, 0]
    ytick8 = [0, 200, 400]
    ytick9 = [0, 100, 200]
    ytick10 = [0, 20, 40, 60, 80]

    xtick_list = [xtick1, xtick1, xtick1, xtick2, xtick1,\
                  xtick2, xtick2, xtick1, xtick3, xtick3,\
                  xtick3, xtick4, xtick4, xtick4, xtick4,\
                  xtick4]
    ytick_list = [ytick1, ytick1, ytick1, ytick2, ytick3,\
                  ytick4, ytick4, ytick6, ytick6, ytick7,\
                  ytick7, ytick8, ytick8, ytick9, ytick9,\
                  ytick10]
    
    step_v = list(range(stim_v_start_list[channel_id],\
                        stim_v_end_list[channel_id] + 1,\
                        stim_v_step_list[channel_id]))
    
    tc = test_channel(channel_name = channel_name_list[channel_id],\
                 step_v=step_v,\
                 gbar=gbar_list[channel_id],\
                 stim_time=stim_time_list[channel_id])

    tc.run_simulation()
    tc.save_voltage_current(xtick_list[channel_id], ytick_list[channel_id])

    print("test %s done!"%tc.channel_name)

    #tc.advance_simulation(stim_value=-80)
    #tc.save_param()
