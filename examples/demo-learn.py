from clause import Learner, Options
from clause.util.utils import get_base_dir
from c_clause import Loader


# *** Example for rule learning with AnyBURL or Amie  ***

path_train = f"{get_base_dir()}/data/wnrr/train.txt"
path_rules_output = f"{get_base_dir()}/local/myrules/rules-wn18rr.txt"

# load custom config from file
options = Options(f"{get_base_dir()}/config-my.yaml")

# set "amie" or "anyburl" and define specifc arguments

# AMIE
# due to these params AMIE is w.r.t language bias and output format compatibel to AnyBURL
options.set("learner.amie.raw.bias", "amie.mining.assistant.pyclause.AnyBurlMiningAssistant") # BETTER NOT CHANGE
options.set("learner.amie.raw.ofmt", "anyburl") # BETTER NOT CHANGE
# these are examples of some other changes, which are fine 
options.set("learner.mode", "amie")
options.set("learner.amie.raw.maxad", 2)

# AnyBURL
# options.set("learner.mode", "anyburl")
# options.set("learner.anyburl.time", 30)
# options.set("learner.anyburl.raw.MAX_LENGTH_CYCLIC", 5)



learner = Learner(options=options.get("learner"))
learner.learn_rules(path_data=path_train, path_output=path_rules_output)

# directly load the rules into c_clause
loader = Loader(options.get("loader"))
loader.load_data(data=path_train)
loader.load_rules(rules=path_rules_output)

