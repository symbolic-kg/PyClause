import time
import c_clause
from clause.util.utils import get_base_dir, list_from_idx_file
import numpy as np


### Setting:  answer queries on the fly

train = "./data/wnrr/train.txt"
rules = "./data/wnrr/anyburl-rules-c5-3600"

entity_idx = "./data/wnrr/entity_ids.del"
relation_idx = "./data/wnrr/relation_ids.del"

options = {
    # apply options
    "num_preselect": "10",  ## equal to topk means we immediately stop rule application when we have topk cands (max-aggregation)
    "topk": "10", #number of candidates to output
    "tie_handling": "random",
    "filter_w_train": "false",
    "filter_w_target": "false",
    "disc_at_least":"-1", ## -1 is off
    "num_threads": "-1", # -1 uses all threads, when you answer only a few queries on the fly per invocation, set a low number here
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
loader = c_clause.Loader(options)
# ensure the backend uses our idx->str maps for entities and relations
# given we want to insert arrays and not strings later
ent_idx = list_from_idx_file(entity_idx)
rel_idx = list_from_idx_file(relation_idx)

loader.set_entity_index(ent_idx)
loader.set_relation_index(rel_idx)

# loading from string data is fine as we have set the index; loading from idx data is added later
# takes 5-7 minutes for wd5m on server
# note that loader is a pointer; the data is not exposed to python
# however, we never tried what happens when you'd copy this e.g. in a pytorch.dataLoader with multiple workers or what not
# maybe your pc explodes, so be careful
loader.load_data(train)
loader.load_rules(rules)


# QA handler
qa_handler = c_clause.QAHandler(options)


## this stuff you can do on the fly now

# we have to convert tensors to arrays
batch = np.array([[4,1,5], [44,1,45], [271, 6, 272]])

# tail queries
batch_t = batch[:, 0:2]
# this gets more efficient when more queries are in the batch
tails = qa_handler.answer_queries(batch_t, loader, "tail")

# every subsequent call of answer_queries deletes previous data; data is not cached

print("Tail candidates")

for i in range(len(tails)):
    # list of tuples tup with tup[0]=cand idx and tup[1] = cand score
    # can be converted to array, but the whole list can't as the elments lengths (num_cands) are not equal
    tail_cands = tails[i]
    print(tail_cands)
    print("")


# head queries;  inputs are always in entity, relation order, here (tail,rel)
print("Head candidates")
batch_h = batch[:, [2, 1]]
heads = qa_handler.answer_queries(batch_h, loader, "head")
for i in range(len(heads)):
    ## the last guy here has more than topk=10 candidates, the reason is we never cut the predictions of one single rule
    ## e.g. we have 8 candidates already and the next rule predicts 10 new ones, then we output 18
    ## if we would cut, results would be random
    print(heads[i])
    print("")
