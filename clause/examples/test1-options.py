import sys
import os
sys.path.append(os.getcwd())

import yaml

from clause.config.options import Options

if __name__ == '__main__':

    with open('default-config.yaml', 'r') as file:
        default_options = yaml.safe_load(file)

        
    # options = Options()
    # print(options.flat())
