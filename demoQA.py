import c_clause


train = "./data/wnrr/train.txt"
filter = "./data/wnrr/valid.txt"
target = "./data/wnrr/test.txt"

rules = "./data/wnrr/anyburl-rules-c5-3600"


options = {
    # ranking options
    "aggregation_function": "maxplus",
    "num_preselect": "10000000",
    "topk": "100",
    "filter_w_train": "true",
    "filter_w_target": "true",
    "disc_at_least":"100", ## -1 for off, must not be bigger than topk
    # rule options 
    "rule_b_max_branching_factor": "-1",
    "use_zero_rules": "false",
    "rule_zero_weight":"0.01",
    "use_u_c_rules": "true",
    "use_b_rules": "false",
    "use_u_d_rules": "false",
    "rule_u_d_weight":"0.01",
    "use_u_xxc_rules": "false",
    "use_u_xxd_rules": "false",
    "tie_handling": "frequency"
}


qa = c_clause.QAHandler(options)
qa.load_datasets(train, filter)
qa.load_rules(rules)
#answers = qa.answer_queries([(4,5), (0,1)], "tail")
#print(answers)



answers = qa.answer_queries([("10341660xasdasd","_instance_hypernym")], "tail")
print(answers)
print("hallo")


