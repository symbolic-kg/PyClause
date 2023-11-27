import sys
import os
sys.path.append(os.getcwd())
import multiprocessing

from clause.util.utils import get_amie_dir, get_base_dir, join_u

from os import path
from subprocess import CalledProcessError, Popen, PIPE



def learn(train_path, params, path_rules_output = None):

    base_dir = get_base_dir()
    learn_dir = join_u(base_dir, join_u("local", "amie-learn"))

    if not path.isdir(learn_dir):
        os.mkdir(learn_dir)

    if path_rules_output == None:
        rule_path = join_u(learn_dir, "amie-rules")
    else:
        rule_path = path_rules_output


    cpu_count = multiprocessing.cpu_count()
    cpu_count = 1 if cpu_count <= 1 else cpu_count - 1


    with Popen(f"java -jar {get_amie_dir()}", stdout=PIPE, bufsize=1, universal_newlines=True) as p:
        for line in p.stdout:
            print("-> amie call: " + line, end='') # process line here

    if p.returncode != 1:
        raise CalledProcessError(p.returncode, p.args)
    
    return rule_path + "-" + str(time)


if __name__ == '__main__':
    base_dir = get_base_dir()
    train_path = join_u(base_dir, join_u("data", "wnrr", "train.txt"))
    learn(train_path, 30, 3, 5, 0.001)
