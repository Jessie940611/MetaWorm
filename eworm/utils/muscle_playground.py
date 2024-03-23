import numpy as np
import numpy.random as nr
import math
import os
import _pickle as pkl


def calculate_rotation_matrix(axis, theta):
    """
    Return the rotation matrix associated with counterclockwise rotation about
    the given axis by theta radians.
    """
    theta = np.pi*theta/180
    axis = np.asarray(axis)
    axis = axis / math.sqrt(np.dot(axis, axis))
    a = math.cos(theta / 2.0)
    b, c, d = -axis * math.sin(theta / 2.0)
    aa, bb, cc, dd = a * a, b * b, c * c, d * d
    bc, ad, ac, ab, bd, cd = b * c, a * d, a * c, a * b, b * d, c * d
    return np.array([[aa + bb - cc - dd, 2 * (bc + ad), 2 * (bd - ac)],
                     [2 * (bc - ad), aa + cc - bb - dd, 2 * (cd + ab)],
                     [2 * (bd + ac), 2 * (cd - ab), aa + dd - bb - cc]])


def muscle_data_factory(num, tstop, dt, start_phase, phase_shift, half_period_t, amp, input_volt, amp_decay=None,
                        forward=True, seed=None, with_sign=False):
    if seed is not None:
        nr.seed(seed)
    len_sim, half_period = int(tstop / dt), int(half_period_t / dt)
    wave_traces, wave_signs = [], []
    shift_sign = 1 if forward else -1
    x = np.arange(len_sim)
    for i in range(num):
        tmp_wave = np.sin((x/half_period)*np.pi+start_phase-shift_sign*(i/num)*phase_shift)
        wave_signs.append(np.sign(tmp_wave))
        if amp_decay is not None:
            amp_decay_rate = amp_decay**(i/num) if forward else amp_decay**((num-i)/num)
        else:
            amp_decay_rate = 1
        wave_traces.append(amp*amp_decay_rate*(tmp_wave+1)/2)
    wave_traces = np.array(wave_traces)
    wave_signs = np.array(wave_signs)
    if with_sign:
        input_wave = (np.sin((x/half_period)*np.pi) + 1)/2 * input_volt[1] + input_volt[0]
        return wave_traces, wave_signs, input_wave
    else:
        return wave_traces


def direct(muscle_traces, directions, amps):
    assert len(directions) == muscle_traces.shape[-1] == len(amps)
    zeros, ones = np.zeros(24), np.ones(24)
    chill = {
        "L": np.concatenate([ones, ones, zeros, zeros]),
        "R": np.concatenate([zeros, zeros, ones, ones]),
        "D": np.concatenate([zeros, ones, zeros, ones]),
        "V": np.concatenate([ones, zeros, ones, zeros]),
    }
    for time_step, direction in enumerate(directions):
        if np.linalg.norm(direction) > 0.1:            
            coef = (direction)**2
            # coef = (direction / np.linalg.norm(direction))**2
            loose = (coef[0] * chill['L'] + coef[1] * chill['R'] + coef[2] * chill['D'] + coef[3] * chill['V']) * amps[
                time_step]
            muscle_traces[:, time_step] = muscle_traces[:, time_step] * (1 - loose)
    return muscle_traces


def dir_norm(direction):
    return direction / np.linalg.norm(direction)


def memory_save(state_config, direction_name, direction_value, memory_capacity):
    state_config[direction_name].append(direction_value / np.linalg.norm(direction_value))
    state_config[direction_name] = state_config[direction_name][-memory_capacity:]
    direction_value = np.mean(np.stack(state_config[direction_name], axis=0), axis=0)
    direction_value = direction_value / np.linalg.norm(direction_value)
    return state_config, direction_value


