import json

def load_json(filename):
    with open(filename,'r+') as f:
        data_dic = json.load(f)
    return data_dic