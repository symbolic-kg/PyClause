import time
from c_clause import RankingHandler, Loader
from clause.util.utils import get_base_dir
from clause.config.options import Options

train = f"{get_base_dir()}/data/wnrr/train.txt"
filter = f"{get_base_dir()}/data/wnrr/valid.txt"
target = f"{get_base_dir()}/data/wnrr/test.txt"

rules = f"{get_base_dir()}/data/wnrr/anyburl-rules-c5-3600"

ranking_file = f"{get_base_dir()}/local/ranking-file.txt"

options = Options()
options.set("ranking_handler.disc_at_least", 10)
options.set("ranking_handler.aggregation_function", "maxplus")
options.set("ranking_handler.num_top_rules", -1)

#### Calculate a ranking and serialize / use in python
start = time.time()
loader = Loader(options=options.get("loader"))
loader.load_data(data=train, filter=filter, target=target)
loader.load_rules(rules=rules)


ranker = RankingHandler(options=options.get("ranking_handler"))
ranker.calculate_ranking(loader=loader)
ranker.write_ranking(path=ranking_file, loader=loader)
rankingtime = time.time()
headRanking = ranker.get_ranking(direction="head", as_string=False)
tailRanking = ranker.get_ranking(direction="tail", as_string=False)
serializeTime = time.time()


print(f"all time: {serializeTime-start}")
print(f"ranking time: {rankingtime-start}")
print(f"serialize time: {serializeTime-rankingtime}")


# you can change the loader rules options and load a different rule set 
# with the loader while keeping the data
# you cannot load a new dataset with the same loader though, you have to 
# create a new loader
print("Calculating new ranking")
options.set("loader.load_u_c_rules", False)
options.set("loader.load_u_d_rules", False)
options.set("loader.load_u_xxc_rules", False)
options.set("loader.load_u_xxd_rules", False)
loader.set_options(options.get("loader"))
loader.load_rules(rules)
ranker.calculate_ranking(loader)


