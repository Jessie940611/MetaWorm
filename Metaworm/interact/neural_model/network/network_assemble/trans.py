def set_up_net_connection(self, connect_file):
    self.input_synapse = [None, None, None, None]
    circuit = pickle.load(open(connect_file, 'rb'))
    for post_neuron in circuit.neurons:  # for each post neuron
        id_post_cell = post_neuron.index
        if (id_post_cell > 0) and (str(id_post_cell) not in self.cells_id_sim):
            print("neuron ids in config file and connection do not match!")
            exit()
        for pre_connect in post_neuron.pre:
            id_pre_cell = pre_connect.pre.index
            id_pre_point = pre_connect.pre_segment.index
            id_post_point = pre_connect.post_segment.index
            category = pre_connect.category
            w = pre_connect.weight
            if (id_pre_cell > 0) and (str(id_pre_cell) not in self.cells_id_sim):  # for each pre neuron
                print("neuron ids in config file and connection do not match!")
                exit()
            else:
                pre_seg = self.get_3dp_segment(str(id_pre_cell), id_pre_point) if id_pre_cell > 0 else None
                post_seg = self.get_3dp_segment(str(id_post_cell), id_post_point)
                if category == 'gj': # construct gap junction
                    cate = 0
                    wx = w
                    gapjunction = h.gapjunction(post_seg)
                    gapjunction.weight = w
                    gapjunction._ref_vpre = pre_seg._ref_v
                    self.synapse_list.append(gapjunction)
                elif category == 'syn' and w > 0:
                    # construct synapse
                    cate = 1
                    wx = w
                    synapse = h.neuron_to_neuron_exc_syn(post_seg) if pre_seg else h.exc_syn_advance(post_seg)
                    synapse.weight = w
                    if pre_seg:
                        synapse._ref_vpre = pre_seg._ref_v
                    self.synapse_list.append(synapse)
                elif category == 'syn' and w < 0:
                    cate = 2
                    wx = w
                    synapse = h.neuron_to_neuron_inh_syn(post_seg) if pre_seg else h.inh_syn_advance(post_seg)
                    synapse.weight = abs(w)
                    if pre_seg:
                        synapse._ref_vpre = pre_seg._ref_v
                    self.synapse_list.append(synapse)
                if not pre_seg:
                    self.input_synapse[int(abs(id_pre_cell)-1)] = synapse

                if id_pre_cell > 0:
                    x_pre, y_pre, z_pre = self.get_3dp_info(str(id_pre_cell), id_pre_point)
                    x_post, y_post, z_post = self.get_3dp_info(str(id_post_cell), id_post_point)
                    self.synapse_points.append([x_pre, y_pre, z_pre, x_post, y_post, z_post, cate])

                # cell level connection info
                if id_post_cell in self.id_connected_id.keys():
                    self.id_connected_id[id_post_cell].append(id_pre_cell)
                else:
                    self.id_connected_id[id_post_cell] = [id_pre_cell]
                if id_post_cell in self.id_connected_weight.keys():
                    self.id_connected_weight[id_post_cell].append(wx)
                else:
                    self.id_connected_weight[id_post_cell] = [wx]
