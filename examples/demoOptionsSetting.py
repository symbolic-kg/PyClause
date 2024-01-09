from c_clause import PredictionHandler, Loader
from clause.config.options import Options
from clause.util.utils import get_base_dir


train = f"{get_base_dir()}/data/wnrr/train.txt"
filter = f"{get_base_dir()}/data/wnrr/valid.txt"
target = f"{get_base_dir()}/data/wnrr/test.txt"

rules = f"{get_base_dir()}/data/wnrr/anyburl-rules-c5-3600"


options = Options()

loader = Loader(options.get("loader"))
loader.load_data(train, filter)
loader.load_rules(rules)


scorer = PredictionHandler(options=options.get("prediction_handler"))


triples = [("12184337","_hypernym", "13112664")]

scorer.calculate_scores(triples, loader)
print(scorer.get_scores(True))

scorer.set_options(
    {
        "aggregation_function": "noisyor",
        "collect_explanations": "True",
    })
scorer.calculate_scores(triples, loader)
print(scorer.get_scores(True))

print(scorer.get_explanations(True)[1])
print(scorer.get_explanations(True)[2])


scorer.set_options({"num_top_rules": "-1"})

scorer.calculate_scores(triples, loader)
print(scorer.get_scores(True))



