import c_clause
from clause.util.utils import get_base_dir



entity_names_f = f"{get_base_dir()}/data/wnrr/entity_strings.txt"


train = f"{get_base_dir()}/data/wnrr/train.txt"
filter_set = f"{get_base_dir()}/data/wnrr/valid.txt"
target = f"{get_base_dir()}/data/wnrr/test.txt"

rules = f"{get_base_dir()}/data/wnrr/anyburl-rules-c5-3600"


entity_names = {}
with open(entity_names_f, 'r') as file:
    for line in file:
        key, value = line.strip().split('\t')
        entity_names[key] = value



options = {
    # ranking options
    "aggregation_function": "maxplus",
    "num_preselect": "10000000",
    "topk": "100",
    "filter_w_data": "false",
    "filter_w_target": "false",
    "disc_at_least":"100", ## -1 for off, must not be bigger than topk
    # rule options 
    "rule_b_max_branching_factor": "-1",
    "load_zero_rules": "false",
    "rule_zero_weight":"0.01",
    "load_u_c_rules": "true",
    "load_b_rules": "true",
    "load_u_d_rules": "true",
    "rule_u_d_weight":"0.01",
    "load_u_xxc_rules": "true",
    "load_u_xxd_rules": "true",
    "tie_handling": "frequency"
}

loader = c_clause.Loader(options)
loader.load_data(train, filter_set)
loader.load_rules(rules)
loader.replace_ent_strings(entity_names)


qa_handler = c_clause.QAHandler(options)

qa_handler.calculate_answers([("italy", "_has_part")], loader, "head")
answers = qa_handler.get_answers(True)
answers = answers[0]
for i in range(len(answers)):
    print(answers[i][0], answers[i][1])