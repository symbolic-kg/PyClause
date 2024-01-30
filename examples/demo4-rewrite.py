from clause.data.triples import TripleSet
from clause.util.utils import get_base_dir

    

# ** Example: Removing special characters from input KGs to avoid parsing problems **
#  use this if you have problems with special characters in your dataset when, e.g., loading rules
#  or when you have problems with the windows default en/decoding 

path_train = f"{get_base_dir()}/data/yago3-10/train.txt"
path_valid = f"{get_base_dir()}/data/yago3-10/valid.txt"
path_test = f"{get_base_dir()}/data/yago3-10/test.txt"

train = TripleSet(path_train, encod="utf-8")
valid = TripleSet(path_valid, encod="utf-8")
test = TripleSet(path_test, encod="utf-8")

train.write_masked(path_train.replace(".txt", "_masked.txt"))
valid.write_masked(path_valid.replace(".txt", "_masked.txt"))
test.write_masked(path_test.replace(".txt", "_masked.txt"))
