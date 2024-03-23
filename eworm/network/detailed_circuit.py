"""
Modules containing class that
    1. maintain detailed morphology information of the circuit.
    2. function for single neuron simulation and whole circuit simulation in NEURON
"""
import numpy as np
from neuron import h
from tqdm import trange


class Point:
    """abstract class recording point information

    Attributes:
        index: int, point index, starts from 0
        category: int, 1 for Soma, 2 for Neurite
        location: array of three float, [loc_x, loc_y, loc_z] indicate of location of the point
        diameter: float, point diameter
        parent_id: int, index of parent point
    """

    def __init__(self, index, category, location, diameter, parent_id):
        self.index, self.category, self.location, self.diameter, self.parent_id = \
            int(index), category, location, diameter, parent_id


class Segment:
    """class for segment between two points

    Attributes:
        cell: Cell class or Point Cell class, the cell that contains the segment
        index: int, segment index in the cell, starts from 0
        hoc_obj: hoc object generated by NEURON
        name: str, segment name generated by NEURON
        location: array of three float, [loc_x, loc_y, loc_z] indicate of location of the midpoint
        diameter: float
        pre_connections, post_connections: list of Connection, the segment acts as postsynaptic segment or presynaptic segment
        voltage: NEURON Vector class recording voltage of the segment
    """

    def __init__(self, index, hoc_obj, point1, point2, cell, name, rec_voltage=False):
        from eworm.network.point_circuit import PointCell
        assert isinstance(point1, Point) and isinstance(point2, Point)
        assert isinstance(cell, (Cell, PointCell))

        self.cell, self.index, self.hoc_obj, self.name = cell, int(index), hoc_obj, name
        self.location = (point1.location + point2.location) / 2
        self.diameter = (point1.diameter + point2.diameter) / 2
        self.point1 = point1
        self.point2 = point2
        self.pre_connections, self.post_connections = [], []
        self.voltage = None
        if rec_voltage:
            self.voltage = h.Vector().record(self.hoc_obj._ref_v)

    def __del__(self):
        self.hoc_obj = None

    def add_pre_connection(self, new_connection):
        """the new connection points to self segment"""
        assert isinstance(new_connection, Connection)

        if self.index == new_connection.post_segment.index:
            self.pre_connections.append(new_connection)
        else:
            raise ValueError("Segment Index Incompatible!")

    def add_post_connection(self, new_connection):
        """the new connection points from self segment"""
        assert isinstance(new_connection, Connection)

        if self.index == new_connection.pre_segment.index:
            self.post_connections.append(new_connection)
        else:
            raise ValueError("Segment Index Incompatible!")

    def voltage_switch(self, new_state=True):
        if new_state:
            if self.voltage is None:
                self.voltage = h.Vector().record(self.hoc_obj._ref_v)
        else:
            self.voltage = None


