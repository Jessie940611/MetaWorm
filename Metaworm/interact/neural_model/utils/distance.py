from pykdtree.kdtree import KDTree
import numpy as np

# input:
# pre_points: pre neuron 3d points, shape (npoint, ndim)
# post_points: post neuron 3d points, shape (npoint, ndim)
# nsyn: synapse number
# thresh: distance threshold
# output:
# pre_id: an array, pre neuron 3d point id where synapse on
# post_id: an array, post neuron 3d point id where synapase on
# dist: an array, distance between two points
# cnsyn: constructed synapse number
# these three array have the same shape
def find_synapse_loc(pre_points, post_points, nsyn, thresh, dp):
    kd_tree = KDTree(pre_points)
    if np.all(pre_points == post_points):
        dist, idx = kd_tree.query(post_points, k = 2)
        dist = np.squeeze(np.delete(dist, 0, axis=-1))
        idx = np.squeeze(np.delete(idx, 0, axis=-1))
    else:
        dist, idx = kd_tree.query(post_points, k = 1)

    # sort (first sort, then unique, then sort)
    sorted_id1 = np.argsort(dist, kind='quicksort')
    d1, d2, d3 = dist[sorted_id1], idx[sorted_id1], sorted_id1

    # unique
    unique, unique_id, unique_cnt = np.unique(d2, return_index=True, return_inverse=False, return_counts=True)
    c1, c2, c3 = d1[unique_id], d2[unique_id], d3[unique_id]
    di = []
    su = []
    if np.all(pre_points == post_points):
        for i in range(len(c1)):
            if abs(c2[i] - c3[i]) <= 1.1 or (c3[i], c2[i]) in su or c2[i] in dp or c3[i] in dp:
                di.append(i)
            else:
                su.append((c2[i], c3[i]))
        c1 = np.delete(c1, di)
        c2 = np.delete(c2, di)
        c3 = np.delete(c3, di)
                

    # sort
    sorted_id2 = np.argsort(c1, kind='quicksort')
    dist, pre_id, post_id = c1[sorted_id2], c2[sorted_id2], c3[sorted_id2]

    cncand = 0
    while(cncand < len(dist)):
        if dist[cncand] > thresh: # or abs(dist[cnsyn] - 0) < 10e-3:
            break
        cncand += 1
    if cncand == 0:
        return None, None, None, 0
    elif cncand <= nsyn:
        return pre_id[:cncand], post_id[:cncand], dist[:cncand], cncand
    else:
        selectid = np.random.choice(cncand, nsyn, replace=False)
        return pre_id[selectid], post_id[selectid], dist[selectid], len(selectid)



if __name__ == "__main__":
    pre_points = np.array([[1,2,3], [3,4,5], [5,6,7],[7,8,9]])
    post_points = np.array([[1,1,1], [3,3,3], [5,5,5], [13,9,9]])
    for i, (a, b) in enumerate(zip(pre_points, post_points)):
        print(i,a,b)
    #print(find_synapse_loc(pre_points, post_points, 3, 6))