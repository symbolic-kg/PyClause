import time
import c_clause
from clause.util.utils import get_base_dir




# train = "/home/patrick/Desktop/PyClause/data/fb15k-237/train.txt"
# filter = "/home/patrick/Desktop/PyClause/data/fb15k-237/valid.txt"
# target = "/home/patrick/Desktop/PyClause/data/fb15k-237/test.txt"
# rules = "/home/patrick/Desktop/PyClause/data/fb15k-237/anyburl-rules-c3-3600"
# ranking_file = "/home/patrick/Desktop/PyClause/data/fb15k-237/rankingFile.txt"


train = "./data/wnrr/train.txt"
filter = "./data/wnrr/valid.txt"
target = "./data/wnrr/test.txt"

rules = "./data/wnrr/anyburl-rules-c5-3600"
ranking_file = "./local/rankingFile.txt"


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
    "use_zero_rules": "true",
    "rule_zero_weight":"0.01",
    "use_u_c_rules": "true",
    "use_b_rules": "true",
    "use_u_d_rules": "true",
    "rule_u_d_weight":"0.01",
    "use_u_xxc_rules": "true",
    "use_u_xxd_rules": "true",
    "tie_handling": "frequency",
    "rule_num_unseen": "5",
    
}


loader = c_clause.DataHandler(options)
loader.load_datasets(target, train, filter)
loader.load_rules(rules)








### Calculate exact rule statistics through materialization
#calcStats returns list[num_pred, num_correct_pred]

handler = c_clause.RuleHandler(train)
print("Get predictions")
rules_list = [
    "_hypernym(X,06355894) <= _synset_domain_topic_of(X,A), _synset_domain_topic_of(06355894,A)",
    "_hypernym(X,06355894) <= "
]
rules_list = rules_list
## calculateRulesPredictions: input: list, bool, bool
## list: list of string rule
## first boolean parameter: if predictions will be returned
## second boolean: if stats will be returned
## returns: tuple with tuple[0] being the string predictions, and tuple[1] being the stats
start = time.time()
preds = handler.calcRulesPredictions(rules_list, True, True)
end = time.time()
print(f"All time (+serialization) was {end-start} seconds")

handlerNew = c_clause.RulesHandler()
predsNew = handlerNew.stats_and_predictions(rules_list, loader,  True, True)




#### Calculate a ranking and serialize / use in python
start = time.time()




ranker = c_clause.RankingHandler(options)
ranker.calc_ranking(loader)
ranker.write_ranking(ranking_file, loader)
rankingtime = time.time()
headRanking = ranker.get_ranking("head")
tailRanking = ranker.get_ranking("tail")
serializeTime = time.time()






print(f"all time: {serializeTime-start}")
print(f"ranking time: {rankingtime-start}")
print(f"serialize time: {serializeTime-rankingtime}")


