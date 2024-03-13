import os
import matplotlib.pyplot as plt
import numpy as np

from setup_paths import *
import measurement
import linear_model
import visu_common
import histogram
import visu_3d
import visu

def if_large_size(size):
    '''Fuction to return sizes for the long size plots'''
    return (size % 64) == 0 or size == 1 or size == 16380

def if_small_size(size):
    '''Function to return sizes for the short size plots'''
    return size <= 256

def main():
    '''Printing and writing out all final plots'''
    # Common variables
    sizes_short = [1] + [16*x for x in range(1, 17)]
    sizes_long = ([1]
                + [16*x for x in range(1, 17)]
                + [512, 1024, 1536]
                + [1024*x for x in range(2, 16)] + [16376])
    sizes_long = ([1]
                + list(filter(lambda x: x%64 == 0, sizes_long))
                + [16376])
    # todo: when the new results are done, then update this file
    
    # base_dir = 'v8_O3'
    # model_path = os.path.join(MODELS_PATH, base_dir, 'models_long.json')
    # mem_regex = r'D[0-9](_idcache_mpu_ncacheable)?'
    # linear_model.print_table(model_path, mem_regex) 
    # todo: print tables   

    # =====================================================================
    # initial plot to show difference between release and debug, long and short latency and datarate
    meas_configs = {
        # direction in loop
        'clkM7': [480],
        'clkM4': [60],
        'repeat': [256],
        # datasize in loop
        'mem': ['D1'],
        'cache': ['none'],
    }
    base_dirs = ['v10_O0', 'v8_O3', 'v12_Osize', 'v13_Ofast']
    meas_type = 'latency'

    # todo
    filename = 'optimization_size.pdf'
    i = 0
    plt.figure(figsize=(10, 9.5), layout='tight')
    for meas_type, sizes in zip(['latency', 'datarate'], 
                            [sizes_short, sizes_long]):
        for direction in ['r', 's']:
            meas_configs['direction'] = direction
            meas_configs['datasize'] = sizes
            ax = plt.subplot(221 + i)
            if meas_type == 'latency':
                plt.ylim(top=180)
            else: # datarate
                plt.ylim(top=4)
            if sizes == sizes_long:
                plt.xticks(np.arange(9)*2048, rotation=12)
            else:
                plt.xticks(np.arange(5)*64)
            visu.final_size_func_foreach(
                meas_configs, base_dirs, meas_type,
                if_model=True, if_mid_checkpoint=False)
            i = i + 1
    out = os.path.join(FIGURES_PATH, filename)
    if not os.path.exists(out):
        plt.savefig(out)

    # =====================================================================
    # size plot clock dependecy 
    clkM7s = [[240, 120, 60], [240]]
    clkM4s = [[60], [240, 120, 60]]
    base_dirs = ['v8_O3']
    filenames = ['clock_m7_size.pdf', 'clock_m4_size.pdf']

    meas_configs = {
        # direction in loop
        # clkM4 in loop
        # clkM7 in loop
        'repeat': [256],
        # datasize in loop
        'mem': ['D2'],
        'cache': ['id'],
    }
    for filename, clkM7, clkM4 in zip(filenames, clkM7s, clkM4s):
        i = 0
        plt.figure(figsize=(10, 9.5), layout='tight')
        for meas_type, sizes in zip(['latency', 'datarate'], 
                                    [sizes_short, sizes_long]):
            for direction in ['r', 's']:
                meas_configs['direction'] = direction
                meas_configs['datasize'] = sizes
                meas_configs['clkM4'] = clkM4
                meas_configs['clkM7'] = clkM7
                ax = plt.subplot(221 + i)
                if filename == filenames[0]: # first figure
                    if meas_type == 'latency':
                        plt.ylim(top=120)
                    else: # datarate
                        plt.ylim(top=4.5)
                else: # second figure
                    if meas_type == 'latency':
                        plt.ylim(top=100)
                    else: # datarate
                        plt.ylim(top=18)
                if sizes == sizes_long:
                    plt.xticks(np.arange(9)*2048, rotation=12)
                else:
                    plt.xticks(np.arange(5)*64)
                visu.final_size_func_foreach(
                    meas_configs, base_dirs, meas_type,
                    if_model=True, if_mid_checkpoint=False)
                i = i + 1
    out = os.path.join(FIGURES_PATH, filename)
    if not os.path.exists(out):
        plt.savefig(out)

    # =====================================================================
    # 3d clocks dependency from checkpoints on each core
    meas_configs = {
        # direction in loop
        'clkM7': [120, 240, 480],
        'clkM4': [60, 120, 240],
        'repeat': [256],
        # datasize in loop
        'mem': ['D2'],
        'cache': ['none'],
    }
    base_dir = 'v8_O3'
    meas_type = 'latency'
    filename = 'seperate_clocks_3d.pdf'

    i = 0
    fig = plt.figure(figsize=(10, 9.5), layout='tight')
    for datasize in [[1], [16376]]:
        for direction in ['r', 's']:
            meas_configs['direction'] = direction
            meas_configs['datasize'] = datasize
            ax = fig.add_subplot(221 + i, projection='3d')
            visu_3d.final3d_foreach(meas_configs, base_dir, meas_type, ax,
                                    if_cut=False, linecount=3, if_meas=False,
                                    if_mid_checkpoints=True)
            if datasize == [1]: # first row
                ax.set_zlim(top=35)
            else: # second row
                ax.set_zlim(top=2500)
            i = i + 1
    out = os.path.join(FIGURES_PATH, filename)
    if not os.path.exists(out):
        plt.savefig(out)

    # =====================================================================
    # 3d clock dependency and differnce between mems
    meas_configs = {
        # direction in loop
        'clkM7': [120, 240, 480],
        'clkM4': [60, 120, 240],
        'repeat': [256],
        # datasize in loop
        'mem': ['D1', 'D2', 'D3'],
        'cache': ['id'],
    }
    base_dir = 'v8_O3'
    filename = 'memories_cache_3d.pdf'

    i = 0
    fig = plt.figure(figsize=(10, 9.5), layout='tight')
    for meas_type, datasize in zip(['latency', 'datarate'], [[1], [16376]]):
        for direction in ['r', 's']:
            meas_configs['direction'] = direction
            meas_configs['datasize'] = datasize
            ax = fig.add_subplot(221 + i, projection='3d')
            visu_3d.final3d_foreach(meas_configs, base_dir, meas_type, ax,
                                    if_cut=False, linecount=3)
            if meas_type == 'latency':
                ax.set_zlim(top=40)
            else: # datarate
                ax.set_zlim(top=18)
            i = i + 1
    out = os.path.join(FIGURES_PATH, filename)
    if not os.path.exists(out):
        plt.savefig(out)

    # =====================================================================
    # difference between the memories plot for function of size 
    meas_configs = {
        # direction in loop
        'clkM7': [480],
        'clkM4': [240],
        'repeat': [256],
        # datasize in loop
        'mem': ['D1', 'D2', 'D3'],
        'cache': ['id'],
    }
    base_dir = 'v8_O3'
    filename = 'mems_size.pdf'

    i = 0
    plt.figure(figsize=(10, 9.5), layout='tight')
    for meas_type, sizes in zip(['latency', 'datarate'], 
                                [sizes_short, sizes_long]):
        for direction in ['r', 's']:
            meas_configs['direction'] = direction
            meas_configs['datasize'] = sizes
            ax = plt.subplot(221 + i)
            if meas_type == 'latency': # top row
                plt.ylim(top=35)
            else: # bottom row
                plt.ylim(top=18)
            if sizes == sizes_long:
                plt.xticks(np.arange(9)*2048, rotation=12)
            else:
                plt.xticks(np.arange(5)*64)
            visu.final_size_func_foreach(
                meas_configs, base_dirs, meas_type,
                if_model=True, if_mid_checkpoint=False)
            i = i + 1
    out = os.path.join(FIGURES_PATH, filename)
    if not os.path.exists(out):
        plt.savefig(out)

    # =====================================================================
    # difference between the memories with cache and mpu
    meas_configs = {
        # direction in loop
        'clkM7': [240],
        'clkM4': [240],
        'repeat': [256],
        # datasize in loop
        'mem': ['D1'],
        'cache': ['none', 'i', 'd', 'id'],
    }
    base_dirs = ['v8_O3']
    filename = 'cache_size.pdf'

    i = 0
    plt.figure(figsize=(10, 9.5), layout='tight')
    for meas_type, sizes in zip(['latency', 'datarate'], 
                                [sizes_short, sizes_long]):
        for direction in ['r', 's']:
            meas_configs['direction'] = direction
            meas_configs['datasize'] = sizes
            ax = plt.subplot(221 + i)
            if meas_type == 'latency': # top row
                plt.ylim(top=50)
            else: # bottom row
                plt.ylim(top=16)
            if sizes == sizes_long:
                plt.xticks(np.arange(9)*2048, rotation=12)
            else:
                plt.xticks(np.arange(5)*64)
            visu.final_size_func_foreach(
                meas_configs, base_dirs, meas_type,
                if_model=True, if_mid_checkpoint=False)
            i = i + 1
    out = os.path.join(FIGURES_PATH, filename)
    if not os.path.exists(out):
        plt.savefig(out)

    # show graph
    plt.show()

if __name__ == '__main__':
    main()
