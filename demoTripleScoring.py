import c_clause
import numpy as np


train = "./data/wnrr/train.txt"
filter = "./data/wnrr/valid.txt"
target = "./data/wnrr/test.txt"

rules = "./data/wnrr/anyburl-rules-c5-3600"


entity_names_f = './data/wnrr/entity_strings.txt'
entity_names = {}
with open(entity_names_f, 'r') as file:
    for line in file:
        key, value = line.strip().split('\t')
        entity_names[key] = value



options = {
    # scoring options
    "aggregation_function": "maxplus",
    "collect_explanations": "true",
    "num_top_rules":"1",
    # rule options 
    "rule_b_max_branching_factor": "-1",
    "use_zero_rules": "false",
    "rule_zero_weight":"0.01",
    "use_u_c_rules": "true",
    "use_b_rules": "true",
    "use_u_d_rules": "true",
    "rule_u_d_weight":"0.01",
    "use_u_xxc_rules": "false",
    "use_u_xxd_rules": "false",
}

loader = c_clause.DataHandler(options)

# okay loading like this is wrong, but it should not segm fault me
#loader.load_datasets(target, filter, train)


loader.load_datasets(target, train, filter)




loader.load_rules(rules)
scorer = c_clause.PredictionHandler(options)
scorer.score_triples("./data/wnrr/test.txt", loader)
loader.replace_ent_tokens(entity_names)
explanations = scorer.get_explanations()



for i in range(len(explanations[0])):
    print("-----------------------------------------------")
    print(f"Target triple: {explanations[0][i]}")
    for j in range(len(explanations[1][i])):
        print("Rule:")
        print(explanations[1][i][j])
        print("Groundings:")
        for grounding in explanations[2][i][j]:
            print(grounding)
    
#scorer.score_triples([("07554856",	"_hypernym", 	"07553301")], loader)
#
#explanations = scorer.get_explanations()
#scorer.get_scores(True)
#print(explanations)

#print(scorer.get_scores(True))





# scorer = c_clause.PredictionHandler(options)
# scorer.score_triples([("07554856",	"_hypernym", 	"07553301")], loader)
# print(scorer.get_scores(False))
# print(scorer.get_scores(True))


# loader2 = c_clause.DataHandler(options)
# loader2.load_datasets(target, train, filter)
# loader2.load_rules(rules)
# loader.replace_ent_tokens(entity_names)




# print(scorer.get_scores(False))
# print(scorer.get_scores(True))




#loader2.replace_ent_tokens(entity_names)








#scorer.score_triples("./data/wnrr/test.txt", loader2)

#arr = scorer.get_scores(True)

# for i in range(len(arr)):
#     print(arr[i])



## queries = [("12184337","_hypernym"), ("12184337","_verb_group")]




