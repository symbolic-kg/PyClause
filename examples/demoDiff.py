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

# ranking_path = f"{get_base_dir()}/local/demo-eval1-ranking.txt"

ranking_path_p = f"{get_base_dir()}/local/patrick-ranking-wnrr.txt"
ranking_path_c = f"{get_base_dir()}/local/christian-ranking-wnrr.txt"

testset = TripleSet(target)
ranking_p = Ranking(ranking_path_p)
ranking_p.compute_scores(testset.triples)

ranking_c = Ranking(ranking_path_c)
ranking_c.compute_scores(testset.triples)


print("Patrick vs Christian vs Christian,filtered") 
print("MRR " + '{0:.6f}'.format(ranking_p.hits.get_mrr()) + "   hits@1 " + '{0:.6f}'.format(ranking_p.hits.get_hits_at_k(1)) + "   hits@10 " + '{0:.6f}'.format(ranking_p.hits.get_hits_at_k(10)))
print("MRR " + '{0:.6f}'.format(ranking_c.hits.get_mrr()) + "   hits@1 " + '{0:.6f}'.format(ranking_c.hits.get_hits_at_k(1)) + "   hits@10 " + '{0:.6f}'.format(ranking_c.hits.get_hits_at_k(10)))
