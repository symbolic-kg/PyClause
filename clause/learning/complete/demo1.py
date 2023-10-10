from torm import Torm
from triples import TripleSet
from rules import RuleSet,Rule




if __name__ == '__main__':

    path_train = "E:/exp/data/wn18rr/train.txt"
    path_rules_output = "E://exp/pyclause/dev/rules-pctorm-wn18rr-b3-xall"

    # load a triple set from a file and display some meta info about it
    triples = TripleSet(path_train)
    print("loaded triple set: " +str(triples))

    # the index that has been created while loading the triple set can be accessed as follows:
    print(str(triples.index))

    # create a torm object to learn rules, as rule mining is always against a specific datasets, the index of the dataset is used
    # we choose all relations in the datasets as targets to learn rules for
    # instead of that, a specific list of target relations can be determined as first argument
    torm = Torm(triples.rels, triples)
    
    # mine rules
    torm.mine_rules()

    # write the rules that have been mined to a file
    torm.rules.write(path_rules_output)