
Evaluation for KGC
==================

The ``clause`` packe provides two utility classes ``TripleSet`` and ``Ranking`` that can be used to calculate ranking-based evaluation metrics
such as MRR or hits@k . They also allow to perform finegrained evaluations, e.g., relation-wise or for particular triple subsets. We show a complete example here,
that creates from a ranking file which was stored with ``RankingHandler.write_ranking(..)`` the full MRR and relation-wise and direction-wise MRRs.


.. code-block:: python

    from clause import Ranking
    from clause import TripleSet

    # target KG on which ranking was created
    target = "test.txt"
    # path of the stored ranking
    ranking_path = "ranking.txt"

    testset = TripleSet(target)
    ranking = Ranking(ranking_path)
    ranking.compute_scores(testset.triples)

    print("MRR     " + '{0:.6f}'.format(ranking.hits.get_mrr()))
    print("hits@1  " + '{0:.6f}'.format(ranking.hits.get_hits_at_k(1)))
    print("hits@10 " + '{0:.6f}'.format(ranking.hits.get_hits_at_k(10)))


    print("*** EVALUATION RESULTS ****")
    print("Num triples: " + str(len(testset.triples)))
    print("MRR     " + '{0:.6f}'.format(ranking.hits.get_mrr()))
    print("hits@1  " + '{0:.6f}'.format(ranking.hits.get_hits_at_k(1)))
    print("hits@3  " + '{0:.6f}'.format(ranking.hits.get_hits_at_k(3)))
    print("hits@10 " + '{0:.6f}'.format(ranking.hits.get_hits_at_k(10)))
    print()

    # display relation and direction MRRs and the respective number of triples
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
