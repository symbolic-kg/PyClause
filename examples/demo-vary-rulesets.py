from c_clause import RankingHandler, Loader
from clause.util.utils import get_base_dir, read_jsonl
from clause import Options
import copy

# ** Example for performing rule application with different rulesets with the same loader that loaded one global ruleset **

train = f"{get_base_dir()}/data/wnrr/train.txt"
# when loading from disk set to "" to not use additional filter
filter_set = f"{get_base_dir()}/data/wnrr/valid.txt"
target = f"{get_base_dir()}/data/wnrr/test.txt"

rules = f"{get_base_dir()}/data/wnrr/anyburl-rules-c5-3600"

out = f"{get_base_dir()}/local/ranking-file.txt"

options = Options()
loader = Loader(options=options.get("loader"))
loader.load_data(data=train, filter=filter_set, target=target)

# load ALL rules
loader.load_rules(rules=rules)
ranker = RankingHandler(options=options.get("ranking_handler"))

# set usage of all rule types to false
for opt in ["load_b_rules", "load_u_c_rules", "load_u_d_rules", "load_u_xxd_rules", "load_u_xxc_rules"]:
    options.set("loader." + opt, False)
opts_b = copy.deepcopy(options)
opts_c = copy.deepcopy(options)

opts_b.set("loader.load_b_rules", True)
opts_c.set("loader.load_u_c_rules", True)

# calculate one ranking with only b-rules and one with c-rules
for exp_opts in [opts_b, opts_c]:
    loader.set_options(exp_opts.get("loader"))
    # will update the rules used for application
    loader.update_rules()
    ranker.calculate_ranking(loader=loader)
    # do something with it

# go back to original full ruleset
loader.set_options(Options().get("loader"))
loader.update_rules()
# calculate ranking with all rules
ranker.calculate_ranking(loader=loader)
