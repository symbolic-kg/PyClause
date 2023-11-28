import c_clause
import numpy as np


train = "./data/wnrr/train.txt"
filter = "./data/wnrr/valid.txt"
target = "./data/wnrr/test.txt"

rules = "./data/wnrr/anyburl-rules-c5-3600"


options = {
    # ranking options
    "aggregation_function": "maxplus",
    "num_preselect": "10000000",
    "topk": "100",
    "filter_w_train": "false",
    "filter_w_target": "true",
    "tie_handling": "frequency",
    "disc_at_least":"100", ## -1 for off, must not be bigger than topk
    # rule options 
    "rule_b_max_branching_factor": "-1",
    "use_zero_rules": "false",
    "rule_zero_weight":"0.01",
    "use_u_c_rules": "true",
    "use_b_rules": "true",
    "use_u_d_rules": "true",
    "rule_u_d_weight":"0.01",
    "use_u_xxc_rules": "true",
    "use_u_xxd_rules": "true",
}

loader = c_clause.DataHandler(options)
loader.load_data(train, filter)
loader.load_rules(rules)



qa_handler = c_clause.QAHandler(options)


## string inputs mode
## input queries:
## list of tuples with 2 strings (or list of list)
## the first element of the tuple is the source entity the second element is the relation
queries = [("12184337","_hypernym"), ("12184337","_verb_group")]

## input args: queries (see above), string: "head" or "tail" the query type
## head: the query rel(?, sourceEnt) will be answered
## tail: the query rel(sourceEnt, ?) will be answered
##
## output: list[list[tuple[string,float]]] 
## e.g. output[0] contains a list with the answers for the first query
## note that output[i] does not have same length as output[k] as different queries have different amounts of answers
qa_handler.calculate_answers(queries, loader, "tail")
as_string = True
answers = qa_handler.get_answers(as_string)
print(answers)


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


