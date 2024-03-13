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
        self.direction = direction
        self.mem = mem
        self.cache = cache
        if (direction is not None 
            and mem is not None 
            and cache is not None):
            self.params = self.allparams[mem][cache][direction]
        else:
            self.params = None

    def set_model_from_config(self, meas_configs):
        '''Sets the model parameters according to the meas_configs dict
        
        Args:
            meas_configs: dict holding the measurement configuration, the
                signgle elements can be lists'''
        mem = meas_configs['mem']
        cache = meas_configs['cache']
        direction = meas_configs['direction']
        if isinstance(mem, list): 
            mem = mem[0]
        if isinstance(cache, list): 
            cache = cache[0]
        if isinstance(direction, list): 
            direction = direction[0]
        self.set_model(mem, cache, direction)

    def get_latency(self, m7, m4, sizes):
        '''Calculates the latency based on the model'''
        assert self.params is not None
        m7 = np.asarray(m7).reshape((-1, 1, 1))
        m4 = np.asarray(m4).reshape((1, -1, 1))
        sizes = np.asarray(sizes).reshape((1, 1, -1))
        m7_const, m7_variable, m4_variable, m4_const = self.params
        pred = m7_const/m7 + m4_const/m4 \
               + m7_variable/m7*sizes + m4_variable/m4*sizes
        return pred.squeeze()

    def get_latency_from_config(self, meas_configs):
        '''Calculates latency based on the model
        
        Args:
            meas_configs: dict of measurement configuration parameters'''
        m7 = meas_configs['clkM7']
        m4 = meas_configs['clkM4']
        sizes = meas_configs['datasize']
        return self.get_latency(m7, m4, sizes)

    def get_datarate(self, m7, m4, sizes):
        '''Calculates the datarate based on the model'''
        return np.array(sizes)/self.get_latency(m7, m4, sizes)

    def get_datarate_from_config(self, meas_configs):
        '''Calculates the datarate based on the model
        
        Args:
            meas_configs: dict of measurement configuration parameters'''
        m7 = meas_configs['clkM7']
        m4 = meas_configs['clkM4']
        sizes = meas_configs['datasize']
        return self.get_datarate(m7, m4, sizes)

    def get_output(self, m7, m4, sizes, meas_type):
        '''Calculates datarate or latency depending on meas_type'''
        if meas_type == 'latency':
            return self.get_latency(m7, m4, sizes)
        elif meas_type == 'datarate':
            return self.get_datarate(m7, m4, sizes)
        else:
            raise RuntimeError('Invalid measurement type')

    def get_output_from_config(self, meas_configs, meas_type):
        '''Calculates datarate or latency depending on meas_type'''
        if meas_type == 'latency':
            return self.get_latency_from_config(meas_configs)
        elif meas_type == 'datarate':
            return self.get_datarate_from_config(meas_configs)
        else:
            raise RuntimeError('Invalid measurement type')

    def get_grid_from_config(self, grouped_config, meas_type, clock_res=100):
        '''Calculates the output in the range of the given clocks

        Args:
            grouped_config: dict of configuration parameters, where the 
                clocks are lists, the other parameters single values

        Returns:
            m7: m7 clock axis values
            m4: m4 clock axis values
            pred: predictions on the grid'''
        conf_m7, conf_m4 = grouped_config['clkM7'], grouped_config['clkM4']
        assert isinstance(conf_m7, list)
        assert isinstance(conf_m4, list)
        m7 = np.linspace(np.min(conf_m7), np.max(conf_m7), clock_res)
        m4 = np.linspace(np.min(conf_m4), np.max(conf_m4), clock_res)
        pred = self.get_output(m7, m4, grouped_config['datasize'],
                               meas_type)
        return m7, m4, pred

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

def print_table(json_path, nest=2):
    '''Print latex table from the json file, memories can be filtered with mem_regex'''
    def print_nesting(nest):
        print(' ' * 4 * nest, end='')

    def print_nested(nest, *args, end='\n', sep=' '):
        print_nesting(nest)
        print(*args, end=end, sep=sep)

    with open(json_path, 'r') as file:
        params = json.load(file)

    for direction in ['r', 's']:
        dir_text = ('M4-M7' if direction=='r' else 'M7-M4')
        print_nested(nest, '\\midrule')
        print_nested(nest, dir_text + '& \\multicolumn{4}{ c }{}\\\\')
        print_nested(nest, '\\midrule')
        for mem in params:
            for cache in params[mem]:
                cache_text = ('without cache' 
                            if cache == 'none'
                            else f"with {str.upper(cache)} cache")
                print_nested(nest, f'{mem} {cache_text}', end=' & ')
                float_params = ['{:.2f}'.format(e) for e in params[mem][cache][direction]]
                print(*float_params, end='\\\\\n', sep=' & ')

def get_mse(model_out, true_out, axis):
    '''Returns mse between the inputs, mean along the axis'''
    model_out, true_out = np.array(model_out), np.array(true_out)
    return np.mean(np.square(model_out - true_out), axis=axis)
