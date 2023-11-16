import sys
import os
sys.path.append(os.getcwd())

from clause.learning.miner import Miner
from clause.data.triples import TripleSet

from clause.config.options import Options





if __name__ == '__main__':

    path_train = "data/wnrr/train.txt"
    path_valid = "data/wnrr/valid.txt"
    path_test  = "data/wnrr/test.txt"

    path_rules_output = "local/rules-wn18rr-torm-b1-special"


    # load a triple set from a file and display some meta info about it
    triples = TripleSet(path_train)

    # create a torm object to learn rules, as rule mining is always against a specific dataset, the index of the dataset is used
    # we choose all relations in the datasets as targets to learn rules for
    # instead of that, a specific list of target relations can be determined as first argument

    options = Options()

    options.set("learning.mode", "torm")

    # options.set("learning.anyburl.b_length", 1)
    # options.set("learning.anyburl.time", 30)

    options.set("learning.torm.b.length", 1)
    options.set("learning.torm.uc.support", 2)
    options.set("learning.torm.xx_ud.active", False)
    options.set("learning.torm.xx_uc.active", False)
    options.set("learning.torm.ud.active", False)
    options.set("learning.torm.z.active", False)



    miner = Miner(options, triples.rels, triples)
    # miner= Miner([triples.index.to2id["_also_see"]], triples)



    # mine rules, what type of rules are mined is determined in config.py
    # 
    miner.mine_rules(path_rules_output)

    # write the rules that have been mined to a file
    # miner.rules.write(path_rules_output)
    # use outputformat of NanyTorm, which differs only with respect to xx rules
    # miner.rules.write(path_rules_output, "PyClause")
