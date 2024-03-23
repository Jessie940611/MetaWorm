# This file is for visualization
# you can export data for neuronXcore to see the 3D model
# or export voltage figures of all neurons


from network.Network import Network
from network.Connection import Connection
from network.Cell import Cell
from network.Segment import Segment
import numpy as np
from matplotlib import pyplot as plt
import os


# export morphology data (.swc) or synapse data (.snp) for neuronXcore
def export_morphology_and_synapse(net: Network, morph_path: str, syn_path: str):
    # net: a network
    # morph_path: save morphology data (.swc) to this path
    # syn_path: save synapse data (.snp) to this path
    point_info = []
    with open(morph_path, "w") as f:  # to clear the file
        pass
    ipoint = 1
    id_dict = {}
    for cell in net.cells:
        for seg in cell.segs:
            id = ipoint + seg.index
            [x,y,z] = seg.loc
            diam = seg.diam
            pid = (seg.parent_index + ipoint) if (seg.parent_index != -1) else seg.parent_index
            cate = seg.category
            point_info.append("%d %d %.4f %.4f %.4f %.4f %d\n"%(id, cate, x, y, z, diam, pid))
            if len(point_info) > 5000:
                with open(morph_path, "a") as f:
                    f.writelines(point_info)
                    point_info = []
            if (cell.index, seg.index) not in id_dict:
                id_dict[(cell.index, seg.index)] = id
            else:
                assert id_dict[(cell.index, seg.index)] == id
        ipoint += cell.nseg
    if len(point_info) > 0:
        with open(morph_path, "a") as f:
            f.writelines(point_info)
    print("saved morphology to %s"%morph_path)
    

    with open(syn_path, 'w') as file:
        cate_dic = {'gap_jun':0, 'exc_syn':1, 'inh_syn':2}
        for conn in net.connections:
            if not conn.pre_cell or not conn.pre_segment:
                continue
            pre_id = id_dict[(conn.pre_cell.index, conn.pre_segment.index)]
            post_id = id_dict[(conn.post_cell.index, conn.post_segment.index)]
            cate = cate_dic[conn.category]
            file.write('%d %d %d 1\n' % (pre_id, post_id, cate))
    print("saved synapse info to %s"%syn_path)



# export voltage data (.vtg) of all segment for neuronXcore
def export_voltage(net: Network, path: str):
    # a network
    # path: save voltage data (.vtg) to this path
    import struct
    with open(path, 'wb') as voltageFile:
        time_step = len(net.cells[0].segs[0].volt_vec.as_numpy())
        print(time_step)
        voltageFile.write(struct.pack("i", time_step))  
    with open(path, 'ab') as voltageFile:
        for cell in net.cells:
            for seg in cell.segs:
                volt = seg.volt_vec.as_numpy().astype('float32')
                voltageFile.write(volt.tobytes())
    print("saved voltage to %s"%path)


# save figures of all neurons (red: soma trace, gray: other segments)
def export_volt_figure(net: Network, path: str):
    # net: a network
    # path: save figures to this path    
    assert os.path.isdir(path)
    for cell in net.cells:
        plt.figure(figsize=(10,4))
        for seg in cell.segs:
            plt.plot(net.t_vec, seg.volt_vec, linewidth=1, color=[0.8, 0.8, 0.8])
        plt.plot(net.t_vec, cell.segs[0].volt_vec, linewidth=1, color=[1,0,0])
        plt.ylabel("voltage (mV)")
        plt.xlabel("time (ms)")
        plt.tight_layout()
        plt.savefig(os.path.join(path, str(cell.index) + "_" + cell.name + '.png'))
        plt.close()
    print("saved figures to %s"%path)