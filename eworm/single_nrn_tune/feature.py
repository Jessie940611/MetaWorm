import numpy as np

min_volt, max_volt = -100, 50
min_cur, max_cur = 0, 0.5
dynamic_type = 'v'


def volt_norm(array_):
    if dynamic_type == 'v':
        return (array_ - min_volt) / (max_volt - min_volt)
    elif dynamic_type == 'i':
        return (array_ - min_cur) / (max_cur - min_cur)
    else:
        raise NotImplementedError


def mean_abs(trace):
    mean_ = np.mean(trace, axis=-1)
    abs_ = np.mean(np.abs(trace - mean_[..., np.newaxis]), axis=-1)
    return volt_norm(mean_), abs_


def min_max(trace):
    max_ = np.max(trace, axis=-1)
    min_ = np.min(trace, axis=-1)
    return volt_norm(max_), volt_norm(min_)


def extract_features(dynamic_data):
    """
    #####Example#####
    Separate traces into different phases, each phase calculate the mean & variance (& min & max)
    """
    assert dynamic_data.ndim >= 2
    feature = np.concatenate((
        *mean_abs(dynamic_data[..., :, 0:240]),
        *mean_abs(dynamic_data[..., :, 260:900]),
        *mean_abs(dynamic_data[..., :, 900:1450]),
        *mean_abs(dynamic_data[..., :, 1600:]),
        *min_max(dynamic_data[..., :, 260:550]),
        *min_max(dynamic_data[..., :, 550:900]),
        *min_max(dynamic_data[..., :, 900:1450]),
        *min_max(dynamic_data[..., :, 1350:1500]),
    ), axis=-1)
    return feature

