
import os
from os import path
from subprocess import CalledProcessError, Popen, PIPE, STDOUT
import sys
import ast

from clause.util.utils import get_ab_dir, join_u


def learn(train_path, time, options, path_rules_output):

    # keep it simple: anyburl log + config is written to the folder 
    # of the specified rule file
    learn_dir = os.path.dirname(path_rules_output)

    if not path.isdir(learn_dir):
        os.mkdir(learn_dir)

    conf_path = join_u(learn_dir, "config-learn.properties")

    learn_config = [
        "PATH_TRAINING = "  + train_path,
        "PATH_OUTPUT   = " + path_rules_output,
        "SNAPSHOTS_AT = " +  str(time),
    ]

    param_list = []
    for param in options:
        if param.startswith("raw."):
            token = param.split(".")
            param_list.append("" + token[1] + " = " + str(options[param]))
    learn_config.extend(param_list)

    
    with open(conf_path, "w") as f:
        f.write("\n".join(learn_config))
    f.close()

    java_options = ast.literal_eval(options.get("java_options"))
    java_options = " ".join(java_options)

    cmd_call = f"java {java_options} -cp {get_ab_dir()} de.unima.ki.anyburl.Learn {conf_path}"
    cmd_call = cmd_call.replace("  ", " ")

    print("-> anyburl call: " + cmd_call)
    with Popen(cmd_call, shell=True, stdout=PIPE, stderr=STDOUT, bufsize=1, universal_newlines=True) as p:
        for line in p.stdout:
            print("-> anyburl call: " + line, end='') # process line here

    if p.returncode != 1:
        print("AnyBURL exited with error, see error msg above.")
        sys.exit(0)
    
    return path_rules_output + "-" + str(time)