import sys
import os
sys.path.append(os.getcwd())
import multiprocessing

from clause.util.utils import get_amie_dir, get_base_dir, join_u

from os import path
from subprocess import CalledProcessError, Popen, PIPE



def learn(train_path, options, path_rules_output):

    base_dir = get_base_dir()
    learn_dir = join_u(base_dir, join_u("local", "amie-learn"))

    if not path.isdir(learn_dir):
        os.mkdir(learn_dir)

    rule_path = path_rules_output

    cpu_count = multiprocessing.cpu_count()
    cpu_count = 1 if cpu_count <= 1 else cpu_count - 1

    out_rule_writer = open(rule_path, "w")

    rule_counter = 0

    param_list = []
    for param in options:
        if param.startswith("raw."):
            token = param.split(".")
            param_list.append("-" + token[1] + " " + str(options[param]))

    param_string = ' '.join(param_list)
    with Popen(f"java -jar {get_amie_dir()} {train_path} " + param_string, stdout=PIPE, bufsize=1, universal_newlines=True) as p:
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
    
    return rule_path


if __name__ == '__main__':
    base_dir = get_base_dir()
    train_path = join_u(base_dir, join_u("data", "wnrr", "train.txt"))
    learn(train_path)
