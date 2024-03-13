import os
import itertools
import operator as op
import collections
import random

import serial
import numpy as np
import pandas as pd

from setup_paths import *

SEND_LINE_END = '\r'
SERIAL_TIMEOUT = 1.0
SERIAL_BAUD = 115200
SERIAL_PORT = 'COM5'

def measure(meas_config) -> str:
    '''Function that controls the serial measurement and returns the results
    Args:
        meas_config: configuration parameters of the measurement
    Returns: the response string
    '''
    try:
        ser = serial.Serial(SERIAL_PORT, SERIAL_BAUD,
                            timeout=SERIAL_TIMEOUT)
    except Exception as err:
        print('Error while opening serial: ', err)
    else:
        with ser:
            # reset connection
            ser.reset_input_buffer()
            ser.write(SEND_LINE_END.encode('ascii'))
            ser.read_until(b'>')

            # configure measurement
            string_to_send = 'direction ' \
                           + meas_config['direction'] \
                           + SEND_LINE_END
            string_to_send = string_to_send.encode('ascii')
            ser.write(string_to_send) #'r' or 's'
            resp = ser.readline() # readline reads until \n

            string_to_send = 'clk ' \
                        + str(meas_config['clkM7'] * 1000000) \
                        + ' ' \
                        + str(meas_config['clkM4'] * 1000000) \
                        + SEND_LINE_END # converted to Hz
            string_to_send = string_to_send.encode('ascii')
            ser.write(string_to_send)
            resp = ser.readline()

            string_to_send = 'repeat ' \
                           + str(meas_config['repeat']) \
                           + SEND_LINE_END
            string_to_send = string_to_send.encode('ascii')
            ser.write(string_to_send)
            resp = ser.readline()
            
            string_to_send = 'datasize ' \
                           + str(meas_config['datasize']) \
                           + SEND_LINE_END
            string_to_send = string_to_send.encode('ascii')
            ser.write(string_to_send)
            resp = ser.readline()

            string_to_send = 'mem ' \
                           + str(meas_config['mem']) \
                           + SEND_LINE_END
            string_to_send = string_to_send.encode('ascii')
            ser.write(string_to_send)
            resp = ser.readline()

            string_to_send = 'cache ' \
                           + str(meas_config['cache']) \
                           + SEND_LINE_END
            string_to_send = string_to_send.encode('ascii')
            ser.write(string_to_send)
            resp = ser.readline()

            # start measurement
            string_to_send = 'start' + SEND_LINE_END
            string_to_send = string_to_send.encode('ascii')
            ser.write(string_to_send)
            resp = ser.readline()

            # read measured values
            ser.timeout = None # block indefinitely
            response = ser.read_until(b'>') \
                          .removesuffix(b'>') \
                          .decode('ascii')
    assert len(response.splitlines()) == meas_config['repeat'] + 1
    return response

def write_meas_to_file(response, meas_config, base_dir=None):
    '''Function for writing the measurement results similarly to putty
    Args:
        response: measurement data as a string
        meas_config: dict, measurement configuration parameters'''
    dir_prefix, filename = get_path_for_meas(meas_config, base_dir)
    os.makedirs(os.path.join(MEASUREMENTS_PATH, dir_prefix), exist_ok=True)

    path = os.path.join(MEASUREMENTS_PATH, dir_prefix, filename)
    with open(path, 'w', encoding='utf-8') as file:
        file.write(response)
    print(f'written to {path}') # todo might log

def config_to_config_list(meas_configs):
    '''Makes a list that contains the cartesian product of all
    configurations from the given dict
    
    Args:
        meas_configs: dict of lists of all config parameters'''
    # product of values
    config_values_list = itertools.product(*meas_configs.values())
    meas_config_list = [
        {k: v for k, v in zip(meas_configs.keys(), config_values)}
        for config_values
        in config_values_list] # remaking into meas_config dict form
    meas_config_list = [x for x in meas_config_list 
                        if x['clkM4'] <= x['clkM7']] # valid clk pair
    return meas_config_list

