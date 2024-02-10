from c_clause import RankingHandler, Loader
from clause.util.utils import get_base_dir
from clause import Options

from clause import Ranking
from clause import TripleSet


# *** Example Evaluation ***

## This example illustrates how to create a ranking from a rule set that has been created previously.
## The ranking is evaluated on the fly before storing it on disc.
## The example shows also at the end how to use a few lines of code to create a
## structured results table that informs about relation and direction specific MRR and hits scores.

train = f"{get_base_dir()}/data/wnrr/train.txt"
filter_set = f"{get_base_dir()}/data/wnrr/valid.txt"
target = f"{get_base_dir()}/data/wnrr/test.txt"

rules = f"{get_base_dir()}/data/wnrr/anyburl-rules-c5-3600"

ranking_file = f"{get_base_dir()}/local/ranking-wnrr.txt"

options = Options()
options.set("ranking_handler.aggregation_function", "maxplus")
options.set("ranking_handler.num_top_rules", -1) # reset to -1
options.set("ranking_handler.topk", 100)

options.set("ranking_handler.disc_at_least", 100)
options.set("loader.load_u_d_rules", False)
options.set("loader.load_u_xxc_rules", False)
options.set("loader.load_u_xxd_rules", False)


#### Calculate a ranking
loader = Loader(options=options.get("loader"))
loader.load_data(data=train, filter=filter_set, target=target)
loader.load_rules(rules=rules)

ranker = RankingHandler(options=options.get("ranking_handler"))
ranker.calculate_ranking(loader=loader)
headRanking = ranker.get_ranking(direction="head", as_string=True)
tailRanking = ranker.get_ranking(direction="tail", as_string=True)

testset = TripleSet(target)
ranking = Ranking(k=100)

# process the handler ranking which is defined on queries and not
# on triples, e.g. assign to every triple of 'testset' the corresponding query rankings
ranking.convert_handler_ranking(headRanking, tailRanking, testset)
ranking.compute_scores(testset.triples)

print("*** EVALUATION RESULTS ****")
print("Num triples: " + str(len(testset.triples)))
print("MRR     " + '{0:.6f}'.format(ranking.hits.get_mrr()))
print("hits@1  " + '{0:.6f}'.format(ranking.hits.get_hits_at_k(1)))
print("hits@3  " + '{0:.6f}'.format(ranking.hits.get_hits_at_k(3)))
print("hits@10 " + '{0:.6f}'.format(ranking.hits.get_hits_at_k(10)))
print()

# now some code to some nice overview on the different relations and directions
# the loop interates over all relations in the test set
print("relation".ljust(25) + "\t" + "MRR-h" + "\t" + "MRR-t" + "\t" + "Num triples")
for rel in testset.rels:
   rel_token = testset.index.id2to[rel]
   # store all triples that use the current relation rel in rtriples
   rtriples = list(filter(lambda x: x.rel == rel, testset.triples))

   # compute scores in head direction ...
   ranking.compute_scores(rtriples, True, False)
   (mrr_head, h1_head) = (ranking.hits.get_mrr(), ranking.hits.get_hits_at_k(1))
   # ... and in tail direction
   ranking.compute_scores(rtriples, False, True)
   (mrr_tail, h1_tail) = (ranking.hits.get_mrr(), ranking.hits.get_hits_at_k(1))
   # print the resulting scores
   print(rel_token.ljust(25) +  "\t" + '{0:.3f}'.format(mrr_head) + "\t" + '{0:.3f}'.format(mrr_tail) + "\t" + str(len(rtriples)))


# finally, write the ranking to a file, there are two ways to to this, both reults into the same ranking
# 1) use the ranking object
ranking.write(ranking_file)

# 2) ask the ranker to write the ranking directly
# ranker.write_ranking(path=ranking_file, loader=loader)