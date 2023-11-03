import sys
import os
sys.path.append(os.getcwd())

from clause.data.triples import TripleSet

if __name__ == '__main__':

    # load and rewrite a triple set such that it does not contains , or ( or ) within its names
    # morever, a suffix e (entity) or r (relation is added to avoid names that can be confused with varaibles
    # this seems to be a simpler and robust way to circumvent problems with datasets that use strange nameing schemes


    # TAKE CARE: For running this you need to speciy the correct paths to a the yago dataset, which is not
    # delivered with the PyClause release

    path_train = "E:/exp/data/yago/train.txt"
    path_valid = "E:/exp/data/yago/valid.txt"
    path_test  = "E:/exp/data/yago/test.txt"

    # in case of some special encoding (or issues with windows default coding) set encoding explicitly here
    train = TripleSet(path_train, encod="utf-8")
    valid = TripleSet(path_valid, encod="utf-8")
    test = TripleSet(path_test, encod="utf-8")

    train.write_masked(path_train.replace(".txt", "_masked.txt"))
    valid.write_masked(path_valid.replace(".txt", "_masked.txt"))
    test.write_masked(path_test.replace(".txt", "_masked.txt"))
