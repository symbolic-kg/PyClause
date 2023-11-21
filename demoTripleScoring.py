import c_clause
import numpy as np


train = "./data/wnrr/train.txt"
filter = "./data/wnrr/valid.txt"
target = "./data/wnrr/test.txt"
rules = "./data/wnrr/anyburl-rules-c5-3600"

options = {
    # scoring/ranking options
    "aggregation_function": "maxplus",
    "collect_explanations": "true",
    "disc_at_least": "-1",
    "topk":"40000", #complete ranking as we need every candidate
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
loader.load_data(train, filter, target)
loader.load_rules(rules)


scorer = c_clause.PredictionHandler(options)
scorer.score_triples("./data/wnrr/test.txt", loader)

idx_scores = scorer.get_scores(False)
str_scores = scorer.get_scores(True)


ranker = c_clause.RankingHandler(options)
ranker.calc_ranking(loader)

tails = ranker.get_ranking("tail")
heads = ranker.get_ranking("head")

for i in range(len(idx_scores)):
    # same scores; string scores are rounded to 6 decimals in backend
    assert(round(idx_scores[i][3], 6)==float(str_scores[i][3]))
    head = idx_scores[i][0]
    rel = idx_scores[i][1]
    tail = idx_scores[i][2]

    
    # the score must match when calculated in a ranking from queries
    if (idx_scores[i][3]!=0):
        tail_ranking = tails[rel][head]
        tail_ranking = dict(tail_ranking)
        assert(idx_scores[i][3]==tail_ranking[tail])
        
        head_ranking = heads[rel][tail]
        head_ranking = dict(head_ranking)
        assert(idx_scores[i][3]==head_ranking[head])      
        








exit()


entity_names_f = './data/wnrr/entity_strings.txt'
entity_names = {}
with open(entity_names_f, 'r') as file:
    for line in file:
        key, value = line.strip().split('\t')
        entity_names[key] = value

loader.replace_ent_tokens(entity_names)

ranker = c_clause.PredictionHandler()





explanations = scorer.get_explanations(False)


# print(scorer.get_scores(True)[0])



for i in range(len(explanations[0])):
    print("-----------------------------------------------")
    print(f"Target triple: {explanations[0][i]}")
    for j in range(len(explanations[1][i])):
        print("Rule:")
        print(explanations[1][i][j])
        print("Groundings:")
        ctr = 0
        for grounding in explanations[2][i][j]:
            ctr +=1
            if ctr>5:
                break
            print("Next grounding")
            print(grounding)
    


print("debug")

exit()


scorer = c_clause.PredictionHandler(options)
scorer.score_triples([['suborder_manteodea', '_member_meronym', 'mantidae']], loader)
explanations = scorer.get_explanations()
print(explanations[0])
print(explanations[1])
print(explanations[2])


for i in range(len(explanations[0])):
    print("-----------------------------------------------")
    print(f"Target triple: {explanations[0][i]}")
    for j in range(len(explanations[1][i])):
        print("Rule:")
        print(explanations[1][i][j])
        print("Groundings:")
        for grounding in explanations[2][i][j]:
            print(grounding)



for gr in explanations[2][0][0]:
    print(gr)
    print("")


    
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




