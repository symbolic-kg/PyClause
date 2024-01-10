from c_clause import PredictionHandler, Loader
from clause.util.utils import get_base_dir, read_jsonl

import numpy as np

train = f"{get_base_dir()}/data/wnrr/train.txt"
filter = f"{get_base_dir()}/data/wnrr/valid.txt"
target = f"{get_base_dir()}/data/wnrr/test.txt"

rules = f"{get_base_dir()}/data/wnrr/anyburl-rules-c5-3600"

num_top_rules = 2

options = {
        # scoring/ranking options
        "aggregation_function": "maxplus",
        "collect_explanations": "true", #set true to track groundings and rules; can be obtained with scorer.get_explanation(bool)
        "num_top_rules": str(num_top_rules), #stop scoring a triple after it was predicted by the higehst num_top_rules;
                                             #score will not be affected under maxplus aggregation, but number of rules/groundings
                                             # when tracked will be   
        # rule options 
        "rule_b_max_branching_factor": "-1",
        "load_zero_rules": "false",
        "rule_zero_weight":"0.01",
        "load_u_c_rules": "true",
        "load_b_rules": "true",
        "load_u_d_rules": "true",
        "rule_u_d_weight":"0.01",
        "load_u_xxc_rules": "false",
        "load_u_xxd_rules": "false",
}

loader = Loader(options)
loader.load_data(data=train, filter=filter, target=target)
loader.load_rules(rules)


scorer = PredictionHandler(options=options)
## you can also input np.array with idx's or list of string triples
triples = [["02233096","_member_meronym","02233338"], ["08621598","_hypernym","08620061"], ["12400489","_hypernym","12651821"] ]

scorer.calculate_scores(triples=triples, loader=loader)

## false --> idx's are returned (set index if you want your own)
idx_scores = scorer.get_scores(as_string=False)
## true --> strings are returned
str_scores = scorer.get_scores(as_string=True)

## true,false as above
## explanation is a tuple with 3 elements
## 0: list of target (input) triples
## 1: list of list of rules (at position i, a list of rules that predicted target i)
## 2: list of list of list of groundings (at position [i][j] a list of groundings that ground rule j predicting target i)
idx_explanations = scorer.get_explanations(as_string=False)
str_explanations = scorer.get_explanations(as_string=True)

# list of triples 
targets = idx_explanations[0]
# rules[i] is a list of rules that predict triples targets[i]
rules = idx_explanations[1]
# groundings[i][j] is a list of groundings for rule j of target i
# every grounding itself is a list of triples; every triple itself is a list of strings/tuples
groundings = idx_explanations[2]


explanations = str_explanations
## list of string rules rule_idx[i] gives back rule string of rule i
rule_idx = loader.rule_index()

for i in range(len(explanations[0])):
    print("-----------------------------------------------")
    print(f"Target triple: {explanations[0][i]}")
    for j in range(len(explanations[1][i])):
        print("Rule:")
        print(explanations[1][i][j])
        print("Groundings:")
        ctr = 0
        for grounding in explanations[2][i][j]:
            print("Next grounding")
            ## each grounding is a list of triples where a triple is a list
            print(grounding)

scorer.write_explanations(path="local/groundings_str.jsonl", as_string=True)

#list of dicts
str_exp = read_jsonl("local/groundings_str.jsonl")

scorer.write_explanations(path="local/groundings_idx.jsonl", as_string=False)

# list of dicts
idx_exp = read_jsonl("local/groundings_idx.jsonl")
