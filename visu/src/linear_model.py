import json
import regex
import numpy as np

class LinearModel():
    '''Linear model'''
    def __init__(self, json_path, mem=None, cache=None, direction=None):
        self.load_params(json_path)
        self.set_model(mem, cache, direction)

    def load_params(self, json_path):
        '''Loads the model params from the json file'''
        with open(json_path, 'r') as file:
            params = json.load(file)
        self.allparams = params

    def set_model(self, mem, cache, direction):
        '''Sets the model parameters for mem and direction'''
        self.params = self.allparams[mem][cache][direction]
        self.direction = direction
        self.mem = mem

    def get_latency(self, m7, m4, sizes):
        '''Calculates the latency based on the model'''
        m7 = np.array(m7).reshape((-1, 1, 1))
        m4 = np.array(m4).reshape((1, -1, 1))
        sizes = np.array(sizes).reshape((1, 1, -1))
        m7_const, m7_variable, m4_variable, m4_const = self.params
        pred = m7_const/m7 + m4_const/m4 \
               + m7_variable/m7*sizes + m4_variable/m4*sizes
        return pred.squeeze()
    
    def get_datarate(self, m7, m4, sizes):
        '''Calculates the datarate based on the model'''
        return np.array(sizes)/self.get_latency(m7, m4, sizes)
    
    def get_output(self, m7, m4, sizes, meas_type):
        '''Calculates datarate or latency depending on meas_type'''
        if meas_type == 'latency':
            return self.get_latency(m7, m4, sizes)
        elif meas_type == 'datarate':
            return self.get_datarate(m7, m4, sizes)
        else:
            raise RuntimeError('Invalid measurement type')
        
    def get_grid_for_range(self, clocks, size, meas_type, clock_res=100):
        '''Calculates the output in the range of the given clocks
        Args:
            clock_res: resolution of the clock grid
        Returns:
            m7, m4, pred: the grid of clocks used and the predictions'''
        m7, m4 = zip(*clocks)
        m7 = np.linspace(np.min(m7), np.max(m7), clock_res)
        m4 = np.linspace(np.min(m4), np.max(m4), clock_res)
        return m7, m4, self.get_output(m7, m4, size, meas_type)

def print_table(json_path, mem_regex):
    '''Print latex table from the json file, memories can be filtered with mem_regex'''
    pattern = regex.compile(pattern=mem_regex)
    with open(json_path, 'r') as file:
        params = json.load(file)

    for direction in ['r', 's']:
        print('        \\midrule\n        '
            + ('M4-M7' if direction=='r' else 'M7-M4')
            + '& \\multicolumn{4}{ c }{}\\\\\n'
            '        \\midrule')
        for k, v in params.items():
            if pattern.fullmatch(k):
                print('        ' 
                    + k.replace('_idcache_mpu_ncacheable', ' gyorsítótárral')
                    + ' & '
                    , end='')
                for i, elem in enumerate(v[direction]):
                    if i == 3:
                        print('{:.2f}'.format(elem) + '\\\\')
                    else:
                        print("{:.2f}".format(elem) + ' & ', end='')

def get_mse(model_out, true_out, axis):
    '''Returns mse between the inputs, mean along the axis'''
    model_out, true_out = np.array(model_out), np.array(true_out)
    return np.mean(np.square(model_out - true_out), axis=axis)
