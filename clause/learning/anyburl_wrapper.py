import sys
import os
sys.path.append(os.getcwd())
import multiprocessing

from clause.util.utils import get_ab_dir, get_base_dir, join_u

from os import path
from subprocess import CalledProcessError, Popen, PIPE



def learn(train_path, time, options, all_rules, path_rules_output = None):

    base_dir = get_base_dir()
    learn_dir = join_u(base_dir, join_u("local", "anyburl-learn"))

    if not path.isdir(learn_dir):
        os.mkdir(learn_dir)

    if path_rules_output == None:
        rule_path = join_u(learn_dir, "anyburl-rules")
    else:
        rule_path = path_rules_output

    conf_path = join_u(learn_dir, "config-learn.properties")

    cpu_count = multiprocessing.cpu_count()
    # cpu_count = cpu_count - 1
    cpu_count = 1 if cpu_count <= 1 else cpu_count - 1

    learn_config = [
        "PATH_TRAINING = "  + train_path,
        "PATH_OUTPUT   = " + rule_path,
        "SNAPSHOTS_AT = " +  str(time),
        "WORKER_THREADS = " + str(cpu_count),
        ]
    if all_rules == False:
        learn_config.extend([
            "ZERO_RULES_ACTIVE = false",
            "MAX_LENGTH_ACYCLIC = 0",
            "MAX_LENGTH_GROUNDED_CYCLIC = 0",
            "EXCLUDE_AC2_RULES = true",
        ])

    param_list = []
    for param in options:
        if param.startswith("raw."):
            token = param.split(".")
            param_list.append("" + token[1] + " = " + str(options[param]))
    learn_config.extend(param_list)

    
    with open(conf_path, "w") as f:
        f.write("\n".join(learn_config))
    f.close()


    with Popen(f"java -cp {get_ab_dir()} de.unima.ki.anyburl.Learn {conf_path}", shell=True, stdout=PIPE, bufsize=1, universal_newlines=True) as p:
        for line in p.stdout:
            print("-> anyburl call: " + line, end='') # process line here

    if p.returncode != 1:
        raise CalledProcessError(p.returncode, p.args)
    
    return rule_path + "-" + str(time)


if __name__ == '__main__':
    base_dir = get_base_dir()
    train_path = join_u(base_dir, join_u("data", "wnrr", "train.txt"))
    learn(train_path, 30, 3, 5, 0.001)
