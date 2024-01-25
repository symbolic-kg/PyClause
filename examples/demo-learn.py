from clause import Learner, Options
from clause.util.utils import get_base_dir
from c_clause import Loader


# *** Example for rule learning with AnyBURL or Amie  ***

path_train = f"{get_base_dir()}/data/wnrr/train.txt"
path_rules_output = f"{get_base_dir()}/local/myrules/rules-wn18rr-anyburl.txt"

# load custom config from file
options = Options(f"{get_base_dir()}/config-my.yaml")

# set "amie" or "anyburl"
options.set("learner.mode", "anyburl")
options.set("learner.anyburl.time", 30)
# raw AnyBURL parameter
options.set("learner.anyburl.raw.MAX_LENGTH_CYCLIC", 5)

learner = Learner(options=options.get("learner"))
learner.learn_rules(path_data=path_train, path_output=path_rules_output)

# directly load the rules into c_clause
loader = Loader(options.get("loader"))
loader.load_data(data=path_train)
loader.load_rules(rules=path_rules_output)

