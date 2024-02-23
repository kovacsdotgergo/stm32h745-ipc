import os
import itertools

import matplotlib.pyplot as plt
import matplotlib as mpl
import numpy as np

from setup_paths import *
import measurement as meas
import visu_common
import linear_model

def errorbar_3d(clocks, data, ax, label, color):
    ''' 3d plot without figure and annotation
    Args:
        clocks: list of (m7, m4) clocks
        data: ndarray of (mean, min, max) measurements of 
            shape (3, len(clocks))
        ax: subplot ax'''
    data = meas.upper_lower_from_minmax(data)
    data = data.squeeze()
    mean = data[0, :]
    err = data[(1, 2), :]
    m7, m4 = zip(*clocks)
    # plot the data with error bars
    return ax.errorbar(m7, m4, mean, zerr=err, 
                       label=label, fmt='o', color=color)

def setup_ax(ax, direction, meas_type, size):
    '''Sets up all annotation on the 3d plot'''
    ax.set_xlabel('M7 clk [MHz]')
    ax.set_ylabel('M4 clk [MHz]')
    zlabel = 'Datarate [MB/s]' if \
            'datarate' == meas_type else \
            'Latency [us]'
    ax.set_zlabel(zlabel)
    dir_text = 'from M7 to M4' if 's' == direction else 'from M4 to M7'
    ax.set_title(f'{size[0]} B {dir_text}')
    ax.set_xlim([0, 480])
    ax.set_xticks(np.arange(5)*120)
    ax.set_ylim([0, 240])
    ax.set_yticks(np.arange(5)*60)
    ax.set_zlim(0)
    ax.legend()

def model_grid(m7, m4, pred, ax, color, if_cut=False, count=3):
    '''3d plot without figure and annotation
    Grid for the clocks and using it for a wireframe for pred
    Args:
        cut: if the invalid clock pairs should be cut off'''
    m4_grid, m7_grid = np.meshgrid(m4, m7)
    if if_cut:
        # masking invalid clock pairs
        mask = m4_grid < m7_grid
        m4_grid = np.where(mask, m4_grid, np.nan)
        m7_grid = np.where(mask, m7_grid, np.nan)
        pred = np.where(mask, pred, np.nan)
        mask = np.nanargmax(m4_grid, axis=0), np.arange(m4_grid.shape[0])
        m4_edge = m4_grid[mask]
        m7_edge = m7_grid[mask]
        pred_edge = pred[mask]
        plt.plot(m7_edge, m4_edge, pred_edge, color=color, zorder=2,
                linestyle='dashed')
    ax.plot_wireframe(m7_grid, m4_grid, pred, rcount=count, ccount=count,
                      color=color, zorder=2, linestyle='dashed')

def final3d_foreach(meas_configs, base_dir, meas_type, ax, 
                   if_cut=False, linecount=3):
    '''Draw the 3d plots for the given size and mems
    Args:
        cut: boolean if the invalid clocks should be cut off'''
    assert isinstance(meas_configs['mem'], list)
    wire_alpha = 0.6
    cmap = mpl.colormaps['tab10'].colors
    wire_cmap = mpl.colors.to_rgba_array(cmap, wire_alpha)

    model_filename = 'models_long.json'

    model_path = os.path.join(MODELS_PATH, base_dir, model_filename)
    model = linear_model.LinearModel(model_path)
    if meas_type == 'latency':
        ax.view_init(elev=30, azim=60)
    elif meas_type == 'datarate':
        ax.view_init(elev=30, azim=-120)

    meas_config_list = meas.config_to_config_list(meas_configs)
    grouped_config_list = meas.group_config_except(meas_config_list, ['clkM7', 'clkM4'])
    for color_idx, grouped_config in enumerate(grouped_config_list):        
        data, config_list = meas.get_and_calc_meas(grouped_config, 
                                                   meas_type, 
                                                   base_dir)
        clocks = [(x['clkM7'], x['clkM4']) for x in config_list]
        errorbar_3d(clocks, data, ax, 
                    f'{grouped_config["mem"]}_{grouped_config["cache"]}',
                    cmap[color_idx])
        
            # Predictions by the model
        model.set_model_from_config(grouped_config)
        m7, m4, pred = model.get_grid_from_config(grouped_config,
                                                  meas_type)
        model_grid(m7, m4, pred, ax, wire_cmap[color_idx], if_cut=if_cut,
                   count=linecount)
    setup_ax(ax, meas_configs['direction'][0], meas_type, 
             meas_configs['datasize'])

def main():
    '''Reading in measurements, calculating mean, std then visualizing'''
    meas_type = 'latency'
    base_dir = 'v6_O3'
    meas_configs = {
        'direction': ['r', 's'],
        'clkM7': [60, 120, 240, 480],
        'clkM4': [60, 120, 240],
        'repeat': [256],
        'datasize': [256],
        'mem': ['D1', 'D2', 'D3'],
        'cache': ['none'],
    }
    if_cut = False
    model_path = os.path.join(MODELS_PATH, base_dir, 'models_long.json')

    cmap = mpl.colormaps['tab10'].colors
    wire_alpha = 0.6
    wire_cmap = mpl.colors.to_rgba_array(cmap, wire_alpha)

    config_list = meas.config_to_config_list(meas_configs)
    config_groups_list = meas.group_config_except(config_list, 
                                                  ['clkM7', 'clkM4'])
    for direction in meas_configs['direction']:
        ax = plt.figure().add_subplot(111, projection='3d')
        if meas_type == 'latency':
            ax.view_init(elev=30, azim=45)
        elif meas_type == 'datarate':
            ax.view_init(elev=30, azim=-135)

        for clr, (mem, cache) in enumerate(itertools.product(meas_configs['mem'],
                                                             meas_configs['cache'])):
            grouped_config = [cfg for cfg in config_groups_list
                            if cfg['direction'] == direction
                                and cfg['mem'] == mem
                                and cfg['cache'] == cache]
            
            data, data_config_list = meas.get_and_calc_meas(grouped_config[0], meas_type, base_dir)
            clocks = [(x['clkM7'], x['clkM4']) for x in data_config_list]
            errorbar_3d(clocks, data, ax, f'{mem}_{cache}', cmap[clr])

            # Predictions by the model
            model = linear_model.LinearModel(model_path, mem, cache, direction)
            m7, m4, pred = model.get_grid_for_range(clocks, meas_configs['datasize'][0], meas_type)
            model_grid(m7, m4, pred, ax, wire_cmap[clr], if_cut=if_cut)
        setup_ax(ax, direction, meas_type, meas_configs['datasize'])
    # show graph
    plt.show()


if __name__ == '__main__':
    main()