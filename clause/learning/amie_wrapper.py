
import os
import sys
import ast
from os import path
from subprocess import CalledProcessError, Popen, PIPE, STDOUT

from clause.util.utils import get_amie_dir


def learn(train_path, options, path_rules_output):

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
            v = str(options[param])
            if str(options[param]) == "*flag*": v = ""
            amie_params_options.append("-" + token[1] + " " + v)

    amie_params.extend(amie_params_options)
    amie_params_string = " ".join(amie_params)

    java_options = ast.literal_eval(options.get("java_options"))
    java_options = " ".join(java_options)

    cmd_call = f"java {java_options} -jar {get_amie_dir()} " + amie_params_string + " "
    cmd_call = cmd_call.replace("  ", " ")
    print("-> amie call: " + cmd_call)
    with Popen(cmd_call, shell=True, stdout=PIPE, stderr=STDOUT, bufsize=1, universal_newlines=True) as p:
        for line in p.stdout:
            print("-> amie call: " + line, end='') # process line here

    if p.returncode != 0:
        print("Amie exited with error, see error msg above.")
        sys.exit(0)
    return path_rules_output