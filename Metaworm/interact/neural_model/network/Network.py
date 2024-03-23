from network.Connection import Connection
from network.Cell import Cell
from network.Segment import Segment
from neuron import h


class Network(object):
    def __init__(self) -> None:
        self.ncell = 0               # number of cells, int
        self.cells = []              # all cells, list of Cell object
        self.name_cell_dict = {}     # key: cell name, str; value cell, Cell object
        self.index_cell_dict = {}    # key: cell index, str; value cell, Cell object
        self.connections = []        # all connections, list of Connection object
        self.input_connections = []  # all input connections, list of Connection object
        self.stims = []              # all stimulus, list of stim hoc object
        self._play_stims = []        # all played stimulus, only for varing stimulus
        self.t_vec = None            # time steps, a Vector
    

    def add_cell(self, cell: Cell):
        # add cell for self network and set time record
        self.cells.append(cell)
        self.name_cell_dict[cell.name] = cell
        self.index_cell_dict[cell.index] = cell
        self.ncell += 1
        self.t_vec = h.Vector().record(h._ref_t)
    

    def add_connection(self, conn: Connection):
        # add connection for self network
        self.connections.append(conn)
        if conn.pre_cell and conn.pre_segment:
            conn.pre_cell.add_as_pre_connection(conn)
            conn.pre_segment.add_as_pre_connection(conn)
        else:
            self.input_connections.append(conn)
        conn.post_cell.add_as_post_connection(conn)        
        conn.post_segment.add_as_post_connection(conn)


    def add_stim(self, stim):
        # add stimulus for self network
        self.stims.append(stim)


    def add_play_stim(self, play_stim):
        # add play stimulus for self network
        self._play_stims.append(play_stim)