def group_config_except(config_list: list[dict], blacklist:list[str]):
    '''Groups the dicts in the config_list, except for the keywords given
    in blacklist
    
    Note: the elements grouped will NOT be lists'''
    assert 0 < len(config_list)
    key = op.itemgetter(*[k for k in config_list[0] if k not in blacklist])
    s = sorted(config_list, key=key)
    groups = []
    for _, g in itertools.groupby(s, key):
        blacklist_vals = collections.defaultdict(set)
        for config_dict in g:
            for k in blacklist:
                blacklist_vals[k].add(config_dict[k])
        meas_configs = config_dict.copy()
        for k in blacklist_vals:
            blacklist_vals[k] = sorted(list(blacklist_vals[k]))
        meas_configs.update(dict(blacklist_vals))
        groups.append(meas_configs)
    return groups

def check_meas_content(meas_config: dict, content: pd.DataFrame) -> bool:
    '''Checks if the content is in sync with the meas configuration 
    parameters'''
    if False in content['couldBlock']:
        print("WARN: measurement has value where couldn't block")
    if (content['mem'][0] == meas_config['mem']
        and content['cache'][0] == meas_config['cache']
        and content['datasize'][0] == meas_config['datasize']
        and content['clkM4'][0] / 1_000_000 == meas_config['clkM4'] # MHz
        and content['clkM7'][0] / 1_000_000 == meas_config['clkM7']
        and (content['direction'][0][0] == meas_config['direction'] # abbreviation is the first letter
             or content['direction'][0] == meas_config['direction'])
        and content['repeat'][0] == meas_config['repeat']
        and content['time'].size == meas_config['repeat']):
        return True
    else:
        return False

def get_path_for_meas(meas_config:dict, base_dir=None,
                      filename_prefix='meas_', ext='.csv'):
    '''Returns the path to the measurement file, relative to 
    MEASUREMENT_PATH
    
    Args:
        meas_config: dict, config parameters
        base_dir: base directory relative to MEASUREMENT_PATH
        filename_prefix: filename prefix of the files storing the 
            measurements
        ext: file extension
        
    Returns: 
        dir_prefix: directories relative to MEASUREMENT_PATH
        filename: filename to be used'''
    filename = filename_prefix + str(meas_config['datasize']) + ext
    dir_prefix = (f"meas_{meas_config['direction']}"
                  f"_{meas_config['clkM7']}"
                  f"_{meas_config['clkM4']}")
    dir_prefix = os.path.join(f'{meas_config["mem"]}_{meas_config["cache"]}',
                              dir_prefix)
    if base_dir is not None:
        dir_prefix = os.path.join(base_dir, dir_prefix)
    return dir_prefix, filename

def read_meas_from_file(
        meas_config: dict,
        base_dir=None,
        checkpoint_type='time'
        ) -> tuple[np.ndarray, int]:
    '''Returns the measurement values as a numpy array to the given 
    measurement config

    Args:
        meas_config: dict of the configuration parameters of the meas
        base_dir: base directory relative to MEASUREMENT_PATH
        checkpoint_type: the field in the input file which should be read
        
    Returns:
        meas_values: ndarray of measurement values (type depends on
            checkpoint_type)
        timer: timer clk [MHz]'''
    dir_prefix, filename = get_path_for_meas(meas_config, base_dir)
    path = os.path.join(MEASUREMENTS_PATH, dir_prefix, filename)
    content = pd.read_csv(path, encoding='utf-8')
    assert check_meas_content(meas_config, content)
    return (content[checkpoint_type].to_numpy(),
            content['timer'][0] / 1_000_000)

def read_meas_from_files(
        meas_configs,
        base_dir=None,
        checkpoint_type='time'
        ) -> tuple[list[np.ndarray], list[int], list[dict]]:
    '''Read all files for all possible configurations

    Args:
        meas_configs: dict of lists, cartesian product of the values will 
            be used (single values don't have to be lists)
        base_dir: base directory relative to MEASUREMENT_PATH

    Returns: 
        all_meas_values: list of the measurements
        timers: list of timer clks used for time measurement [MHz]
        meas_config_list: corresponding dict of configs'''
    meas_configs = {k: (v if isinstance(v, list) else [v])
                    for  k, v in meas_configs.items()}
    config_values_list = itertools.product(*meas_configs.values()) # product of values
    meas_config_list = [
        {k: v for k, v in zip(meas_configs.keys(), config_values)}
        for config_values
        in config_values_list] # remaking into meas_config dict form
    meas_config_list = [x for x in meas_config_list 
                        if x['clkM4'] <= x['clkM7']] # invalid clk pair
    all_meas_values, timers = [], []
    for meas_config in meas_config_list:
        meas_values, timer = read_meas_from_file(meas_config,
                                                 base_dir,
                                                 checkpoint_type)
        all_meas_values.append(meas_values)
        timers.append(timer)
    return all_meas_values, timers, meas_config_list

