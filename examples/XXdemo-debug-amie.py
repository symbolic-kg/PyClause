from clause import Learner, Options
from clause.util.utils import get_base_dir
from c_clause import Loader


# *** Example for rule learning with AnyBURL or Amie  ***

path_train = f"{get_base_dir()}/data/yago3-10/train.txt"
path_rules_output = f"{get_base_dir()}/local/myrules/rules-yago3-10.txt"

# load custom config from file
options = Options(f"{get_base_dir()}/config-my.yaml")

# set "amie" or "anyburl" and define specifc arguments
# AMIE
options.set("learner.mode", "amie")
# we are choosing a parameter setting here, which works well for the KBC scenario
options.set("learner.amie.raw.maxad", 2)
options.set("learner.amie.raw.minc", 0.001)
options.set("learner.amie.raw.minpca", 0.001)
options.set("learner.amie.raw.minhc", 0.001)
options.set("learner.amie.raw.mins", 5)
options.set("learner.amie.raw.const", "*flag*") # special syntax for enforcing -const to be used as flag without value
options.set("learner.amie.raw.maxadc", 2)
options.set("learner.amie.raw.pm", "support") 

# AnyBURL
# options.set("learner.mode", "anyburl")
# options.set("learner.anyburl.time", 30)
# options.set("learner.anyburl.raw.MAX_LENGTH_CYCLIC", 5)

learner = Learner(options=options.get("learner"))
learner.learn_rules(path_data=path_train, path_output=path_rules_output)


loader = Loader(options.get("loader"))
loader.load_data(data=path_train)
loader.load_rules(rules=path_rules_output)

