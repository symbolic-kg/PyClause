import sys
import os

import multiprocessing

from clause.util.utils import get_amie_dir, get_base_dir, join_u

from os import path
from subprocess import CalledProcessError, Popen, PIPE


def learn(train_path, options, path_rules_output):

    learn_dir = os.path.dirname(path_rules_output)

    if not path.isdir(learn_dir):
        os.mkdir(learn_dir)

    out_rule_writer = open(path_rules_output, "w")

    rule_counter = 0

    param_list = []
    for param in options:
        if param.startswith("raw."):
            token = param.split(".")
            param_list.append("-" + token[1] + " " + str(options[param]))

    param_string = ' '.join(param_list)
    with Popen(f"java -jar {get_amie_dir()} {train_path} " + param_string, stdout=PIPE, bufsize=1, universal_newlines=True) as p:
        ## TODO let AMIE write output file
        for line in p.stdout:
            if line.startswith("?"):
                rule_counter = rule_counter +1
                out_rule_writer.write(line)
                if rule_counter  % 1000 == 0:
                    print("-> ... amie mined " + str(rule_counter) + " rules ...")
            else:
                print("-> amie call: " + line, end='') # process line here

    out_rule_writer.close()
    if p.returncode != 1 and p.returncode != 0:
        raise CalledProcessError(p.returncode, p.args)
    
    return path_rules_output
