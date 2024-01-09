from clause.data.triples import TripleSet
from clause.util.utils import get_base_dir

if __name__ == '__main__':

    # load and rewrite a triple set such that it does not contains , or ( or ) within its names
    # morever, a suffix e (entity) or r (relation is added to avoid names that can be confused with varaibles
    # this seems to be a simpler and robust way to circumvent problems with datasets that use strange nameing schemes


    # TAKE CARE: For running this you need to ensure that the data is present

    path_train = f"{get_base_dir()}/data/yago/train.txt"
    path_valid = f"{get_base_dir()}/data/yago/valid.txt"
    path_test = f"{get_base_dir()}/data/yago/test.txt"


    # in case of some special encoding (or issues with windows default coding) set encoding explicitly here
    train = TripleSet(path_train, encod="utf-8")
    valid = TripleSet(path_valid, encod="utf-8")
    test = TripleSet(path_test, encod="utf-8")

    train.write_masked(path_train.replace(".txt", "_masked.txt"))
    valid.write_masked(path_valid.replace(".txt", "_masked.txt"))
    test.write_masked(path_test.replace(".txt", "_masked.txt"))
