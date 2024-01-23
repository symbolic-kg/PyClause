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


# ** QA with string inputs - tail queries**

# e.g.,  ("12184337","_hypernym", ?)
tail_queries_str = [
    ("08801678","_has_part")
    ("12184337","_hypernym"),
]

# queries: alternatively specify file path with lines "source\trelation"
qa_handler.calculate_answers(queries=tail_queries_str, loader=loader, direction="tail")

# you can output entity/rule idx's or entity/rule strings independent of input type
# set as_string to True to retrieve rule strings/entity strings
as_string = False
answers = qa_handler.get_answers(as_string=as_string)
rules = qa_handler.get_rules(as_string=as_string)

# write to disk
qa_handler.write_answers("tail-query-answers.jsonl", as_string=as_string)
qa_handler.write_rules("tail-query-rules.jsonl", as_string=as_string)

# rules and answers for first query
print(answers[0])
print(rules[0])

exit()


# ** QA with idx inputs -  head queries **

# we did not provide our own entity/relation index
# we use the one constructed from the loader
# dict: str->idx
entity_index = loader.get_entity_index()
relation_index = loader.get_relation_index()



### idx input mode
### input: queries, string
### queries list of tuples with two integer, the integer are the idx's 
### or Nx2 np.array where N is the number of queries, first column are source entity ids second are relation ids
### output: the output remains a list[list[tuple[int,float]]] as different queries have different amounts of answers
### each set i of answers can be converted to np.array with np.array(output[i])
queries = np.array([(4,5), (0,1)])

# answers to first query

qa_handler.calculate_answers(queries, loader, "tail")
answers = qa_handler.get_answers(not as_string)
answer_set = np.array(answers[0], dtype=object)
# print idx answers
print(answer_set[:,0])
# print confidences
print(answer_set[:,1])


