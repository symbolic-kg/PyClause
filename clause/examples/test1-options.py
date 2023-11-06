import sys
import os
sys.path.append(os.getcwd())

from clause.config.options import Options

if __name__ == '__main__':

    options = Options()
    print(options.flat())
