import os
import serial
import numpy as np
import itertools

from setup_paths import *

SEND_LINE_END = '\r'
SERIAL_TIMEOUT = 1.0
SERIAL_BAUD = 115200
SERIAL_PORT = 'COM5'

def measure(meas_config) -> list:
    '''Function that controlls the serial measurement and returns the results
    Args:
        meas_config: configuration parameters of the measurement
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
                        + str(meas_config['m7_clk'] * 1000000) \
                        + ' ' \
                        + str(meas_config['m4_clk'] * 1000000) \
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
    assert(len(response.splitlines()) == meas_config['repeat'] + 1)
    return response

def write_meas_to_file(dir_prefix, response, meas_config):
    '''Function for writing the measurement results similarly to putty
    Args:
        timer_clock: timer clock frequency in MHz
        response: measurement data
        sent_data_size: number of bytes sent
        num_meas: repetition count of the measurement
        direction: 'r' or 's' for the direction of the IPC communication'''
    filename = f'meas_{meas_config["datasize"]}.csv'
    fullpath = os.path.join(dir_prefix, filename)
    MODE = 'w' if os.path.exists(fullpath) else 'x'
    with open(fullpath, MODE, encoding='utf-8') as file:
        # header
        file.write(response)
    print(f'written to {filename}')

def read_meas_from_files(sizes, dir_prefix,
                         filename_prefix='meas') -> list[list]:
    '''Read all files for the all data sizes
    Args:
        sizes: list of sizes to be measured
        dir_prefix: folder of the measurement files
        filename_prefix: common first part of the files containing the measurement values
    Returns: a list of lists that contain all the measurement values'''
    filenames = [os.path.join(dir_prefix, f'{filename_prefix}{x}.log') for x in sizes]
    all_meas_values = []
    for i, filename in enumerate(filenames):
        # cutting the expected datasize from the filename
        buffer_len = sizes[i]

        cur_meas_values = []
        with open(filename, 'r', encoding='ascii') as file:
            file.readline() # header
            file.readline() # s
            meas_length = int(file.readline()) # length of the measurement
            file.readline() # empty line
            read_buffer_len = int(file.readline())
            if read_buffer_len != buffer_len:
                print('Wrong buffer size')
            file.readline() # empty line, could be left out
            for line in file:
                line = line.strip() # strip line ending
                if line: # if not empty line
                    line = line.split(sep=' ')[0]
                    cur_meas_values.append(int(line))
        if len(cur_meas_values) != meas_length: # read data and expected length
            print('Wrong file len')
        all_meas_values.append(cur_meas_values)
    return all_meas_values

def get_and_calc_meas(timer_clock, dir_prefix, sizes, meas_type):
    '''Reads measurement values (mean, min, max) and calculates datarates
        or latencies

    Args:
        timer_clock: timer clock frequency in [MHz]
        dir_prefix: name of the directory
        sizes: measured message sizes
        meas_type: 'datarate' or 'latency'

    Returns:
        np.array(mean, min, max), shape: (3, len(sizes)) [Mbyte/s]'''
    all_meas_values = np.array(read_meas_from_files(sizes, dir_prefix))
    if 'datarate' == meas_type:
        data_min = sizes / np.max(all_meas_values, axis=1) * timer_clock # Mbyte/s
        data_max = sizes / np.min(all_meas_values, axis=1) * timer_clock # Mbyte/s
        data_mean = sizes / np.mean(all_meas_values, axis=1) * timer_clock # Mbyte/s
    elif 'latency' == meas_type:
        data_mean = np.mean(all_meas_values, axis=1) / timer_clock # us
        data_min = np.min(all_meas_values, axis=1) / timer_clock # us
        data_max = np.max(all_meas_values, axis=1) / timer_clock # us
    else:
        raise RuntimeError('type not datarate of latency')
    return np.array((data_mean, data_min, data_max))

def get_all_latencies(clocks, sizes, meas_num=1024,\
                      dir_prefix_without_clk='meas_'):
    '''Reads all measurement values for each clk and size
    Args:
        clocks: list of tuple of clks (m7, m4)
        sizes: list of sizes
        meas_num: number of measurements in each file
        dir_prefix_without_clk: dir prefix without the clks 
            e.g. meas_ in case of meas_72_72
    Returns:
        np.array() with size (len(clocks), len(sizes), num_meas)'''
    all_latencies = np.empty((0, len(sizes), meas_num))
    for m7, m4 in clocks:
        new_meas_values = np.array( \
            read_meas_from_files(sizes, f'{dir_prefix_without_clk}{m7}_{m4}'))
        new_meas_values = np.expand_dims(new_meas_values, axis=0)
        all_latencies = np.concatenate(
            (all_latencies, new_meas_values / m4), axis=0) #us
    return all_latencies

def upper_lower_from_minmax(mean_min_max):
    '''Calculates lower and upper error from min and max
    Args:
        np.array of shape (x, 3, y), holding mean, min, max     
    Returns:
        np.array of shape (x, 3, y), holding mean, lower error, upper error
    '''
    assert len(mean_min_max.shape) == 3 and mean_min_max.shape[1] == 3
    mean_lower_upper = np.ndarray(mean_min_max.shape)
    mean_lower_upper[:, 0] = mean_min_max[:, 0] # mean
    mean_lower_upper[:, 1] = mean_min_max[:, 0] - mean_min_max[:, 1] # mean - min
    mean_lower_upper[:, 2] = mean_min_max[:, 2] - mean_min_max[:, 0] # max - mean
    return mean_lower_upper

def main():
    '''Measuring for several different sizes, saving the result to file'''
    # num_meas = 1024

    sizes_short = [1 if x==0 else 16*x for x in range(17)]
    sizes_long = [1 if x==0 else 1024*x for x in range(16)] + [512, 1536, 16376]
    sizes_max = [16380] # actual size is 16376
    #config begin
    caches = ['none', 'i', 'd', 'id']
    mems = ['D1', 'D2', 'D3']
    datasizes = [1, 256, 4096, 16380]#sizes_long[1:] + sizes_short
    directions = ['r', 's']
    m7_clks = 120
    m4_clks = 120
    # todo itertools.product


    # for direction in meas_directions:
    #     for sent_data_size in sizes:
    meas_config = {
        'direction': 'r',
        'm7_clk': 60,
        'm4_clk': 60,
        'repeat': 256,
        'datasize': 16376,
        'mem': 'D1',
        'cache': 'none',
    }
    dir_prefix = 'meas_{0}_{1}_{2}' \
                    .format(meas_config['direction'],
                            meas_config['m7_clk'],
                            meas_config['m4_clk'])
    dir_prefix = os.path.join(MEASUREMENTS_PATH,
                            meas_config["mem"] + '_' + meas_config['cache'],
                            dir_prefix)
    if not os.path.exists(dir_prefix):
        os.makedirs(dir_prefix)
    response = measure(meas_config)
    write_meas_to_file(dir_prefix, response, meas_config)

if __name__ == '__main__':
    main()
