import os
import numpy as np
import matplotlib.pyplot as plt
import scipy.stats as stats

from setup_paths import *
import measurement
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

def main():
    'Main functions, that draws the histogram of the pilot measurements'
    mems = visu_common.get_mems(PILOT_PATH)
    for mem in mems:
        dir_prefix = os.path.join(PILOT_PATH, mem)

        directions = ['s', 'r']
        for direction in directions:
            clocks = visu_common.get_clocks_in_folder(dir_prefix, prefix=f'meas_{direction}_')
            for m7, m4 in clocks:
                measurement_folder = os.path.join(dir_prefix, f'meas_{direction}_{m7}_{m4}')
                sizes = [16380] #visu_common.get_sizes(measurement_folder)
                raw = measurement.read_meas_from_files(sizes, measurement_folder)

                # for raw_per_size, size in raw, sizes:
                plt.figure()
                dir_txt = 'M7 to M4' if direction=='s' else 'M4 to M7'
                title = f'Size:{sizes[0]} B, M7: {m7} MHz, M4: {m4} MHz, {dir_txt}'
                histogram_intervals(raw, title)
    plt.show()

if __name__ == '__main__':
    main()
