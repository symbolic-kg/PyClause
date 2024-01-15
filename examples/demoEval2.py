"""
This example illustrates how to read and evaluate a ranking that has been stored previously.
Please run demoEval1.py first to create this ranking.
"""

from clause.util.utils import get_base_dir
from clause.config.options import Options

from clause.eval.evaluation import Ranking
from clause.data.triples import TripleSet

train = f"{get_base_dir()}/data/wnrr/train.txt"
filter_set = f"{get_base_dir()}/data/wnrr/valid.txt"
target = f"{get_base_dir()}/data/wnrr/test.txt"

rules = f"{get_base_dir()}/data/wnrr/anyburl-rules-c5-3600"

ranking_path = f"{get_base_dir()}/local/demo-eval1-ranking.txt"

testset = TripleSet(target)
ranking = Ranking(ranking_path)
ranking.add_filter_set(testset)
ranking.compute_scores(testset.triples)

print("MRR     " + '{0:.4f}'.format(ranking.hits.get_mrr()))
print("hits@1  " + '{0:.4f}'.format(ranking.hits.get_hits_at_k(1)))
print("hits@10 " + '{0:.4f}'.format(ranking.hits.get_hits_at_k(10)))