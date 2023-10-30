from torm import Torm
from triples import TripleSet

from rules import RuleSet
from ruleparser import RuleReader

import c_clause




if __name__ == '__main__':

    # load and rewrite a triple set such that it does not contains , or ( or ) within its names
    # morever, a suffix e (entity) or r (relation is added to avoid names that can be confused with varaibles
    # this seems to be a simpler and robust way to circumvent problems with datasets that use strange nameing schemes

    path_train = "E:/exp/data/yago/train.txt"
    path_valid = "E:/exp/data/yago/valid.txt"
    path_test  = "E:/exp/data/yago/test.txt"

    path_train = "E:/exp/data/yago/train.txt"
    path_valid = "E:/exp/data/yago/valid.txt"
    path_test  = "E:/exp/data/yago/test.txt"


    train = TripleSet(path_train)
    valid = TripleSet(path_valid)
    test = TripleSet(path_test)

    train.write_masked(path_train.replace(".txt", "_masked.txt"))
    valid.write_masked(path_valid.replace(".txt", "_masked.txt"))
    test.write_masked(path_test.replace(".txt", "_masked.txt"))