def get_and_calc_meas(meas_configs, meas_type, base_dir=None,
                      checkpoint_type='time'):
    '''Reads measurement values (mean, min, max) and calculates datarates
        or latencies

    Args:
        meas_configs: dict of lists of configuration parameters 
            (single elements don't have to be lists)
        meas_type: 'datarate' or 'latency'
        base_dir: base directory relative to MEASUREMENT_PATH

    Returns:
        datarates/latencies: np.array(mean, min, max), 
            shape: (3, len(meas_configs)) [Mbyte/s]
        meas_config_list: list of config dicts'''
    (all_meas_values,
        timers,
        meas_config_list) = read_meas_from_files(meas_configs, base_dir,
                                                 checkpoint_type)
    all_meas_values = np.vstack(all_meas_values)
    timers = np.asarray(timers)
    sizes = [x['datasize'] for x in meas_config_list]
    if 'datarate' == meas_type:
        data_min = sizes / np.max(all_meas_values, axis=1) * timers # Mbyte/s
        data_max = sizes / np.min(all_meas_values, axis=1) * timers # Mbyte/s
        data_mean = sizes / np.mean(all_meas_values, axis=1) * timers # Mbyte/s
    elif 'latency' == meas_type:
        data_mean = np.mean(all_meas_values, axis=1) / timers # us
        data_min = np.min(all_meas_values, axis=1) / timers # us
        data_max = np.max(all_meas_values, axis=1) / timers # us
    else:
        raise RuntimeError('type is neither datarate nor latency')
    return np.asarray((data_mean, data_min, data_max)), meas_config_list

def upper_lower_from_minmax(mean_min_max):
    '''Calculates lower and upper error from min and max
    Args:
        np.array of shape (3, y), holding mean, min, max     
    Returns:
        np.array of shape (3, y), holding mean, lower error, upper error
    '''
    assert len(mean_min_max.shape) == 2 and mean_min_max.shape[0] == 3
    mean_lower_upper = np.ndarray(mean_min_max.shape)
    mean_lower_upper[0, :] = mean_min_max[0, :] # mean
    mean_lower_upper[1, :] = mean_min_max[0, :] - mean_min_max[1, :] # mean - min
    mean_lower_upper[2, :] = mean_min_max[2, :] - mean_min_max[0, :] # max - mean
    return mean_lower_upper

def main():
    '''Measuring for several different sizes, saving the result to file'''
    # num_meas = 1024

    pilot_sizes = [1, 512, 16376]
    sizes =  ([1]
              + [16*x for x in range(1, 17)]
              + [512, 1024, 1536]
              + [1024*x for x in range(2, 16)] + [16376])
    
    # meas_configs = {
    #         'direction': ['r', 's'],
    #         'clkM7': [60, 480],
    #         'clkM4': [60, 240],
    #         'repeat': [8192],
    #         'datasize': pilot_sizes,
    #         'mem': ['D1', 'D2', 'D3'],
    #         'cache': ['none', 'id'],
    # }
    meas_configs = {
            'direction': ['r', 's'],
            'clkM7': [60, 120, 240, 480],
            'clkM4': [60, 120, 240],
            'repeat': [256],
            'datasize': sizes,
            'mem': ['D1', 'D2', 'D3'],
            'cache': ['none', 'i', 'd', 'id'],
    }
    base_dir = 'v13_Ofast'

    config_list = config_to_config_list(meas_configs)
    # to shuffle completely randomly
    random.shuffle(config_list)
    # to order some way for effective cache usage
    config_list = sorted(config_list, 
                         key=lambda x:(x['cache'], x['mem']))

    for meas_config in config_list:
        response = measure(meas_config)
        write_meas_to_file(response, meas_config, base_dir)

if __name__ == '__main__':
    main()
