from clause import TormLearner, Options
from clause import TripleSet
from clause.util.utils import get_base_dir

from c_clause import Loader

# *** Example for rule mining with the experimental module TORM  ***

path_train = f"{get_base_dir()}/data/wnrr/train.txt"
path_rules_output = f"{get_base_dir()}/local/myrules/rules-torm.txt"

# load a triple set from a file and display some meta info about it
triples = TripleSet(path_train)

options = Options()
options.set("torm_learner.mode", "torm")

## some example options
options.set("torm_learner.torm.b.active", False)
options.set("torm_learner.torm.uc.support", 10)
options.set("torm_learner.torm.xx_ud.support", 10)
options.set("torm_learner.torm.xx_uc.support", 10)
options.set("torm_learner.torm.ud.support", 10)
options.set("torm_learner.torm.z.support", 10)

# create a miner object; as rule mining is always against a specific dataset,
# the index of the dataset is used;  we choose all relations in the datasets
# as targets to learn rules for instead of that, a specific list of target
# relations can be set as first argument
# e.g.,  learner = TormLearner(options, [triples.index.to2id["_also_see"]], triples)

learner = TormLearner(options=options, targets=triples.rels, triples=triples)

# mine rules
learner.mine_rules(path_rules_output)

# write the rules that have been mined to a file
learner.rules.write(path_rules_output)

loader = Loader(options.get("loader"))
loader.load_data(data=path_train)
loader.load_rules(rules=path_rules_output)