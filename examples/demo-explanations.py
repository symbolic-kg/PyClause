from c_clause import PredictionHandler, Loader
from clause.util.utils import get_base_dir, read_jsonl
from clause import Options


# *** Example for target triple explanations ***

train = f"{get_base_dir()}/data/wnrr/train.txt"
rules = f"{get_base_dir()}/data/wnrr/anyburl-rules-c5-3600"

# load some custom config
opts = Options(path=f"{get_base_dir()}/config-my.yaml")

opts.set("prediction_handler.collect_explanations", True)
# allow a maximum of two predicting rules for each target
# the 2 with the highest confidences will be taken
opts.set("prediction_handler.num_top_rules", 2)

loader = Loader(opts.get("loader"))
loader.load_data(data=train)
loader.load_rules(rules)

scorer = PredictionHandler(options=opts.get("prediction_handler"))

## alternatively input np.arrays or specify a file path
targets = [
    ["02233096","_member_meronym","02233338"],
    ["08621598","_hypernym","08620061"],
    ["12400489","_hypernym","12651821"],
    ["12400489","_hypernym","12400489"]
]

scorer.calculate_scores(triples=targets, loader=loader)

# obtain the mapping from strings to integer idx's
loader.get_entity_index()
loader.get_relation_index()

idx_explanations = scorer.get_explanations(as_string=False)
str_explanations = scorer.get_explanations(as_string=True)

# retrieve string of rule with idx i as rule_index[i]
# when you use integer format.
rule_index = loader.rule_index()

targets_str, pred_rules_str, groundings_str = str_explanations

for i in range(len(targets_str)):
    print("-----------------------------------------------")
    print(f"Target triple: {targets_str[i]}")
    for j in range(len(pred_rules_str[i])):
        print(f"Rule {j}:")
        print(pred_rules_str[i][j])
        print("Groundings:")
        ctr = 0
        for grounding in groundings_str[i][j]:
            print("Next grounding")
            ## each grounding is a list of triples
            print(grounding)
        print("")

# write to file
scorer.write_explanations(path="local/explanations_str.jsonl", as_string=True)
# load; list of dicts
str_exp = read_jsonl(path="local/explanations_str.jsonl")

# write with idx's
scorer.write_explanations(path="local/explanations_idx.jsonl", as_string=False)
# load; list of dicts
idx_exp = read_jsonl(path="local/explanations_idx.jsonl")
