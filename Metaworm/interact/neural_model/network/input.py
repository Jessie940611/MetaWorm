# This file is to give neurons stimulus


from network.Network import Network
from network.Connection import Connection
from network.Cell import Cell
from network.Segment import Segment
from network.init import *
from network_assemble.data_structure import Circuit, Neuron, Connect, make_connect
from network_assemble.data_structure import Segment as segment
from utils.jsonfunc import *
from utils.excelfunc import *
from utils.distance import *
import pickle
import numpy as np
from neuron import h

# add a constance current input
def add_constant_current_stimulus(net: Network, cell_name, section, loc, start_time, end_time, amplitude):
    # net: a network
    # cell_name: string, e.g. "AWAL"
    # section: string, e.g. "Soma"
    # loc: 0-1 float, e.g. 0.5
    cell = net.name_cell_dict[cell_name].cell
    stim = h.IClamp(getattr(cell, section)(loc))
    stim.delay = start_time
    stim.dur = end_time - start_time
    stim.amp = amplitude * 1e-3  # (nA)
    record = {k:h.Vector().record(v) for k,v in zip(['t', 'v', 'stim_i', 'amp'],
                                                [h._ref_t, 
                                                        getattr(cell, section)(loc)._ref_v, 
                                                        stim._ref_i, 
                                                        stim._ref_amp])}
    net.add_stim(stim)
    return record


# give one neuron a current input according to an array
def add_varing_current_stimulus(net: Network, cell_name, section, loc, input):
    # neuron_name: string, "AWAL"
    # section: string, "Soma"
    # loc: 0-1 float, 0.5
    cell = net.name_cell_dict[cell_name].cell
    stim = h.IClamp(getattr(cell, section)(loc))
    stim.dur = 1e9
    play_stim = h.Vector().from_python(input)  # (pA)    
    play_stim.play(stim, stim._ref_amp, h.dt)
    record = {k:h.Vector().record(v) for k,v in zip(['t', 'v', 'stim_i', 'amp'],
                                                    [h._ref_t, 
                                                     getattr(cell, section)(loc)._ref_v, 
                                                     stim._ref_i, 
                                                     stim._ref_amp])}
    net.add_stim(stim)
    net.add_play_stim(play_stim)
    return record
