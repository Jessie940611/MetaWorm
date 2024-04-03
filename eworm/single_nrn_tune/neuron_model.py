from neuron import h
import numpy as np


elements_name = ("minf_cca1", "hinf_cca1", "tm_cca1", "th_cca1", "m_cca1", "h_cca1",
                 "minf_egl2", "tm_egl2", "m_egl2",
                 "minf_egl19", "hinf_egl19", "tm_egl19", "th_egl19", "m_egl19", "h_egl19",
                 "mfinf_egl36", "mminf_egl36", "msinf_egl36", "tmf_egl36",
                 "tmm_egl36", "tms_egl36", "mf_egl36", "mm_egl36", "ms_egl36",
                 "minf_irk", "tm_irk", "m_irk",
                 "minf_kqt3", "tmf_kqt3", "tms_kqt3", "winf_kqt3",
                 "sinf_kqt3", "tw_kqt3", "ts_kqt3", "mf_kqt3", "ms_kqt3", "w_kqt3", "s_kqt3",
                 "minf_kvs1", "hinf_kvs1", "tm_kvs1", "th_kvs1", "m_kvs1", "h_kvs1",
                 "minf_shk1", "hinf_shk1", "tm_shk1", "th_shk1", "m_shk1", "h_shk1",
                 "minf_shl1", "hfinf_shl1", "hsinf_shl1", "tm_shl1", "thf_shl1",
                 "ths_shl1", "m_shl1", "hf_shl1", "hs_shl1",
                 "minf_slo1_egl19", "tm_slo1_egl19", "mcavinf_slo1_egl19",
                 "tmcav_slo1_egl19", "hcavinf_slo1_egl19", "thcav_slo1_egl19",
                 "m_slo1_egl19", "hcav_slo1_egl19", "mcav_slo1_egl19",
                 "minf_slo1_unc2", "tm_slo1_unc2", "mcavinf_slo1_unc2",
                 "tmcav_slo1_unc2", "hcavinf_slo1_unc2", "thcav_slo1_unc2",
                 "m_slo1_unc2", "hcav_slo1_unc2", "mcav_slo1_unc2",
                 "minf_slo2_egl19", "tm_slo2_egl19", "mcavinf_slo2_egl19",
                 "tmcav_slo2_egl19", "hcavinf_slo2_egl19", "thcav_slo2_egl19",
                 "m_slo2_egl19", "hcav_slo2_egl19", "mcav_slo2_egl19",
                 "minf_slo2_unc2", "tm_slo2_unc2", "mcavinf_slo2_unc2",
                 "tmcav_slo2_unc2", "hcavinf_slo2_unc2", "thcav_slo2_unc2",
                 "m_slo2_unc2", "hcav_slo2_unc2", "mcav_slo2_unc2",
                 "minf_unc2", "hinf_unc2", "tm_unc2", "th_unc2", "m_unc2", "h_unc2", )


