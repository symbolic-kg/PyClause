import time
import c_clause
from clause.util.utils import get_base_dir



data = "./data/wnrr/train.txt"

options = {
    "collect_statistics": "true",
    "collect_predictions": "true"
}



#### Calculate a ranking and serialize / use in python
start = time.time()
loader = c_clause.DataHandler(options)
loader.load_data(data)


### Calculate exact rule statistics through materialization


rules_list = [
    "_hypernym(X,06355894) <= ",
    "_hypernym(X,06355894) <= _synset_domain_topic_of(X,A), _synset_domain_topic_of(06355894,A)",
]

handler = c_clause.RulesHandler(options=options)

handler.calculate_predictions(rules=rules_list, loader=loader)

# for each rule get string predictions
print(handler.get_predictions(as_string=True))


# for each rule get idx predictions
print(handler.get_predictions(as_string=False))



# for each rule get num_predicted num_true_predicted
print("Statistics")
print(handler.get_statistics())










