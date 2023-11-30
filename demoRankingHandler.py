import time
import c_clause
from clause.util.utils import get_base_dir
from clause.config.options import Options



# train = "/home/patrick/Desktop/PyClause/data/fb15k-237/train.txt"
# filter = "/home/patrick/Desktop/PyClause/data/fb15k-237/valid.txt"
# target = "/home/patrick/Desktop/PyClause/data/fb15k-237/test.txt"
# rules = "/home/patrick/Desktop/PyClause/data/fb15k-237/anyburl-rules-c3-3600"
# ranking_file = "/home/patrick/Desktop/PyClause/data/fb15k-237/rankingFile.txt"

train = "./data/wnrr/train.txt"
filter = "./data/wnrr/valid.txt"
target = "./data/wnrr/test.txt"

rules = "/home/patrick/Desktop/PyClause/data/wnrr/anyburl-rules-c5-3600"

ranking_file = "./local/rankingFile.txt"

options = Options()
options.set("ranking_handler.disc_at_least", 100)

#### Calculate a ranking and serialize / use in python
start = time.time()
loader = c_clause.DataHandler(options.flatS("data_handler"))
loader.load_data(train, filter, target)
loader.load_rules(rules)


ranker = c_clause.RankingHandler(options.flatS("ranking_handler"))
ranker.calculate_ranking(loader)
ranker.write_ranking(ranking_file, loader)
rankingtime = time.time()
headRanking = ranker.get_ranking("head")
tailRanking = ranker.get_ranking("tail")
serializeTime = time.time()


print(f"all time: {serializeTime-start}")
print(f"ranking time: {rankingtime-start}")
print(f"serialize time: {serializeTime-rankingtime}")


