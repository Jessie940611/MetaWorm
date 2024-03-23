from network.Cell import Cell
from network.Segment import Segment
from neuron import h


class Connection(object):
    def __init__(self, pre_cell: Cell, pre_seg:Segment, post_cell: Cell, post_seg: Segment, cate: str, w: float) -> None:
        assert cate in ['gap_jun', 'exc_syn', 'inh_syn']
        assert (w >= 0 and cate in ['gap_jun', 'exc_syn']) or (w <= 0 and cate in ['inh_syn'])
        self.connection = None       # gapjunction or synapse, hoc object
        self.pre_cell = pre_cell     # pre connection cell, Cell object
        self.pre_segment = pre_seg   # post connection segment, Segment object
        self.post_cell = post_cell   # post connection cell, Cell object
        self.post_segment = post_seg # post connection segment, Segment object
        self.category = cate         # category ('gap_jun', 'exc_syn', 'inh_syn'), string
        self.weight = abs(w)         # connection weight, float
        self.make_connection()
    

    def make_connection(self):
        # construct connection, include normal connection and input connection (for external stimulus)
        if self.pre_cell and self.pre_segment: # normal connection
            if self.category == 'gap_jun':
                f = 'gapjunction'
            elif self.category == 'exc_syn':
                f = 'neuron_to_neuron_exc_syn'
            elif self.category == 'inh_syn':
                f = 'neuron_to_neuron_inh_syn'
            else: 
                print("Wrong connection category!")
            self.connection = getattr(h, f)(self.post_segment.segment)
            self.connection._ref_vpre = self.pre_segment.segment._ref_v
        else: # input connection
            if self.category == 'gap_jun':
                f = 'gapjunction_advance'
            elif self.category == 'exc_syn':
                f = 'exc_syn_advance'
            elif self.category == 'inh_syn':
                f = 'inh_syn_advance'
            else: 
                print("Wrong connection category!")
            self.connection = getattr(h, f)(self.post_segment.segment)
        self.connection.weight = self.weight
    

    def change_conn_category(self, cate, w):
        if cate == self.category:
            self.weight = abs(w)
        else:
            self.category = cate
            self.weight = abs(w)
            self.connection = None
            if self.pre_cell and self.pre_segment: # normal connection
                if self.category == 'gap_jun':
                    f = 'gapjunction'
                elif self.category == 'exc_syn':
                    f = 'neuron_to_neuron_exc_syn'
                elif self.category == 'inh_syn':
                    f = 'neuron_to_neuron_inh_syn'
                else: 
                    print("Wrong connection category!")
                self.connection = getattr(h, f)(self.post_segment.segment)
                self.connection._ref_vpre = self.pre_segment.segment._ref_v
            else: # input connection
                if self.category == 'gap_jun':
                    f = 'gapjunction_advance'
                elif self.category == 'exc_syn':
                    f = 'exc_syn_advance'
                elif self.category == 'inh_syn':
                    f = 'inh_syn_advance'
                else: 
                    print("Wrong connection category!")
                self.connection = getattr(h, f)(self.post_segment.segment)
        self.connection.weight = self.weight
        