class Connection:
    """class for connection
    either be synapse (positive weight for excitatory and negative for inhibitory) or gap-junction

    Attributes:
        pre_segment, post_segment: Segment class, pre-synaptic segment or post-synaptic segment
        pre_cell, post_cell: Cell class or Point Cell class, pre-synaptic cell or post-synaptic cell
        category: str, either 'syn' or 'gj'
        weight: float
        hoc_obj: hoc object generated by NEURON
        pair_key: int or None, two gj belongs to one pair has the same unique key in the circuit, syn's key is None
    """

    def __init__(self, pre_segment, post_segment, category, weight, pair_key=None):
        assert category in ('syn', 'gj')
        assert isinstance(pre_segment, (Segment, type(None))) and \
               isinstance(post_segment, (Segment, type(None)))

        self.pre_segment, self.post_segment = pre_segment, post_segment
        self.pre_cell = pre_segment.cell if pre_segment is not None else None
        self.post_cell = post_segment.cell if post_segment is not None else None
        self.category, self.weight, self.pair_key = category, weight, pair_key
        self.hoc_obj = self.make_connection() if post_segment is not None else None

    def __del__(self):
        self.hoc_obj = None

    def make_connection(self):
        """make hoc object Connection according to self.category and self.weight"""
        if self.pre_segment is None:
            # connection receives out-circuit input
            if self.category == 'gj':
                hoc_connection = h.gapjunction_advance(self.post_segment.hoc_obj)
            elif self.category == 'syn':
                if self.weight >= 0:
                    hoc_connection = h.exc_syn_advance(self.post_segment.hoc_obj)
                else:
                    hoc_connection = h.inh_syn_advance(self.post_segment.hoc_obj)
            else:
                raise ValueError
        else:
            # connection receives other cell output
            if self.category == 'gj':
                hoc_connection = h.gapjunction(self.post_segment.hoc_obj)
            elif self.category == 'syn':
                if self.weight >= 0:
                    hoc_connection = h.neuron_to_neuron_exc_syn(self.post_segment.hoc_obj)
                else:
                    hoc_connection = h.neuron_to_neuron_inh_syn(self.post_segment.hoc_obj)
            else:
                raise ValueError
            hoc_connection._ref_vpre = self.pre_segment.hoc_obj._ref_v
        hoc_connection.weight = abs(self.weight)
        return hoc_connection

    def update_info(self, new_weight=None):
        """change the weight or category of the connection"""
        assert new_weight is not None
        if self.hoc_obj is None:
            return
        # do not change gapjunction or synapse type (inh/exc)
        if new_weight * self.weight > 0:
            self.weight = new_weight
            self.hoc_obj.weight = abs(self.weight)
        # change synapse type (inh/exc)
        else:
            self.hoc_obj = None  # delete the hoc object
            self.weight = new_weight
            self.hoc_obj = self.make_connection()


