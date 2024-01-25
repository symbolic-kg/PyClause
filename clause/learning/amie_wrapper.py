import sys
import os

import multiprocessing

from clause.util.utils import get_amie_dir, get_base_dir, join_u

from os import path
from subprocess import CalledProcessError, Popen, PIPE


def learn(train_path, options, path_rules_output):

    # keep it simple: anyburl log + config is written to the folder 
    # of the specified rule file
    learn_dir = os.path.dirname(path_rules_output)

    if not path.isdir(learn_dir):
        os.mkdir(learn_dir)

    amie_params = [
        train_path,
        "-ofile " + path_rules_output,
    ]

    amie_params_options = []
    for param in options:
        if param.startswith("raw."):
            token = param.split(".")
            amie_params_options.append("-" + token[1] + " " + str(options[param]))

    amie_params.extend(amie_params_options)
    amie_params_string = " ".join(amie_params)

    cmd_call = f"java -jar {get_amie_dir()} " + amie_params_string + " -const "
    print("-> amie call: " + cmd_call)
    with Popen(cmd_call, shell=True, stdout=PIPE, bufsize=1, universal_newlines=True) as p:
        for line in p.stdout:
            print("-> amie call: " + line, end='') # process line here

    if p.returncode != 0:
        raise CalledProcessError(p.returncode, p.args)
    
    return path_rules_output