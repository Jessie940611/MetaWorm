from unicodedata import category


class Circuit:
    """
    Class for neuronal circuit, keep and maintain all the neuron cells, connects, segments in it.
    Notice that cell index < 0 means fake input neuron, and cell index > 1000 means fake output muscle
    """
    def __init__(self):
        """
        neurons: keep all neuron cells in the circuit, list of class "Neuron"
        connects: keep all connects in the circuit, list of class "Connect"
        """
        self.neurons = []
        self.connects = []

    def add_neuron(self, neuron_new):
        assert isinstance(neuron_new, Neuron)

        for nrn in self.neurons:
            if nrn.index == neuron_new.index:
                return "previously inserted"
        self.neurons.append(neuron_new)

    def add_connect(self, connect_new):
        assert isinstance(connect_new, Connect)

        self.connects.append(connect_new)
        connect_new.pre.add_post_connect(connect_new)
        connect_new.pre_segment.add_post_connect(connect_new)
        connect_new.post.add_pre_connect(connect_new)
        connect_new.post_segment.add_pre_connect(connect_new)

    def neuron(self, neuron_index):
        """
        get the specific neuron via its index
        :return: class "Neuron" instance
        """
        for nrn in self.neurons:
            if nrn.index == neuron_index:
                return nrn
        raise ValueError("QAQ Neuron Not Found 404 QAQ")

    def change_pre_weight(self, neuron_index, new_weights, weight_categories):
        """
        change neuron #neuron_index all pre connects' weights, weight_categories are provided for verification
        """
        for connect_index, pre_connect in enumerate(self.neuron(neuron_index).pre):
            assert pre_connect.category == weight_categories[connect_index]
            pre_connect.weight = new_weights[connect_index]
    
    def change_net_weight(self, new_weights, weight_categories):
        assert len(new_weights) == len(self.connects)
        assert len(weight_categories) == len(self.connects)
        for conn, weight, category in zip(self.connects ,new_weights, weight_categories):
            assert conn.category == category
            conn.weight = weight


class Neuron:
    """
    Class for neuron cell
    """
    def __init__(self, index: int):
        """
        :param index: the index of neuron cell
        pre: from some other neuron cells (pre-synaptic) point to self (post-synaptic), list of class "Connect"
        post: from self (pre-synaptic) point to some other neuron cells (post-synaptic), list of class "Connect"
        segments: all segment in this neuron, list of class "Segment"
        """
        self.pre = []
        self.post = []
        self.segments = []
        self.index = index

    def add_segment(self, segment_new):
        assert isinstance(segment_new, Segment)

        for seg in self.segments:
            if seg.index == segment_new.index:
                return "previously inserted"
        self.segments.append(segment_new)

    def add_pre_connect(self, connect_new):
        assert isinstance(connect_new, Connect)

        if self.index == connect_new.post.index:
            self.pre.append(connect_new)
            self.add_segment(connect_new.post_segment)
        else:
            raise ValueError('! connect_new added to the wrong neuron !')

    def add_post_connect(self, connect_new):
        assert isinstance(connect_new, Connect)

        if self.index == connect_new.pre.index:
            self.post.append(connect_new)
            self.add_segment(connect_new.pre_segment)
        else:
            raise ValueError('! connect_new added to the wrong neuron !')

    def segment(self, seg_index):
        """
        get the specific segment via its index
        :return: class "Segment" instance
        """
        for seg in self.segments:
            if seg.index == seg_index:
                return seg
        raise ValueError("QAQ Segment Not Found 404 QAQ")


class Connect(object):
    """
    Class for connection between cell.
    The reason why write it as a class instead of a dictionary
    is that "connect_1['pre']" is longer than "connect_1.pre"
    """
    def __init__(self, pre, post, pre_segment, post_segment, category: str, weight):
        """
        :param pre: pre-synaptic neuron with type "Neuron"
        :param post: post-synaptic neuron with type "Neuron"
        :param pre_segment: pre-synaptic segment, in pre-synaptic neuron "pre"
        :param post_segment: post-synaptic segment, in post-synaptic neuron "post"
        :param category: synapse ('syn') or gap junction ('gj')
        :param weight: float or None
        """
        assert isinstance(pre, Neuron) and isinstance(post, Neuron)
        assert isinstance(pre_segment, Segment) and isinstance(post_segment, Segment)
        assert isinstance(weight, float) or (weight is None)
        assert category in ('syn', 'gj')
        if category == 'gj':
            assert (weight is None) or (weight >= 0)

        self.pre = pre
        self.post = post
        self.pre_segment = pre_segment
        self.post_segment = post_segment
        self.category = category
        self.weight = weight


