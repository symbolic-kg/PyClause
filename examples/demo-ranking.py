from c_clause import RankingHandler, Loader
from clause.util.utils import get_base_dir, read_jsonl
from clause import Options

# ** Example for creating ranking and corresponding rule features **

train = f"{get_base_dir()}/data/wnrr/train.txt"
# when loading from disk set to "" to not use additional filter
filter_set = f"{get_base_dir()}/data/wnrr/valid.txt"
target = f"{get_base_dir()}/data/wnrr/test.txt"

rules = f"{get_base_dir()}/data/wnrr/anyburl-rules-c5-3600"

out = f"{get_base_dir()}/local/ranking-file.txt"

options = Options()
options.set("ranking_handler.disc_at_least", 10)
options.set("ranking_handler.aggregation_function", "maxplus")
options.set("ranking_handler.num_top_rules", -1)
options.set("ranking_handler.num_threads", -1)

# load data and rules
loader = Loader(options=options.get("loader"))
loader.load_data(data=train, filter=filter_set, target=target)
loader.load_rules(rules=rules)

# create ranker
# if later rule features should be returned the respective option has to be set
options.set("ranking_handler.collect_rules", True)
ranker = RankingHandler(options=options.get("ranking_handler"))
# create ranking
ranker.calculate_ranking(loader=loader)
# write complete ranking to file (same as AnyBURL's ranking files)
ranker.write_ranking(path=out, loader=loader)

# obtain sub rankings as dicts 
# e.g. for relation i, entity j, head_ranking[i][j] contains head
# candidates for query (?, i, j) from some triple of "target" 
head_ranking = ranker.get_ranking(direction="head", as_string=True)
tail_ranking = ranker.get_ranking(direction="tail", as_string=True)

# obtain rule features for every query
head_rules = ranker.get_rules(direction="head", as_string=True)
tail_rules = ranker.get_rules(direction="tail", as_string=True)

# write rule features
ranker.write_rules(path="local/rule-feats_head.txt", loader=loader, direction="head", as_string=False)
ranker.write_rules(path="local/rule-feats_tail.txt", loader=loader, direction="tail", as_string=False)

# list of dicts
# note that the candidates are not sorted anymore
# as in 'head_ranking
rule_feats_head = read_jsonl("local/rule-feats_head.txt")


# you can change the loader rules options and load a different rule set 
# with the loader while keeping the data
# you cannot load a new dataset with the same loader though, you have to 
# create a new loader

# just take the first 100 rules
# note: they are NOT sorted but you could sort them here
rules_subset = loader.get_rules()[0:100]
loader.set_options(options.get("loader"))
loader.load_rules(rules_subset)
ranker.calculate_ranking(loader)
