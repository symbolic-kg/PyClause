from c_clause import Loader, RulesHandler
from clause import Options
from clause.util.utils import get_base_dir

# *** Example for rule materialization and stats calculation ***

data = f"{get_base_dir()}/data/wnrr/train.txt"

opts = Options()
opts.set("rules_handler.collect_statistics", True)
opts.set("rules_handler.collect_predictions", True)

loader = Loader(opts.get("loader"))
loader.load_data(data)


rules_list = [
    "_hypernym(X,06355894) <= ",
    "_hypernym(X,06355894) <= _synset_domain_topic_of(X,A), _synset_domain_topic_of(06355894,A)",
]

rh = RulesHandler(options=opts.get("rules_handler"))
rh.calculate_predictions(rules=rules_list, loader=loader)

# for each rule get string predictions
print(rh.get_predictions(as_string=True))

# for each rule get idx predictions
print(rh.get_predictions(as_string=False))

# for each rule get num_predicted num_true_predicted
print("Statistics")
print(rh.get_statistics())

# write a new KG (flat=True removes duplicates)
rh.write_predictions("local/out.jsonl", flat=True, as_string=True)










