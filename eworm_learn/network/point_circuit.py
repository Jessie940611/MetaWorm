"""
Modules containing class that
    1. maintain detailed soma morphology information of the neurons in circuit
    2. function for single neuron simulation and whole circuit simulation in NEURON
"""
import numpy as np
from neuron import h
from network.detailed_circuit import *


class PointCell(Cell):
    """class for cell

    Attributes:
        index: int, cell index
        name: str, cell name
        cell_param: dictionary of biophysical info for the cell
        segments: list of Segment object
        points: list of Point object
        pre_connections, post_connections: list of Connection, the Cell acts as post-synaptic cell or pre-synaptic cell respectively
        hoc_obj: hoc object
        soma_volume, neurite_volume: volume of soma or neurite
    """
    def __init__(self, index, name, cell_param, rec_voltage=False, mode="active"):
        super(PointCell, self).__init__(index, name, cell_param, rec_voltage, mode)
        self.remove_neurite()
        self.neurite_volume = 0

    def remove_neurite(self):
        """remove neurites hoc_obj"""
        for section in self.hoc_obj.all:
            if 'Soma' not in section.name():
                section = None

    def prepare(self):
        """section, segment separation, prepare self.segments and self.points"""
        for section in self.hoc_obj.all:
            if 'Soma' not in section.name():
                continue
            section.nseg = 1
            self.points.append(
                Point(index=len(self.points), category=1,
                      location=np.array((section.x3d(0), section.y3d(0), section.z3d(0))),
                      diameter=section.diam3d(0), parent_id=-1))
            self.points.append(
                Point(index=len(self.points), category=2,
                      location=np.array((section.x3d(1), section.y3d(1), section.z3d(1))),
                      diameter=section.diam3d(1), parent_id=0))
            self.segments.append(
                Segment(index=len(self.segments), hoc_obj=section(0.5),
                        point1=self.points[0], point2=self.points[1],
                        cell=self, name=section.name(), rec_voltage=self.rec_v))

    def setup_biophysics(self, cell_param):
        """setup biophysical params of this neuron"""
        mechanism_list = ['slo1_unc2', 'egl2', 'shl1', 'kqt3', 'unc2',
                          'kvs1', 'slo1_egl19', 'slo2_unc2', 'irk', 'egl36',
                          'egl19', 'cca1', 'shk1', 'slo2_egl19', 'nca',
                          'kcnl', 'cainternm']
        self.hoc_obj.Soma.Ra = cell_param['soma']["Ra"]  # (Ohm*cm)
        self.hoc_obj.Soma.cm = cell_param['soma']["cm"]  # (uF/cm2)
        self.hoc_obj.Soma.insert("pas")
        for segment in self.hoc_obj.Soma:
            segment.pas.g = cell_param['soma']["gpas"]   # Passive conductance in S/cm2
            segment.pas.e = cell_param['soma']["epas"]   # Leak reversal potential mV
        if self.mode == "passive":
            return
        elif self.mode != "active":
            raise ValueError
        for mechanism in mechanism_list:
            self.hoc_obj.Soma.insert(mechanism)
        for segment in self.hoc_obj.Soma:
            segment.shl1.gbshl1 = cell_param["soma"]["gbshl1"]  # (nS/um2)
            segment.shk1.gbshk1 = cell_param["soma"]["gbshk1"]
            segment.kvs1.gbkvs1 = cell_param["soma"]["gbkvs1"]
            segment.egl2.gbegl2 = cell_param["soma"]["gbegl2"]
            segment.egl36.gbegl36 = cell_param["soma"]["gbegl36"]
            segment.kqt3.gbkqt3 = cell_param["soma"]["gbkqt3"]
            segment.egl19.gbegl19 = cell_param["soma"]["gbegl19"]
            segment.unc2.gbunc2 = cell_param["soma"]["gbunc2"]
            segment.cca1.gbcca1 = cell_param["soma"]["gbcca1"]
            segment.slo1_egl19.gbslo1 = cell_param["soma"]["gbslo1_egl19"]
            segment.slo1_unc2.gbslo1 = cell_param["soma"]["gbslo1_unc2"]
            segment.slo2_egl19.gbslo2 = cell_param["soma"]["gbslo2_egl19"]
            segment.slo2_unc2.gbslo2 = cell_param["soma"]["gbslo2_unc2"]
            segment.kcnl.gbkcnl = cell_param["soma"]["gbkcnl"]
            segment.nca.gbnca = cell_param["soma"]["gbnca"]
            segment.irk.gbirk = cell_param["soma"]["gbirk"]


class PointCircuit(DetailedCircuit):
    """class for point circuit

    Attributes:
        cells: list of all PointCell object in the circuit
        connections: list of Connection, all connections include input and output connections
        input_connections: list of Connection, input connections
        output_connections: list of Connection, output connections
    """
    def add_cell(self, new_cell):
        """cell append interface"""
        assert isinstance(new_cell, PointCell)
        self.cells.append(new_cell)
