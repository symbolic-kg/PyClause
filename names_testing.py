import c_clause


entity_names_f = './data/wnrr/entity_strings.txt'


train = "./data/wnrr/train.txt"
filter = "./data/wnrr/valid.txt"
target = "./data/wnrr/test.txt"

rules = "./data/wnrr/anyburl-rules-c5-3600"


entity_names = {}
with open(entity_names_f, 'r') as file:
    for line in file:
        key, value = line.strip().split('\t')
        entity_names[key] = value


train = "./data/wnrr/train.txt"
filter = "./data/wnrr/valid.txt"
target = "./data/wnrr/test.txt"

rules = "./data/wnrr/anyburl-rules-c5-3600"


options = {
    # ranking options
    "aggregation_function": "maxplus",
    "num_preselect": "10000000",
    "topk": "100",
    "filter_w_train": "false",
    "filter_w_target": "false",
    "disc_at_least":"100", ## -1 for off, must not be bigger than topk
    # rule options 
    "rule_b_max_branching_factor": "-1",
    "use_zero_rules": "false",
    "rule_zero_weight":"0.01",
    "use_u_c_rules": "true",
    "use_b_rules": "true",
    "use_u_d_rules": "true",
    "rule_u_d_weight":"0.01",
    "use_u_xxc_rules": "true",
    "use_u_xxd_rules": "true",
    "tie_handling": "frequency"
}


qa_handler = c_clause.QAHandler(options)
qa_handler.load_datasets(train, filter)
qa_handler.load_rules(rules)
qa_handler.replace_ent_tokens(entity_names)

answers = qa_handler.answer_queries([("italy", "_has_part")], "head")
answers = answers[0]
for i in range(len(answers)):
    print(answers[i][0], answers[i][1])