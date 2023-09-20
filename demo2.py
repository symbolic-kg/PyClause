import time
import c_clause




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






### Calculate exact rule statistics through materialization
#calcStats returns list[num_pred, num_correct_pred]

handler = c_clause.RuleHandler(train)
print(handler.calcStats("_has_part(X,Y) <= _has_part(X,A), _member_of_domain_region(A,B), _member_of_domain_region(Y,B)"))
print(handler.calcStats("_hypernym(X,06355894) <= _synset_domain_topic_of(X,A), _synset_domain_topic_of(06355894,A)"))

testHandler = c_clause.RuleHandler(target)
print(testHandler.calcStats("_hypernym(X,06355894) <= _synset_domain_topic_of(X,A), _synset_domain_topic_of(06355894,A)"))

print(handler.calcStats("_hypernym(X,06355894) <= "))



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




#### Calculate a ranking and serialize / use in python
start = time.time()
options = {
    # ranking options
    "aggregation_function": "maxplus",
    "num_preselect": "10000000",
    "topk": "100",
    "filter_w_train": "true",
    "filter_w_target": "true",
    "disc_at_least":"10", ## -1 for off
    # rule options 
    "rule_zero_weight":"0.01",
    "rule_b_max_branching_factor": "-1",
    "use_zero_rules": "False",
    "use_u_c_rules": "False",
    "use_b_rules": "True",
}

ranker = c_clause.RankingHandler()
ranker.calculateRanking(target, train, filter, rules, ranking_file, options)


rankingtime = time.time()
headRanking = ranker.getRanking("head")
tailRanking = ranker.getRanking("tail")
serializeTime = time.time()



print(f"all time: {serializeTime-start}")
print(f"ranking time: {rankingtime-start}")
print(f"serialize time: {serializeTime-rankingtime}")


