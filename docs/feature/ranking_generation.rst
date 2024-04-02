
Ranking Generation
==================

The ``c_clause.RankingHandler`` can be used to create full rankings in the context of knowledge graph completion.
Different from the other handlers, rule application is not performed on the fly. 
Instead, The target KG has to be loaded additionally into the loader with the **target** argument.
The ranking is created for the target KG by grounding the rules on the **data** argument of the loader.

A complete ranking for the target KG is defined as follows. For each triple **(head, relation, tail)** from target,
two queries are formed **(head, relation, ?)** and **(?, relation, tail)**. The ranking contains for each of the queries a ranked list of candidate proposals
sorted according to a heuristic based on the aggregation function specified.

First we define data and rules.

.. code-block:: python

    from c_clause import RankingHandler, Loader
    from clause import Options

    train = [
        ("marta", "bornIn", "rome"),
        ("italy", "hasCapital", "rome"),
        ("bernd", "speaks", "french"),
        ("marta", "speaks", "english"),
        ("marta", "speaks", "italian"),
        ("bernd", "teaches", "english"),
        ("enrico", "bornIn", "rome"),
    ]

    valid = [
        ("english", "languageOf", "england"),
        ("australia", "hasCapital", "canberra"),
        ("enrico", "citizenOf", "italy")
    ]

    test = [
        ("marta", "citizenOf", "italy"),
        ("bernd", "teaches", "french")
    ]
    
    rules = [
        "citizenOf(X,Y) <= bornIn(X,A), hasCapital(Y,A)",
        "teaches(X,french) <= teaches(X, english)",
    ]

    stats = [
        [20, 5],
        [21, 1],
    ]

Loading Data and Rules
~~~~~~~~~~~~~~~~~~~~~~

For calculating rankings all three arguments of the  ``load_data(data, filter, target)`` function of the loader have to be used.
The loader can load a filter set (commonly the valid split of a KG). If it is not empty proposed candidates will be filtered always with this filter set.
In our example,  **enrico**  will not be provided as answer to the query **(?, citizenOf, italy)** even if it is predicted by one or more rules. Also see the other filter options
for data and target in the `config-default.yaml <https://github.com/symbolic-kg/PyClause/blob/master/clause/config-default.yaml>`_ .


.. code-block:: python

    opts = Options()
    loader = Loader(opts.get("loader"))
    # set filter to "" if not required
    loader.load_data(data=data, fiter=valid, target=test)
    loader.load_rules(rules=rules, stats=stats)


Calculating and Retrieving Rankings
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ranking handler calculates a ranking with the ``RankingHandler.calculate_ranking(loader)`` function for the KG specified with **target** while rules are applied on **data**. The results are cached until the function
is invoked again.

.. code-block:: python

    ranker = RankingHandler(opts.get("ranking_handler"))
    ranker.calculate_ranking(loader=loader)

The ranking can be retrieved in Python which is separated in the two query directions. 

.. code-block:: python

    head_ranking = ranker.get_ranking(direction="head", as_string=False)
    tail_ranking = ranker.get_ranking(direction="tail", as_string=True)


Where **head_ranking** is a dict and **head_ranking[i][j]** corresponds to query **(?, i, j)**. Note that the relations denote the first key of the dict, s.t. relation-wise rankings can be retrieved easily.
**head_ranking[i][j]** returns a sorted list of of tuples **(cand, score)** with head candidate proposals for the query.
The explanations for the tail direction are identical and the dicts are always accessed with [rel][source-entitiy].


The complete ranking can also be written to a file. The output format is the same as the AnyBURL ranking files. This function only supports string outputs.

.. code-block:: python

    ranker.write_ranking(path=out, loader=loader)


Retrieving Rule Features
~~~~~~~~~~~~~~~~~~~~~~~~~
The ranker can also cache and output, for each candidate of every query, the rules that predicted the candidate. For this the option ``"ranking_handler.collect_rules"``
must be set to *True* (default: *False*) before the ranking is calculated. 

.. code-block:: python
    
    # obtain rule features for every query
    head_rules = ranker.get_rules(direction="head", as_string=True)
    tail_rules = ranker.get_rules(direction="tail", as_string=True)

    # write rule features
    ranker.write_rules(path="rule-feats_head.txt", loader=loader, direction="head", as_string=False)
    ranker.write_rules(path="rule-feats_tail.txt", loader=loader, direction="tail", as_string=False)


    from clause.util.utils import read_jsonl
    # list of dicts
    read_jsonl("rule-feats_tail.txt")


Here, **head_rules[rel][source]** returns a dict for query **(?, rel, source)** and **head_rules[rel][source][cand]** returns the sorted list of predicting rules.


The output files are in **jsonl** format. Each line contains a json:

.. code-block:: python
    
    { "query": [rel, source ], "answers": [ list of candidates], "rules": [[rules_cand_0], [rules_cand_1],...]}

However, note that the list of candidates is not sorted, e.g., it does not match the ordering of **head_ranking[rel][source]** from above.





