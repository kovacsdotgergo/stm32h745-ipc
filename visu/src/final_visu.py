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
    # Histogram
    # filename = 'histogram.pdf'
    # mem = 'D3_idcache_mpu_ncacheable_release' #visu_common.get_mems('pilot', pattern=r'D3_.*')
    # dir_prefix = os.path.join(PILOT_PATH, mem)
    # m7, m4 = 480, 240
    # plt.figure(figsize=(10, 9.5), layout='tight')
    # i = 0
    # for direction in ['r', 's']:
    #     for sizes in [[256], [16380]]:
    #         measurement_folder = os.path.join(dir_prefix, f'meas_{direction}_{m7}_{m4}')
    #         #sizes = [16380] #visu_common.get_sizes(measurement_folder)
    #         raw = measurement.read_meas_from_files(sizes, measurement_folder)

    #         plt.subplot(221 + i)
    #         dir_txt = 'M7 to M4' if direction=='s' else 'M4 to M7'
    #         title = f'Size:{sizes[0]} B, M7: {m7} MHz, M4: {m4} MHz, {dir_txt}'
    #         histogram.histogram_intervals(raw, title)
    #         plt.xticks(rotation=12)
    #         i = i + 1
    # out = os.path.join(FIGURES_PATH, filename)
    # if not os.path.exists(out):
    #     plt.savefig(out)
    # todo: histogram if needed

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
    # 3d clock dependency base
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
            if meas_type == 'latency':
                ax.set_ylim(top=40)
            else: # datarate
                ax.set_ylim(top=18)
            visu_3d.final3d_foreach(meas_configs, base_dir, meas_type, ax,
                                    if_cut=False, linecount=3)
            i = i + 1
    out = os.path.join(FIGURES_PATH, filename)
    if not os.path.exists(out):
        plt.savefig(out)

    # # =====================================================================
    # # difference between the memories plot for function of size 
    # configs = [{'mem': 'D1', 'clk': (240, 240)},
    #            {'mem': 'D2', 'clk': (240, 240)},
    #            {'mem': 'D3', 'clk': (240, 240)},]
    # filename = 'mems_size.pdf'
    # i = 0
    # plt.figure(figsize=(10, 9.5), layout='tight')
    # for size_lambda, meas_type in zip([if_small_size, if_large_size],
    #                                   ['latency', 'datarate']):
    #     for direction in ['r', 's']:
    #         ax = plt.subplot(221 + i)
    #         if meas_type == 'latency':
    #             plt.ylim(0, 60)
    #             plt.xticks(np.arange(5)*64)
    #         else:
    #             plt.ylim(0, 17)
    #             plt.xticks(np.arange(9)*2048, rotation=12)
    #         visu.final_size_func_foreach(configs, meas_type, direction,
    #                                     size_lambda=size_lambda, if_model=True)
    #         i = i + 1
    # out = os.path.join(FIGURES_PATH, filename)
    # if not os.path.exists(out):
    #     plt.savefig(out)

    # # =====================================================================
    # # difference between the memories 3d
    # size = 4096
    # mems = visu_common.get_mems(MEASUREMENTS_PATH, r'D[0-9]')
    # filename = 'memories_3d.pdf'
    # i = 0
    # fig = plt.figure(figsize=(10, 9.5), layout='tight')
    # for meas_type in ['latency', 'datarate']:
    #     for direction in ['r', 's']:
    #         ax = fig.add_subplot(221 + i, projection='3d')
    #         if meas_type == 'latency':
    #             ax.set_zlim([0, 1400])
    #         else:
    #             ax.set_zlim([0, 17])
    #         visu_3d.final3d_foreach(
    #                 size, mems, direction, ax, meas_type=meas_type,
    #                 clock_lambda=(lambda m7, m4: m7%120==0 and m4%60==0),
    #                 if_cut=False)
    #         i = i + 1
    # out = os.path.join(FIGURES_PATH, filename)
    # if not os.path.exists(out):
    #     plt.savefig(out)

    # # =====================================================================
    # # difference between the memories with cache and mpu
    # size = 16380
    # mems = visu_common.get_mems(MEASUREMENTS_PATH, r'D[0-9]_idcache_mpu_ncacheable')
    # filename = 'memories_cache_3d.pdf'
    # i = 0
    # fig = plt.figure(figsize=(10, 9.5), layout='tight')
    # for meas_type in ['latency', 'datarate']:
    #     for direction in ['r', 's']:
    #         ax = fig.add_subplot(221 + i, projection='3d')
    #         if meas_type == 'latency':
    #             ax.set_zlim([0, 5000])
    #         else:
    #             ax.set_zlim([0, 25])
    #         visu_3d.final3d_foreach(
    #                 size, mems, direction, ax, meas_type=meas_type,
    #                 clock_lambda=(lambda m7, m4: m7%120==0 and m4%60==0),
    #                 if_cut=False)
    #         i = i + 1
    # out = os.path.join(FIGURES_PATH, filename)
    # if not os.path.exists(out):
    #     plt.savefig(out)

    # # =====================================================================
    # # for each memory the difference between all the options (2d only)
    # clks = (480, 240)
    # configs = [{'mem': 'D1_idcache_mpu_ncacheable', 'clk': clks},
    #            {'mem': 'D2_idcache_mpu_ncacheable', 'clk': clks},
    #            {'mem': 'D3_idcache_mpu_ncacheable', 'clk': clks},
    #            {'mem': 'D1', 'clk': clks},
    #            {'mem': 'D2', 'clk': clks},
    #            {'mem': 'D3', 'clk': clks},]
    # filename = 'all_mems_size.pdf'
    # i = 0
    # plt.figure(figsize=(10, 9.5), layout='tight')
    # for size_lambda, meas_type in zip([lambda size: size<=512, if_large_size],
    #                                   ['latency', 'datarate']):
    #     for direction in ['r', 's']:
    #         ax = plt.subplot(221 + i)
    #         if meas_type == 'latency':
    #             plt.ylim(0, 80)
    #             plt.xticks(np.arange(9)*64)
    #         else:
    #             plt.ylim(0, 27)
    #             plt.xticks(np.arange(9)*2048, rotation=12)
    #         visu.final_size_func_foreach(configs, meas_type, direction,
    #                                     size_lambda=size_lambda, if_model=True)
    #         i = i + 1
    # out = os.path.join(FIGURES_PATH, filename)
    # if not os.path.exists(out):
    #     plt.savefig(out)

    # show graph
    plt.show()

if __name__ == '__main__':
    main()
