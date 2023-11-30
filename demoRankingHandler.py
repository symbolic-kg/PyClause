import time
import c_clause
from clause.util.utils import get_base_dir







train = "./data/wnrr/train.txt"
filter = "./data/wnrr/valid.txt"
target = "./data/wnrr/test.txt"

train = "/home/patrick/Desktop/PyClause/data/fb15k-237/train.txt"
filter = "/home/patrick/Desktop/PyClause/data/fb15k-237/valid.txt"
target = "/home/patrick/Desktop/PyClause/data/fb15k-237/test.txt"
rules = "/home/patrick/Desktop/PyClause/data/fb15k-237/anyburl-rules-c3-3600"
ranking_file = "/home/patrick/Desktop/PyClause/data/fb15k-237/rankingFile.txt"

ranking_file = "./local/rankingFile.txt"


options = {
    # ranking options
    "aggregation_function": "maxplus",
    "num_preselect": "10000000",
    "topk": "100",
    "filter_w_train": "true",
    "filter_w_target": "true",
    "disc_at_least":"10", ## -1 for off, must not be bigger than topk
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
    "num_threads": "-1" 
}

#### Calculate a ranking and serialize / use in python
start = time.time()
loader = c_clause.DataHandler(options)
loader.load_data(train, filter, target)
loader.load_rules(rules)


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


