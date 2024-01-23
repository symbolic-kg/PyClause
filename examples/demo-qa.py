from c_clause import QAHandler, Loader
import numpy as np
from clause import Options
from clause.util.utils import get_base_dir

# *** Example for query answering on WNRR ***


# ** Preparation **

train = f"{get_base_dir()}/data/wnrr/train.txt"
filter_set = f"{get_base_dir()}/data/wnrr/valid.txt"

rules = f"{get_base_dir()}/data/wnrr/anyburl-rules-c5-3600"
options = Options()

loader = Loader(options.get("loader"))
loader.load_data(train, filter_set)
loader.load_rules(rules)

# leave off for efficiency if not needed
options.set("qa_handler.collect_rules", True)
# output only a few candidate
options.set("qa_handler.topk", 5)
qa_handler = QAHandler(options=options.get("qa_handler"))


# ** 1) QA with string inputs - tail queries (answer tail given head, rel**

# e.g.,  ("12184337","_hypernym", ?)
tail_queries_str = [
    ("08801678","_has_part"),
    ("12184337","_hypernym"),
]

# queries: alternatively specify file path with lines "source\trelation"
qa_handler.calculate_answers(queries=tail_queries_str, loader=loader, direction="tail")

# you can output entity/rule idx's or entity/rule strings independent of input type
# set as_string to False to retrieve rule/entity idx's
as_string = True
answers = qa_handler.get_answers(as_string=as_string)
rules = qa_handler.get_rules(as_string=as_string)

# write to disk
qa_handler.write_answers("tail-query-answers.jsonl", as_string=as_string)
qa_handler.write_rules("tail-query-rules.jsonl", as_string=as_string)

# rules and answers for first query
print(answers[0])
print(rules[0])



# ** 2) QA with idx inputs -  head queries (answer heads given tail, rel) **

# we could also provide our own relation/index before loading data
# here, we use the one constructed from the loader
# dict: str->idx
entity_index = loader.get_entity_index()
relation_index = loader.get_relation_index()

# retrieve idx's 
rel_idx = relation_index["_has_part"]
ent1_idx = entity_index["09477567"]
ent2_idx = entity_index["08780881"]

# note that the queries are (?, rel, tail) but the source entity 
# still needs to be located at the first position
head_queries_idx = [
    [ent1_idx, rel_idx],
    [ent2_idx, rel_idx],
]
# pass it as is or as numpy array
# 2d np.Array
head_queries_idx = np.array(head_queries_idx)

qa_handler.calculate_answers(queries=head_queries_idx, loader=loader, direction="head")

# do all the stuff from above..
print(qa_handler.get_answers(as_string=True))


# ** 3) QA with readable entity names - tail queries **