def navigation(head_position, locomotion_direction, dorsal_direction, state_config=None, food_location=None, show_direction=False):
    if food_location is None:
        food_location = np.zeros(3)
    locomotion_direction, dorsal_direction = dir_norm(locomotion_direction), dir_norm(dorsal_direction)
    target_position = dir_norm(food_location - head_position)
    left_direction = np.cross(dorsal_direction, locomotion_direction)
    if state_config is None:
        state_config = {
            "trial_name": f"{np.round(abs(head_position[0]*np.sqrt(3)),2)}_{np.round(locomotion_direction[0],2)}_{np.round(locomotion_direction[1],2)}_{np.round(locomotion_direction[2],2)}",
            "data_config": {
                "start_phase": -np.pi,
                "phase_shift": 5 * np.pi,
                "dt": 1,
                "tstop": 100,
                "half_period_t": 5,
                "amp": 0.8,
                "input_volt": (-30, 40),
                "amp_decay": 0.1,
                "forward": True,},
            "left_direction": [],
            "dorsal_direction": [],
            "locomotion_direction": [],
        }
        state_config["data_config"]['start_phase'] = -np.pi
        test_muscle_13 = muscle_data_factory(num=24, **state_config["data_config"])

        state_config["data_config"]['start_phase'] = 0
        test_muscle_02 = muscle_data_factory(num=24, **state_config["data_config"])
        total_muscle = np.concatenate([test_muscle_02, test_muscle_13, test_muscle_02, test_muscle_13], axis=0)
        state_config["total_muscle"] = total_muscle
        state_config["phase"] = 0
    memory_capacity = int(2*state_config['data_config']['half_period_t'])
    # state_config, target_position = memory_save(state_config, "target_position", target_position, memory_capacity)
    state_config, left_direction = memory_save(state_config, "left_direction", left_direction, memory_capacity)
    state_config, dorsal_direction = memory_save(state_config, "dorsal_direction", dorsal_direction, memory_capacity)
    state_config, locomotion_direction = memory_save(state_config, "locomotion_direction", locomotion_direction, memory_capacity)
    
    direction = np.zeros(4) 
    if np.dot(target_position, left_direction) > 0:
        direction[0] = np.dot(target_position, left_direction)
    else:
        direction[1] = -np.dot(target_position, left_direction)
    if np.dot(target_position, dorsal_direction) > 0:
        direction[2] = np.dot(target_position, dorsal_direction)
    else:
        direction[3] = -np.dot(target_position, dorsal_direction)    
    # direction = direction / np.linalg.norm(direction)

    amp = 0.6
    # amp = min((1 - np.dot(target_position, locomotion_direction))/2, 0.6)

    if show_direction:
        print(f"Desire direction {direction**2} Amp {amp}")
        print(f"target_position {target_position} locomotion_direction {locomotion_direction}")
    tmp_muscle = state_config["total_muscle"][:, state_config["phase"] % state_config["total_muscle"].shape[-1]].reshape(-1, 1)
    loosed_muscle = direct(tmp_muscle, [direction], [amp])
    state_config["phase"] += 1
    return loosed_muscle, state_config


def algorithm(interact, i, state_config, show_direction=False):
    forward_direction, dorsal_direction = interact.get_forward_direction(i), interact.get_dorsal_direction(i)
    head_position = interact.get_head_location(i)
    init_position = interact.get_init_position(i)
    init_rotation = interact.get_init_rotation(i)
    rotation_matrix = calculate_rotation_matrix(init_rotation[1:], init_rotation[0])
    forward_direction = np.dot(rotation_matrix, forward_direction)
    dorsal_direction = np.dot(rotation_matrix, dorsal_direction)
    
    head_position = np.dot(rotation_matrix, head_position)+init_position
    if show_direction:
        print(f"Forward Vector {forward_direction}")
        print(f"Dorsal Vector {dorsal_direction}")
        print(f"Head Vector {head_position}")
    muscle_signal, state_config = navigation(head_position, forward_direction, dorsal_direction, state_config, show_direction=show_direction)

    #####Dataset Collection#######
    saving_dir = os.path.join(os.path.dirname(__file__), "dataset_collection")
    os.makedirs(saving_dir, exist_ok=True)
    cur_distance = np.linalg.norm(head_position)
    print("-"*10, f"Distance approaching:{cur_distance}")
    if cur_distance < 0.75:
        exit()
    if os.path.exists(os.path.join(saving_dir, state_config['trial_name']+".pkl")):
        data = pkl.load(open(os.path.join(saving_dir, state_config['trial_name']+".pkl"), "rb"))
        if len(data["Muscle"])>800:
            exit()
    else:
        data = {"Forward Vector": [], "Dorsal Vector": [], "Head Vector": [], "Muscle": []}
    data["Forward Vector"].append(forward_direction)
    data["Dorsal Vector"].append(dorsal_direction)
    data["Head Vector"].append(head_position)
    data["Muscle"].append(muscle_signal)
    pkl.dump(data, open(os.path.join(saving_dir, state_config['trial_name']+".pkl"), "wb"))
    return muscle_signal, state_config


if __name__ == "__main__":
    pass
