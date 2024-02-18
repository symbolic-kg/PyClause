from c_clause import PredictionHandler, Loader
from clause import Options
from clause.util.utils import get_base_dir

import numpy as np


## *** Example for triple ccoring on Yago3-10 **


## ** Preparation **

# ensure that the data is downloaded from the data downloads on read the docs
data = f"{get_base_dir()}/data/yago3-10/train.txt"
rules = f"{get_base_dir()}/data/yago3-10/anyburl-rules-small.txt"
opts = Options()

loader = Loader(options=opts.get("loader"))
loader.load_data(data=data)
loader.load_rules(rules)

# prediction score for a triple is the confidence of the highest predicting rule
opts.set("prediction_handler.aggregation_function", "maxplus")
opts.set("prediction_handler.num_top_rules", 1)


scorer = PredictionHandler(options=opts.get("prediction_handler"))


## **1) Scoring from strings **

# from test.txt
triples = [
    ["Radolfzell",	"isLocatedIn", "Baden-Württemberg"],
	["Paul_McCartney", "hasMusicalRole", "wordnet_bass_guitar_102804123"],
    ["Agatha_Christie",	"influences", "Ibn-e-Safi"],
]
scorer.calculate_scores(triples=triples, loader=loader)

# output as string or idx's
str_scores = scorer.get_scores(as_string=True)
arr_scores = np.array(scorer.get_scores(as_string=False))
# list of lists with 4 elements: head, relation, tail, score
print(str_scores)
# 2d np.array
print(arr_scores)

# write to file
scorer.write_scores(path=f"{get_base_dir()}/local/triples-scores.txt", as_string=True)


## **2) Scoring from idx's **

# we use random entites/relation idx's here
# one may set an index before data loading
# or retrieve the index after loading like so
entity_index = loader.get_entity_index()
relation_index = loader.get_relation_index()


triples = np.array([[10, 5, 1050], [3, 0, 34]])
scorer.calculate_scores(triples=triples, loader=loader)

# write to file / output as idx's / output as string; independent of input type
# e.g. output as string
# if a target triple is not predicted by any rule, score is 0
print(scorer.get_scores(as_string=True))  



## **3) Scoring from file **

# we score every triple in the yago testset
targets = f"{get_base_dir()}/data/yago3-10/test.txt"

# the test set contains entities that are not in the train.txt
# and therefore not known in the index but they have to be known
# we therefore use a new loader and also load the test set as filter
# it does not have any other effect here except of filling the index
loader2 = Loader(opts.get("loader"))
loader2.load_data(data=data, filter=targets)
loader2.load_rules(rules=rules)

scorer.calculate_scores(triples=targets, loader=loader2)

# write, output to python as above etc..
out_path = f"{get_base_dir()}/local/yago-test-scores.txt"
scorer.write_scores(path=out_path, as_string=True)


# output some scores
print(scorer.get_scores(as_string=True)[0:5])



















