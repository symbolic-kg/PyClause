import time
from c_clause import RankingHandler, DataHandler
from clause.util.utils import get_base_dir
from clause.config.options import Options



train = "/home/patrick/Desktop/PyClause/data/fb15k-237/train.txt"
filter = "/home/patrick/Desktop/PyClause/data/fb15k-237/valid.txt"
target = "/home/patrick/Desktop/PyClause/data/fb15k-237/test.txt"
rules = "/home/patrick/Desktop/PyClause/data/fb15k-237/anyburl-rules-c3-3600"
ranking_file = "/home/patrick/Desktop/PyClause/data/fb15k-237/rankingFile.txt"

# train = "./data/wnrr/train.txt"
# filter = "./data/wnrr/valid.txt"
# target = "./data/wnrr/test.txt"

# rules = "/home/patrick/Desktop/PyClause/data/wnrr/anyburl-rules-c5-3600"

# ranking_file = "./local/rankingFileNoisy.txt"

options = Options()
options.set("ranking_handler.disc_at_least", 10)
options.set("ranking_handler.aggregation_function", "noisyor")
options.set("ranking_handler.num_top_rules", 5)

#### Calculate a ranking and serialize / use in python
start = time.time()
loader = DataHandler(options.flatS("data_handler"))
loader.load_data(train, filter, target)
loader.load_rules(rules)


ranker = RankingHandler(options.flatS("ranking_handler"))
ranker.calculate_ranking(loader)
ranker.write_ranking(ranking_file, loader)
rankingtime = time.time()
headRanking = ranker.get_ranking("head", False)
tailRanking = ranker.get_ranking("tail", False)
serializeTime = time.time()


print(f"all time: {serializeTime-start}")
print(f"ranking time: {rankingtime-start}")
print(f"serialize time: {serializeTime-rankingtime}")