class WormNeuron(object):
    def __init__(self, cell_name, params):
        self.cell = None
        self.t_vec = None
        self.output_record = None
        self.elements = {}
        self.ion = {}
        self.param_record = None
        self.prefix_rest = 5000
        self.create_cells(cell_name)
        self.define_biophysics(params)
        self.add_records()

    def create_cells(self, cell_name):
        self.cell = getattr(h, cell_name)()

    def calc_soma_volume(self):
        R0 = self.cell.Soma.diam3d(0)
        R1 = self.cell.Soma.diam3d(1)
        A0 = np.pi * (R0 ** 2) / 4
        A1 = np.pi * (R1 ** 2) / 4
        L = self.cell.Soma.L
        soma_volume = L * (A0 + A1 + np.sqrt(A0 * A1)) / 3
        return soma_volume

    def define_biophysics(self, params):
        """
        params: 0-4:    Ra,             cm,             gpas,       epas,           gbshl1,
                5-9:    gbshk1,         gbkvs1,         gbegl2,     gbegl36,        gbkqt3,
                10-14:  gbegl19,        gbunc2,         gbcca1,     gbslo1_egl19,   gbslo1_unc2,
                15-18:  gbslo2_egl19,   gbslo2_unc2,    gbnca,      gbirk
                Delete: gbkcnl,
        """

        mech_list = ['slo1_unc2', 'egl2', 'shl1', 'kqt3', 'cainternn', 'unc2', 'kvs1', 'slo1_egl19',
                     'slo2_unc2', 'irk', 'egl36', 'egl19', 'cca1', 'shk1', 'slo2_egl19', 'nca']
        param_list = ['Ra', 'cm', 'gpas', 'epas', 'gbshl1', 'gbshk1', 'gbkvs1', 'gbegl2', 'gbegl36', 'gbkqt3',
                      'gbegl19', 'gbunc2', 'gbcca1', 'gbslo1_egl19', 'gbslo1_unc2', 'gbslo2_egl19', 'gbslo2_unc2',
                      'gbnca', 'gbirk']

        # set biophysical mechanism for all section and segment
        for sec in self.cell.all:
            sec.Ra = params[0]  # (Ohm*cm)
            sec.cm = params[1]  # (uF/cm2)
            sec.insert('pas')
            for seg in sec:
                seg.pas.g = params[2]  # Passive conductance in S/cm2
                seg.pas.e = params[3]  # Leak reversal potential mV

        # set biophysical mechanism for soma
        for m in mech_list:
            self.cell.Soma.insert(m)
        self.cell.Soma.insert('cainternn')
        # self.cell.Soma.insert('cainternm')
        for seg in self.cell.Soma:
            seg.shl1.gbshl1 = params[4]  # (nS/um2)
            seg.shk1.gbshk1 = params[5]
            seg.kvs1.gbkvs1 = params[6]
            seg.egl2.gbegl2 = params[7]
            seg.egl36.gbegl36 = params[8]
            seg.kqt3.gbkqt3 = params[9]
            seg.egl19.gbegl19 = params[10]
            seg.unc2.gbunc2 = params[11]
            seg.cca1.gbcca1 = params[12]
            seg.slo1_egl19.gbslo1 = params[13]
            seg.slo1_unc2.gbslo1 = params[14]
            seg.slo2_egl19.gbslo2 = params[15]
            seg.slo2_unc2.gbslo2 = params[16]
            # seg.kcnl.gbkcnl = params[17]
            seg.nca.gbnca = params[17]
            seg.irk.gbirk = params[18]
            seg.cainternn.gbcav = np.sum(params[13:17])
            seg.cainternn.vcell = self.calc_soma_volume()  # (um^3)
            # seg.cainternm.vcell = self.calc_soma_volume()  # (um^3)

        self.param_record = {}
        for idx_, param_ in enumerate(param_list):
            self.param_record[param_] = params[idx_]
            
    def add_records(self):
        self.t_vec = h.Vector().record(h._ref_t)

    def add_constant_current_stimulus(self, start_time, end_time, amplitude):
        """
        time unit (ms)
        amplitude (nA)
        """
        self.stim = h.IClamp(self.cell.Soma(0.5))
        self.stim.delay = start_time + self.prefix_rest
        self.stim.dur = end_time - start_time
        self.stim.amp = amplitude * 0.001  # (pA)*0.001=(nA)

        self.output_record = h.Vector().record(self.cell.Soma(0.5)._ref_v)

    def add_constant_voltage_stimulus(self, start_time, end_time, tstop, amplitude):
        """
        time unit (ms)
        amplitude (mV)
        """
        self.stim = h.SEClamp(self.cell.Soma(0.5))
        self.stim.dur1 = start_time + self.prefix_rest
        self.stim.dur2 = end_time - start_time
        self.stim.dur3 = tstop - end_time
        self.stim.amp1 = -60  # (mV)
        self.stim.amp2 = amplitude  # (mV)
        self.stim.amp3 = -60  # (mV)
        self.stim.rs = 20  # (MOhm)

        self.output_record = h.Vector().record(self.stim._ref_i)

    def run(self, dt, tstop, v_init, ions=True):
        h.dt = dt
        h.tstop = tstop+self.prefix_rest
        h.secondorder = 0
        h.finitialize(v_init)
        self.elements = {}
        if ions:
            for element_ in elements_name:
                self.elements[element_] = np.zeros(int(tstop/dt+1))

        cnt = 0
        while h.t <= h.tstop:
            h.fadvance()
            if ions and h.t >= self.prefix_rest:
                for element_ in elements_name:
                    self.elements[element_][cnt] = getattr(self.cell.Soma, element_)
                cnt += 1
        
        output_volt = self.output_record.as_numpy()[-int(tstop/dt+1):]
        if ions:
            for element_ in elements_name:
                self.elements[element_] = np.array(self.elements[element_])
            self.ion = {
                'ica_cca1': self.param_record['gbcca1'] * (self.elements['m_cca1'] ** 2) * self.elements['h_cca1'] * (
                            output_volt - 60),
                'ik_egl2': self.param_record['gbegl2'] * self.elements['m_egl2'] * (output_volt + 80),
                'ica_egl19': self.param_record['gbegl19'] * self.elements['m_egl19'] * self.elements['h_egl19'] * (
                            output_volt - 60), 'ik_egl36': self.param_record['gbegl36'] * (
                            0.33 * self.elements['mf_egl36'] + 0.36 * self.elements['mm_egl36'] + 0.39 * self.elements[
                        'ms_egl36']) * (output_volt + 80),
                'ik_irk': self.param_record['gbirk'] * self.elements['m_irk'] * (output_volt + 80),
                'ik_kqt3': self.param_record['gbkqt3'] * (
                            0.3 * self.elements['mf_kqt3'] + 0.7 * self.elements['ms_kqt3']) * (output_volt + 80),
                'ik_kvs1': self.param_record['gbkvs1'] * self.elements['m_kvs1'] * self.elements['h_kvs1'] * (
                            output_volt + 80), 'ina_nca': self.param_record['gbnca'] * (output_volt - 30),
                'ik_shk1': self.param_record['gbshk1'] * self.elements['m_shk1'] * self.elements['h_shk1'] * (
                            output_volt + 80),
                'ik_shl1': self.param_record['gbshl1'] * (self.elements['m_shl1'] ** 3) * (
                            0.7 * self.elements['hf_shl1'] + 0.3 * self.elements['hs_shl1']) * (output_volt + 80),
                'ik_slo1_egl19': self.param_record['gbslo1_egl19'] * self.elements['m_slo1_egl19'] * self.elements[
                    'hcav_slo1_egl19'] * (output_volt + 80),
                'ik_slo1_unc2': self.param_record['gbslo1_unc2'] * self.elements['m_slo1_unc2'] * self.elements[
                    'hcav_slo1_unc2'] * (output_volt + 80),
                'ik_slo2_egl19': self.param_record['gbslo2_egl19'] * self.elements['m_slo2_egl19'] * self.elements[
                    'hcav_slo2_egl19'] * (output_volt + 80),
                'ik_slo2_unc2': self.param_record['gbslo2_unc2'] * self.elements['m_slo2_unc2'] * self.elements[
                    'hcav_slo2_unc2'] * (output_volt + 80),
                'ica_unc2': self.param_record['gbunc2'] * self.elements['m_unc2'] * self.elements['h_unc2'] * (
                            output_volt - 60)}

        return output_volt

