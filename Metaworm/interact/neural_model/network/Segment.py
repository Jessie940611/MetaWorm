from neuron import h

class Segment(object):
    def __init__(self, segment, loc: list, diam: float, sec_name: str, id: int, pid: int, cate: int) -> None:
        self.segment = segment    # segment, hoc object
        self.loc = loc            # location[x,y,z], list of float
        self.diam = diam          # diam of segment, float
        self.sec_name = sec_name  # section name, string
        self.index = id           # index in self neuron, int
        self.parent_index = pid   # index of parent in self neuron, int
        self.volt_vec = h.Vector().record(self.segment._ref_v) # voltage Vector(), hoc Vector()
        self.category = cate      # category of this segment (Soma: 1, Neurite: 2), int
        self.as_pre_conn = []     # connections that self is the presynaptic segment, list of Connection
        self.as_post_conn = []    # connections that self is the postsynaptic segment, list of Connection


    def add_as_pre_connection(self, conn):
        # add connection that self is the presynaptic segment
        self.as_pre_conn.append(conn)
    

    def add_as_post_connection(self, conn):
        # add connection that self is the postsynaptic segment
        self.as_post_conn.append(conn)