class Segment:
    """
    This class is created due the require to enumerate all segments in one neurons.
    Like class "Connect", this class only serve as a dictionary
    """
    def __init__(self, neuron_index: int, index: int, pre: list, post: list):
        """
        :param neuron_index: the index of neuron in which segment lies
        :param index: the index of segment
        :param pre: from another segment (pre_synaptic) point to self (post_synaptic), list of class "Connect"
        :param post: from self (pre_synaptic) point to another segment(post_synaptic), list of class "Connect"
        """
        self.neuron_index = neuron_index
        self.index = index
        self.pre = pre
        self.post = post

    def add_pre_connect(self, connect_new):
        assert isinstance(connect_new, Connect)

        if self.index == connect_new.post_segment.index:
            self.pre.append(connect_new)
        else:
            raise ValueError('! connect_new added to the wrong segment !')

    def add_post_connect(self, connect_new):
        assert isinstance(connect_new, Connect)

        if self.index == connect_new.pre_segment.index:
            self.post.append(connect_new)
        else:
            raise ValueError('! connect_new added to the wrong segment !')


def make_connect(
        circuit_base: Circuit,
        pre_nrn_idx: int, pre_seg_idx: int,
        post_nrn_idx: int, post_seg_idx: int,
        category: str, weight=None):
    """
    :param circuit_base: new connect should be inserted in this circuit
    :param pre_nrn_idx: pre-synaptic neuron index
    :param pre_seg_idx: pre-synaptic segment index
    :param post_nrn_idx: post-synaptic neuron index
    :param post_seg_idx: post-synaptic neuron index
    :param category: synapse ('syn') or gap junction ('gj')
    :param weight: float or None
    """
    circuit_base.add_neuron(Neuron(pre_nrn_idx))
    pre_neuron = circuit_base.neuron(pre_nrn_idx)
    circuit_base.add_neuron(Neuron(post_nrn_idx))
    post_neuron = circuit_base.neuron(post_nrn_idx)
    pre_neuron.add_segment(Segment(pre_nrn_idx, pre_seg_idx, [], []))
    pre_segment = pre_neuron.segment(pre_seg_idx)
    post_neuron.add_segment(Segment(post_nrn_idx, post_seg_idx, [], []))
    post_segment = post_neuron.segment(post_seg_idx)
    connect_new = Connect(pre_neuron, post_neuron, pre_segment, post_segment, category, weight)
    return connect_new


if __name__ == '__main__':
    """
    Example: how to use class circuit?
    0. create instance circuit
    1. create connect via function "make_connect"
    2. add created connect to circuit via circuit attribute "add_connect"
    3. keep doing 1. & 2. if need
    """
    import sys
    import pickle
    import _pickle
    import numpy as np

    connection_info = pickle.load(open("data/head_simple_connection.pkl", 'rb'))

    example_circuit = Circuit()
    for id_post_cell in connection_info.keys():
        for [id_pre_cell, id_pre_point, id_post_point, type_, w] in connection_info[id_post_cell]:
            if id_pre_point is None:
                if int(id_pre_cell) < 0:
                    id_pre_point = 1
                else:
                    raise ValueError
            if id_post_point is None:
                if int(id_post_cell) > 1000:
                    id_post_point = 1
                else:
                    raise ValueError
            if type_ == 0:
                type__ = 'gj'
            else:
                type__ = 'syn'
            new_connect = make_connect(
                example_circuit,
                int(id_pre_cell), id_pre_point, int(id_post_cell), id_post_point, type__, 5e-2)
            example_circuit.add_connect(new_connect)
    print('Circuit established! :D')

    # Test
    # get the connection information of whole circuit
    for neuron in example_circuit.neurons:
        print('neuron id: ', neuron.index)
        print(np.unique([cnt.pre_segment.index for cnt in neuron.post]))
        print([cnt.post_segment.index for cnt in neuron.pre])
    for neuron in example_circuit.neurons:
        print(neuron.index)
    exit()
    # get all output segment of neuron 13
    print(np.unique([cnt.pre_segment.index for cnt in example_circuit.neuron(13).post]))
    print([cnt.post_segment.index for cnt in example_circuit.neuron(13).pre])

    # get all 1-level down steam neurons of neuron 13
    print([cnt.post.index for cnt in example_circuit.neuron(13).post])

    # sys.setrecursionlimit(10000)
    # _pickle.dump(example_circuit, open('data/head_simple.pkl', 'wb'), protocol=2)
