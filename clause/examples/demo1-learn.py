import sys
import os
sys.path.append(os.getcwd())

from clause.learning.complete.torm import Torm
from clause.data.triples import TripleSet





if __name__ == '__main__':

    path_train = "data/wnrr/train.txt"
    path_valid = "data/wnrr/valid.txt"
    path_test  = "data/wnrr/test.txt"

    path_rules_output = "local/rules-nanytorm-wn18rr"


    # load a triple set from a file and display some meta info about it
    triples = TripleSet(path_train)

    # create a torm object to learn rules, as rule mining is always against a specific dataset, the index of the dataset is used
    # we choose all relations in the datasets as targets to learn rules for
    # instead of that, a specific list of target relations can be determined as first argument
    torm = Torm(triples.rels, triples)
    # torm = Torm([triples.index.to2id["_also_see"]], triples)

    # mine rules, what type of rules are mined is determined in config.py
    torm.mine_rules()

    # write the rules that have been mined to a file
    torm.rules.write(path_rules_output)
    # use outputformat of NanyTorm, which differs only with respect to xx rules
    # torm.rules.write(path_rules_output, "NanyTORM")
