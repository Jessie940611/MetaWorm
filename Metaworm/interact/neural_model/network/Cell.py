from neuron import h
from network.Segment import Segment
import numpy as np


class Cell(object):
    def __init__(self, index: int, name: str, cell_param: dict) -> None:
        # name: cell name, string
        # index: cell index, int
        # cell_param: cell parameters, dictionary
        self.index = index        # cell index, int
        self.name = name          # cell name, string        
        self.cell = None          # cell itself, hoc object
        self.nseg = 0             # segment number, int
        self.segs = []            # segments, list of Segment object
        self.soma_volumn = 0      # volume of soma, float
        self.neurite_volume = 0   # volume of neurites, float
        self.as_pre_conn = []     # connections that self is the presynaptic neuron, list of Connection object
        self.as_post_conn = []    # connections that self is the postsynaptic neuron, list of Connection object
        self.create_cell()
        self.setup_segments()
        self.calc_volume()
        self.setup_biophysics(cell_param)


    def create_cell(self):
        # setup self.cell
        self.cell = getattr(h, self.name)()


    def setup_segments(self):
        # setup self.segs
        cnt = int(0)
        segs = []
        sec_start_end_id = np.zeros((20,1),dtype=np.int32)
        sec_start_end_loc = np.zeros((20,3), dtype=np.float16)
        ssec = 0
        dp = []
        for sec in self.cell.all:
            npt3d = sec.n3d()
            rg = range(npt3d)
            for p3d_i in rg:
                if p3d_i == 0 or p3d_i == sec.n3d()-1:
                    x = sec.x3d(p3d_i)
                    y = sec.y3d(p3d_i)
                    z = sec.z3d(p3d_i)
                    sec_start_end_id[ssec,:] = cnt
                    sec_start_end_loc[ssec,:] = np.array([x, y, z])
                    ssec += 1
                    dp.append(cnt)
                # parent id
                if 'Soma' in sec.name():
                    if p3d_i == 0:
                        pid = -1
                        cate = 1
                    else:
                        pid = cnt - 1
                        cate = 2
                else:
                    cate = 2
                    if p3d_i == 0:
                        xyz = np.array([x, y, z])
                        idist = np.argmin(np.array([np.linalg.norm(_ - xyz) for _ in sec_start_end_loc]))
                        pid = sec_start_end_id[idist]
                    else:
                        pid = cnt - 1
                x = sec.x3d(p3d_i)
                y = sec.y3d(p3d_i)
                z = sec.z3d(p3d_i)
                d = sec.diam3d(p3d_i)
                arc = sec.arc3d(p3d_i)
                segment = sec(arc/sec.L)
                seg = Segment(segment=segment, loc=[x,y,z], diam=d, sec_name=sec.name(), id=cnt, pid=pid, cate=cate)
                segs.append(seg)
                cnt += 1
        self.segs = segs
        self.nseg = cnt
        self.dp = dp  # (for distance calculation) index list of segments that located in the start and end of section
        self.points = np.array([x.loc for x in self.segs]) # (for distance calculation)

    
    def calc_volume(self):
        # calculate volume of soma and neurites
        soma_volume = 0
        neurite_volume = 0
        for sec in self.cell.all:
            if "Soma" in sec.name():
                for seg in sec:
                    soma_volume += seg.volume()
            else:
                for seg in sec:
                    neurite_volume += seg.volume()
        self.soma_volumn = soma_volume
        self.neurite_volume = neurite_volume


    def setup_biophysics(self, cell_param):
        # setup biophysical parmas of this neuron
        mech_list = ['slo1_unc2', 'egl2', 'shl1', 'kqt3', 'unc2', 'kvs1', 'slo1_egl19', 'slo2_unc2', 'irk', 'egl36', 'egl19', 'cca1', 'shk1', 'slo2_egl19', 'nca', 'kcnl','cainternm']
        sec = self.cell.Soma
        sec.Ra = cell_param["soma"]["Ra"]  # (Ohm*cm)
        sec.cm = cell_param["soma"]["cm"]  # (uF/cm2)
        sec.insert('pas')
        for seg in sec:
            seg.pas.g = cell_param["soma"]["gpas"]  # Passive conductance in S/cm2
            seg.pas.e = cell_param["soma"]["epas"]  # Leak reversal potential mV
        for sec in self.cell.all:
            if "Soma" in sec.name():
                continue
            sec.Ra = cell_param["neurite"]["Ra"]  # (Ohm*cm)
            sec.cm = cell_param["neurite"]["cm"]  # (uF/cm2)
            sec.insert('pas')
            for seg in sec:
                seg.pas.g = cell_param["neurite"]["gpas"]  # Passive conductance in S/cm2
                seg.pas.e = cell_param["neurite"]["epas"]  # Leak reversal potential mV
        for m in mech_list:
            self.cell.Soma.insert(m)
        for seg in self.cell.Soma:
            seg.shl1.gbshl1 = cell_param["soma"]["gbshl1"]  # (nS/um2)
            seg.shk1.gbshk1 = cell_param["soma"]["gbshk1"]
            seg.kvs1.gbkvs1 = cell_param["soma"]["gbkvs1"]
            seg.egl2.gbegl2 = cell_param["soma"]["gbegl2"]
            seg.egl36.gbegl36 = cell_param["soma"]["gbegl36"]
            seg.kqt3.gbkqt3 = cell_param["soma"]["gbkqt3"]
            seg.egl19.gbegl19 = cell_param["soma"]["gbegl19"]
            seg.unc2.gbunc2 = cell_param["soma"]["gbunc2"]
            seg.cca1.gbcca1 = cell_param["soma"]["gbcca1"]
            seg.slo1_egl19.gbslo1 = cell_param["soma"]["gbslo1_egl19"]
            seg.slo1_unc2.gbslo1 = cell_param["soma"]["gbslo1_unc2"]
            seg.slo2_egl19.gbslo2 = cell_param["soma"]["gbslo2_egl19"]
            seg.slo2_unc2.gbslo2 = cell_param["soma"]["gbslo2_unc2"]
            seg.kcnl.gbkcnl = cell_param["soma"]["gbkcnl"]
            seg.nca.gbnca = cell_param["soma"]["gbnca"]
            seg.irk.gbirk = cell_param["soma"]["gbirk"]


    def add_as_pre_connection(self, conn):
        # add connections that self is the presynaptic neuron
        self.as_pre_conn.append(conn)
    

    def add_as_post_connection(self, conn):
        # add connections that self is the postsynaptic neuron
        self.as_post_conn.append(conn)