class Cell:
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

    def __init__(self, index, name, cell_param, rec_voltage=False, mode="active", length_per_seg=2, is_muscle=False):
        self.index, self.name, self.cell_param, self.rec_v, self.mode = int(index), name, cell_param, rec_voltage, mode
        self.is_muscle = is_muscle
        self.segments, self.points = [], []
        self.pre_connections, self.post_connections = [], []
        self.hoc_obj = getattr(h, self.name)()
        self.setup_biophysics(cell_param)
        self.prepare(length_per_seg)
        self.soma_volume, self.neurite_volume = self.calculate_volume()

    def __del__(self):
        for segment in self.segments:
            del segment
        for point in self.points:
            del point
        self.hoc_obj = None

    def release_hoc(self):
        """This explicit function is used to solve garbage collection issues before multiprocessing"""
        for segment in self.segments:
            segment.hoc_obj = None
            del segment
        for point in self.points:
            del point
        self.hoc_obj = None

    def prepare(self, length_per_seg):
        """section, segment separation, prepare self.segments and self.points"""
        section_endpoints = []
        for section in self.hoc_obj.all:
            soma_flag = 'Soma' in section.name()

            section.nseg = 1 if soma_flag else int(np.ceil(section.L / length_per_seg))
            init_loc = np.array((section.x3d(0), section.y3d(0), section.z3d(0)))

            # define start endpoint
            if soma_flag:
                self.points.append(
                    Point(index=len(self.points), category=1,
                          location=init_loc,
                          diameter=section.diam3d(0), parent_id=-1))
                previous_point_index = 0
                section_endpoints.append(0)
            else:
                assert len(section_endpoints) > 0
                previous_point_index = section_endpoints[np.argmin(
                    [np.linalg.norm(self.points[endpoint_index].location - init_loc)
                     for endpoint_index in section_endpoints])]

            # append segment
            loading_bar = np.linspace(0, 1, section.nseg + 1)
            loading_cnt = 1
            for start_ind in range(section.n3d() - 1):
                end_ind = start_ind + 1
                start_arc, end_arc = section.arc3d(start_ind) / section.L, section.arc3d(end_ind) / section.L
                start_loc = np.array((section.x3d(start_ind), section.y3d(start_ind), section.z3d(start_ind)))
                end_loc = np.array((section.x3d(end_ind), section.y3d(end_ind), section.z3d(end_ind)))
                start_diam, end_diam = section.diam3d(start_ind), section.diam3d(end_ind)

                while loading_cnt <= section.nseg and loading_bar[loading_cnt] <= end_arc:
                    ratio = (loading_bar[loading_cnt] - start_arc) / (end_arc - start_arc)
                    self.points.append(
                        Point(index=len(self.points), category=2,
                              location=(end_loc - start_loc) * ratio + start_loc,
                              diameter=(end_diam - start_diam) * ratio + start_diam,
                              parent_id=previous_point_index))
                    self.segments.append(
                        Segment(index=len(self.segments),
                                hoc_obj=section((loading_bar[loading_cnt] + loading_bar[loading_cnt - 1]) / 2),
                                point1=self.points[previous_point_index], point2=self.points[-1],
                                cell=self, name=section.name(), rec_voltage=self.rec_v))
                    previous_point_index = self.points[-1].index
                    loading_cnt += 1

            section_endpoints.append(self.points[-1].index)

    def calculate_volume(self):
        soma_volume, neurite_volume = 0, 0
        for section in self.hoc_obj.all:
            if "Soma" in section.name():
                for segment in section:
                    soma_volume += segment.volume()
            else:
                for segment in section:
                    neurite_volume += segment.volume()
        return soma_volume, neurite_volume

    def setup_biophysics(self, cell_param):
        """setup biophysical params of this neuron"""
        mechanism_list = ['slo1_unc2', 'egl2', 'shl1', 'kqt3', 'unc2',
                          'kvs1', 'slo1_egl19', 'slo2_unc2', 'irk', 'egl36',
                          'egl19', 'cca1', 'shk1', 'slo2_egl19', 'nca',
                          'kcnl', 'cainternm']
        for section in self.hoc_obj.all:
            category = 'soma' if 'Soma' in section.name() else 'neurite'

            section.Ra = cell_param[category]["Ra"]  # (Ohm*cm)
            section.cm = cell_param[category]["cm"]  # (uF/cm2)
            section.insert("pas")
            for segment in section:
                segment.pas.g = cell_param[category]["gpas"]  # Passive conductance in S/cm2
                segment.pas.e = cell_param[category]["epas"]  # Leak reversal potential mV
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

    def add_pre_connection(self, new_connection):
        """the new connection points to self cell"""
        assert isinstance(new_connection, Connection)

        if new_connection.post_cell.index == self.index:
            self.pre_connections.append(new_connection)
        else:
            raise ValueError("Connection post-synaptic cell incompatible!")

    def add_post_connection(self, new_connection):
        """the new connection points from self cell"""
        assert isinstance(new_connection, Connection)

        if new_connection.pre_cell.index == self.index:
            self.post_connections.append(new_connection)
        else:
            raise ValueError("Connection pre-synaptic cell incompatible!")

    def segment(self, segment_index=None, segment_name=None):
        """obtain the segment object according to its index or name"""
        assert (segment_index is not None) or (segment_name is not None)
        for segment in self.segments:
            if ((segment.index == segment_index) or (segment_index is None)) and \
                    ((segment.name == segment_name) or (segment_name is None)):
                return segment
        raise ValueError("Segment not found!")

    def update_connections(self, new_weights):
        """updating pre_connections weight"""
        assert len(self.pre_connections) == len(new_weights)
        for connection, new_weight in zip(self.pre_connections, new_weights):
            connection.update_info(new_weight)

    def simulation(self, sim_config, input_traces):
        """single cell simulation"""
        # preparation
        assert len(self.pre_connections) == len(input_traces)
        for pre_connection in self.pre_connections:
            assert pre_connection.pre_segment is None
        output_indices = np.unique([post_connection.pre_segment.index for post_connection in self.post_connections])
        for segment_index in output_indices:
            output_segment = self.segment(segment_index)
            output_segment.voltage = h.Vector().record(output_segment.hoc_obj._ref_v)
        h.dt, h.tstop, h.secondorder = sim_config['dt'], sim_config['tstop'], sim_config['secondorder']
        for connection in self.pre_connections:
            connection.hoc_obj.vpre = sim_config["v_init"]
        h.finitialize(sim_config['v_init'])
        trace_length = int(sim_config['tstop'] / sim_config['dt'])
        for time_step in range(trace_length):
            for connect_index, connect in enumerate(self.pre_connections):
                connect.hoc_obj.vpre = input_traces[connect_index, time_step]
            h.fadvance()
        output_traces = np.stack([np.array(self.segment(segment_index).voltage.to_python())
                                  for segment_index in output_indices], axis=0)
        return output_traces


