import os
import numpy as np
import matplotlib.pyplot as plt
import scipy.stats as stats

from setup_paths import *
import measurement as meas
import visu_common

def histogram(raw_meas, title):
    'Draws a histogram of the input raw measurement data'
    plt.title(title)
    plt.xlabel('Latency in # of clk')
    plt.ylabel('Count')
    bins = np.arange(raw_meas.min(), raw_meas.max() + 2) - 0.5
    raw_meas = raw_meas.squeeze()
    plt.hist(raw_meas, bins=bins, log=True, align='mid', label='measured data')
    plt.grid()
    plt.legend(loc='upper right')

def histogram_intervals(raw_meas, title, std_center=False):
    'Histogram with std and confidence interval for the chosen sample size'
    raw_meas = np.array(raw_meas)

    mean = np.mean(raw_meas)
    std = np.std(raw_meas)
    conf_int = stats.norm.interval(0.95, loc=mean, scale=std/np.sqrt(1024))
    plt.axvline(mean, color='red', linestyle='-', label='Mean')
    plt.axvline(mean - std, color='green', linestyle='--', label='Mean ± Std')
    plt.axvline(mean + std, color='green', linestyle='--')
    plt.axvline(conf_int[0], color='purple', linestyle='-.', label='95% CI for 1024 sample')
    plt.axvline(conf_int[1], color='purple', linestyle='-.')
    d = 5
    if std_center:
        raw_meas = raw_meas[(mean-d*std < raw_meas) 
                            & (raw_meas < mean+d*std)] # removing outlier
    histogram(raw_meas, title)
    if std_center:
        plt.xlim(mean-d*std, mean+d*std)
    plt.ylim(5e-1, 1e5)

def time_progression(raw_meas, title):
    '''Plots the measured values as a function of the indices'''
    plt.title(title)
    plt.ylabel('Latency in # of clks')
    plt.xlabel('Meas index')
    plt.plot(np.arange(len(raw_meas)), raw_meas)
    plt.ylim(raw_meas.min(), raw_meas.max())
    plt.grid()

def main():
    'Main functions, that draws the histogram of the pilot measurements'
    base_dir = 'pilot'
    meas_configs = {
        'direction': ['r', 's'],
        'clkM7': [60, 480],
        'clkM4': [60],
        'repeat': [8192],
        'datasize': [16376],
        'mem': ['D1', 'D2', 'D3'],
        'cache': ['none', 'id'],
    }

    raw_list, _, conf_list = meas.read_meas_from_files(meas_configs,
                                                       base_dir)

    for raw, conf in zip(raw_list, conf_list):
        plt.figure()
        dir_txt = 'M7 to M4' if conf['direction']=='s' else 'M4 to M7'
        title = (f'Size:{conf["datasize"]} B, '
                f'M7: {conf["clkM7"]} MHz, '
                f'M4: {conf["clkM4"]} MHz, '
                f'{dir_txt}, '
                f'{conf["mem"]}_{conf["cache"]}')
        histogram_intervals(raw, title)
        plt.figure()
        time_progression(raw, title)
    plt.show()

if __name__ == '__main__':
    main()
