import time
import c_clause
from clause.util.utils import get_base_dir, list_from_idx_file
import numpy as np


### Setting:  Calculate a ranking and send it to python or write it to disk

train = "./data/wnrr/train.txt"
rules = "./data/wnrr/anyburl-rules-c5-3600"

entity_idx = "./data/wnrr/entity_ids.del"
relation_idx = "./data/wnrr/relation_ids.del"

output_ranking = "/local/output_ranking.txt"




options = {
    # apply options
    "num_preselect": "10",  ## equal to topk means we immediately stop rule application when we have topk cands (max-aggregation)
    "topk": "10",
    "tie_handling": "random",
    "filter_w_train": "false",
    "filter_w_target": "false",
    "disc_at_least":"-1", ## -1
    "num_threads": "-1", # -1 uses all threads, use AS MUCH AS POSSIBLE
    "num_unseen": "5",
    # rule options 
    "load_u_c_rules": "true",
    # shut off all other rules
    "load_zero_rules": "false",
    "load_b_rules": "false",
    "load_u_d_rules": "false",
    "load_u_xxc_rules": "false",
    "load_u_xxd_rules": "false",
}

# data/rules handler
loader = c_clause.DataHandler(options)
# ensure the backend uses our idx->str maps for entities and relations
# given we want to insert arrays and not strings later
ent_idx = list_from_idx_file(entity_idx)
rel_idx = list_from_idx_file(relation_idx)

loader.set_entity_index(ent_idx)
loader.set_relation_index(rel_idx)

# loading from string data is fine as we have set the index
# takes 5-7 minutes for wd5m on server
# here we load loadData(train, valid, test)
# for efficiency the ranking handler does not take any data input
# and our "test" set is train
start = time.time()
loader.load_data(train, "", train)
loader.load_rules(rules)


# Ranking handler
ranker = c_clause.RankingHandler(options)

# Calculate ranking, the data is cached in the ranker (backend)
ranker.calculate_ranking(loader)
ranker.write_ranking(output_ranking, loader)
rankingtime = time.time()



# send head and tail ranking to frontend

# headRanking: the ranking of heads for tail queries
# tailRanking: the ranking of tails for head queries
#headTailRanking is dict[dict[list[tuple[2]]]
# for a head query (?, rel, tail)
#headRanking[rel][tail]
headRanking = ranker.get_ranking("head")
## head query
rel = 1
tail = 5
## this should have the exact same candidates as the results the first triple from the other script
print(headRanking[1][5])


tailRanking = ranker.get_ranking("tail")
# tail query
head = 4
## this should have the exact same candidates as the results of the first triple from the other script
print(tailRanking[rel][head])

## ranker has the data still cached in backend, maybe delete the ranker now
## serialization just copies the data to python

serializeTime = time.time()
print(f"all time: {serializeTime-start}")
print(f"ranking time: {rankingtime-start}")
print(f"serialize time: {serializeTime-rankingtime}")