class DetailedCircuit:
    """class for circuit

    Attributes:
        cells: list of all Cell object in the circuit
        connections: list of Connection, all connections include input and output connections
        input_connections: list of Connection, input connections
        output_connections: list of Connection, output connections
    """

    def __init__(self):
        self.cells, self.connections = [], []
        self.input_connections, self.output_connections = [], []

    def __del__(self):
        for connection in self.connections:
            del connection
        for cell in self.cells:
            del cell

    def release_hoc(self):
        """This explicit function is used to solve garbage collection issues before multiprocessing"""
        for connection in self.connections:
            connection.hoc_obj = None
            del connection
        for cell in self.cells:
            cell.release_hoc()
            del cell

    def add_cell(self, new_cell):
        """cell append interface"""
        assert isinstance(new_cell, Cell)
        self.cells.append(new_cell)

    def add_connection(self, new_connection):
        """connection append interface"""
        assert isinstance(new_connection, Connection)
        self.connections.append(new_connection)
        if new_connection.pre_segment is None:
            self.input_connections.append(new_connection)
        else:
            new_connection.pre_segment.add_post_connection(new_connection)
            new_connection.pre_cell.add_post_connection(new_connection)

        if new_connection.post_segment is None:
            self.output_connections.append(new_connection)
        else:
            new_connection.post_segment.add_pre_connection(new_connection)
            new_connection.post_cell.add_pre_connection(new_connection)

    def cell(self, cell_index=None, cell_name=None):
        """obtain the cell object according to its index or name"""
        assert (cell_index is not None) or (cell_name is not None)
        for cell in self.cells:
            if ((cell.index == cell_index) or (cell_index is None)) and \
                    ((cell.name == cell_name) or (cell_name is None)):
                return cell
        raise ValueError("Cell not found!")

    def simulation(self, sim_config, input_traces, input_cell_names, output_cell_names, make_history=False):
        """init and simulate according to given input"""
        if make_history and (not self.cells[0].rec_v):
            self.voltage_switch(make_history)
        h.dt, h.tstop = sim_config['dt'], sim_config['tstop']
        # h.steps_per_ms = int(1 / sim_config['dt'])
        h.secondorder = sim_config['secondorder']
        for connection in self.input_connections:
            connection.hoc_obj.vpre = sim_config["v_init"]
        h.finitialize(sim_config['v_init'])
        trace_length = int(sim_config['tstop'] / sim_config['dt'])
        assert input_traces.shape == (len(input_cell_names), trace_length)
        input_index = dict(zip(input_cell_names, range(len(input_cell_names))))
        output_index = dict(zip(output_cell_names, range(len(output_cell_names))))
        output_traces = np.zeros((len(output_cell_names), trace_length), dtype=np.float32)
        for time_step in trange(trace_length):
            for connection in self.input_connections:
                connection.hoc_obj.vpre = input_traces[input_index[connection.post_cell.name], time_step]
            h.fadvance()
            for cell_name in output_cell_names:
                output_traces[output_index[cell_name], time_step] = \
                    self.cell(cell_name=cell_name).hoc_obj.Soma(0.5).v
        if make_history:
            for connection in self.connections:
                if connection.pre_segment is None:
                    connection.history = input_traces[input_index[connection.post_cell.name]]
                else:
                    connection.history = np.array(connection.pre_segment.voltage.to_python()[:-1])
        return output_traces

    def update_connections(self, new_weights):
        """updating connects weight"""
        assert len(self.connections) == len(new_weights)
        for connection, new_weight in zip(self.connections, new_weights):
            connection.update_info(new_weight)

    def voltage_switch(self, new_state=True):
        for connection in self.connections:
            if connection.pre_segment is not None:
                connection.pre_segment.voltage_switch(new_state)
