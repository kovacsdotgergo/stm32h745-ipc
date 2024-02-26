import os
import regex

def get_clocks_in_folder(folder_path, prefix='meas_s_',
                         clock_lambda=lambda m7, m4: True):
    '''Returns the clocks in folder, matching the given prefix, lambda can
        be used to filter resulting clks'''
    clocks = []
    pattern = regex.compile(pattern=prefix + r'([0-9]+)_([0-9]+)')
    for directory in os.listdir(folder_path):
        if os.path.isdir(os.path.join(folder_path, directory)):
            match = pattern.match(directory)
            if match and clock_lambda(int(match[1]), int(match[2])):
                clocks.append((int(match[1]), int(match[2])))
    return clocks

def get_sizes(folder_path, pattern=r'meas([0-9]+).log',
              size_lambda=lambda size: True):
    '''Returns the sizes in the folder, mathces for the pattern, lambda can
        be used to filter'''
    sizes = []
    compiled_pattern = regex.compile(pattern=pattern)
    for file in os.listdir(folder_path):
        if os.path.isfile(os.path.join(folder_path, file)):
            match = compiled_pattern.match(file)
            if match and size_lambda(int(match[1])):
                sizes.append(int(match[1]))
    return sizes

def get_mems(folder_path, pattern=r'D[0-9]+.*'):
    '''Return the available mems in the folder, which matches the pattern'''
    mems = []
    compiled_pattern = regex.compile(pattern=pattern)
    for file in os.listdir(folder_path):
        if os.path.isdir(os.path.join(folder_path, file)):
            match = compiled_pattern.fullmatch(file)
            if match:
                mems.append(file)
    return mems
