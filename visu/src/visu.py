import os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl

from setup_paths import *
import measurement as meas
import linear_model

def model_plot(sizes, data, m7, m4, mem, color, if_label=False):
    '''Plot for data in function of size, formatted for the model'''
    plt.plot(sizes, data, alpha=0.5, linestyle='dashed', color=color,
             label=(f'{mem}pred, {m7}, {m4}' if if_label else None))
    
def model_plot_from_config(data, config, color, if_label=False):
    '''Plot for data in function of size, formatted for the model
    
    Args:
        meas_config: grouped configs, 'datasize' has to be a list'''
    model_plot(config['datasize'], data, config['clkM7'], config['clkM4'],
               config['mem'], color, if_label=if_label)

def errorbars(config_list, sizes, datas, cmap, if_line=True, base_dir=None):
    '''Errorbar plot for several clock frequency, latency mesaurement
    
    Args:
        config_list: list of config dicts
        sizes: list of the sent data sizes
        latencies: array of lantencies (mean, neg_err, pos_err)
            for each clk and size [clocks, 3, sizes]
    
    Note: there's no plt.figure call, so it can be used with subplot'''
    for i, (config, data) in enumerate(zip(config_list, datas)):
        means = data[0, :]
        errors = data[1:, :]
        label = (f'{config["mem"]}_{config["cache"]}, '
                f'{config["clkM7"]}, {config["clkM4"]}')
        if base_dir is not None:
            label += ', ' + base_dir
        plt.errorbar(sizes, means, yerr=errors, fmt='o', capsize=5,\
                     label=label, color=cmap[i])
        if if_line:
            plt.plot(sizes, means, alpha=0.5, color=cmap[i])

def setup_errorbars(meas_type, direction):
    '''Annotate errorbar plot'''
    dir_text = 'Sending from M7 to M4' if direction == 's' else 'Sending from M4 to M7'
    plt.title(dir_text)
    plt.ylim(0)
    plt.grid()
    plt.legend(title='Memory, M7, M4 clk [MHz]')
    unit = 'us' if meas_type == 'latency' else 'MB/s'
    plt.ylabel(f'{meas_type.capitalize()} [{unit}]')
    plt.xlabel('Data size [B]')

def final_size_func_foreach(configs, base_dirs, meas_type, if_model=False):
    '''Draws complete final plot for each config
    
    Args:
        configs: dict of lists, all configs, cartesian product will be used
        base_dirs: list of base directories to use, cartesian product again
        meas_type: 'datarate' or 'latency' type of measurement
        if_model: if prediction of model should be plotted'''
    cmap = mpl.colormaps['tab10'].colors
    model_filename = 'models_long.json'
    model_path = os.path.join(MODELS_PATH, base_dirs[0], model_filename)

    sizes = configs['datasize']
    if if_model:
        model = linear_model.LinearModel(model_path)

    config_list = meas.config_to_config_list(configs)
    grouped_config_list = meas.group_config_except(config_list, ['datasize'])
    for base_dir in base_dirs:
        meas_datas = []
        model_path = os.path.join(MODELS_PATH, base_dir, 
                                    model_filename)
        model.load_params(model_path)
        for i, grouped_config in enumerate(grouped_config_list):
            data, config_list = meas.get_and_calc_meas(grouped_config, 
                                                       meas_type, 
                                                       base_dir)
            data = meas.upper_lower_from_minmax(data)
            meas_datas.append(data)
            if if_model:
                model.set_model_from_config(grouped_config)
                pred = model.get_output_from_config(grouped_config, meas_type)
                model_plot_from_config(pred, grouped_config, cmap[i])
        errorbars(grouped_config_list, sizes, meas_datas, cmap, if_line=(not if_model), base_dir=base_dir)
        cmap = cmap[len(grouped_config_list):]
    setup_errorbars(meas_type, configs['direction'][0])

def main():
    '''Reading in measurements, calculating mean, std then visualizing'''
    cmap = mpl.colormaps['tab10'].colors
    sizes_short = [1] + [16*x for x in range(1, 17)]
    sizes_long = ([1]
              + [16*x for x in range(1, 17)]
              + [512, 1024, 1536]
              + [1024*x for x in range(2, 16)] + [16376])
    meas_configs = {
        'direction': ['r', 's'],
        'clkM7': [480],
        'clkM4': [60],
        'repeat': [256],
        'datasize': sizes_long,
        'mem': ['D1'],
        'cache': ['none', 'i', 'd', 'id'],
    }
    base_dir = 'v0_O0'
    meas_type = 'datarate'

    config_list = meas.config_to_config_list(meas_configs)
    config_groups_list = meas.group_config_except(config_list, ['datasize'])
    for direction in meas_configs['direction']:
        filt_groups = [cfg for cfg in config_groups_list 
                         if cfg['direction'] == direction]
        meas_datas = []
        for filt_config in filt_groups:
            ret, _ = meas.get_and_calc_meas(filt_config, meas_type, base_dir)
            ret = meas.upper_lower_from_minmax(ret)
            meas_datas.append(ret)
        plt.figure()
        # list of configs and list of meas_values
        errorbars(filt_groups, meas_configs['datasize'], meas_datas, cmap)
        setup_errorbars(meas_type, direction)

    # show graph
    plt.show()

if __name__ == '__main__':
    main()
