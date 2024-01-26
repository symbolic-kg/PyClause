
Triple Scoring
===============

The ``c_clause.PredictionHandler`` can be used to calculate scores to triple predictions of the form **(head, relation, tail)**.
We assume in the following a loader is created and data and rules are loaded:

.. code-block:: python

    from c_clause import PredictionHandler, Loader
    from clause import Options

    data = [
        ("anna", "livesIn", "london"),
        ("anna", "learns", "english"),
        ("bernd", "speaks", "french")
    ]
    rules = [
        "speaks(X,Y) <= learns(X,Y)",
        "speaks(X,english) <= livesIn(X,london)",
        "speaks(X,english) <= speaks(X,A)"
    ]
    stats = [
        [20, 10],
        [40, 35],
        [50, 5],
    ]
    opts = Options()
    loader = Loader(opts.get("loader"))
    loader.load_data(data=data)
    loader.load_rules(rules=rules, stats=stats)



Calculating Triple Scores
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The handler is initialized with options and scores are calculated with ``PredictionHandler.calculate_scores(triples, loader)``. The calculated scores
depend on the handler options and the selected aggregation function ``aggregation_function`` under the prediction handler options.
If a triple is not predicted by any rule, the calculated score will be 0. The function takes as arguments the triples input and the loader.

.. code-block:: python

    scorer = PredictionHandler(options=opts.get("prediction_handler"))
    triples = [("anna", "speaks", "french"), ("anna", "speaks", "english")]
    scorer.calculate_scores(triples=triples, loader=loader)


**Triple input types**

1. **From Python as strings**: **triples** can be a list of string triples (tuples or list). See above.


2. **From file as strings**:  **triples** can be a file path where every line contains a tab separated string triple, e.g., the standard KG input datasets:


.. code-block:: bash

    anna    speaks  french
    anna   speaks   english


3. **From Python as idx's**: **triples** can be a list or 2d np.array of idx's triples. This requires the user to know
the relation and entity index. Either by retrieving them from the loader **after** data is loaded or by setting them **before** data is loaded. See also in the section :doc:`../data/loading_data`.
The following example sets the entity and relation index before loading data and rules.


.. code-block:: python

    from c_clause import PredictionHandler, Loader
    from clause import Options
    import numpy as np

    data = [
        ("anna", "livesIn", "london"),
        ("anna", "learns", "english"),
        ("bernd", "speaks", "french")
    ]
    rules = [
        "speaks(X,Y) <= learns(X,Y)",
        "speaks(X,english) <= livesIn(X,london)",
        "speaks(X,english) <= speaks(X,A)"
    ]
    stats = [
        [20, 10],
        [40, 35],
        [50, 5],
    ]
   
    opts = Options()
    loader = Loader(opts.get("loader"))
   
    # 0:anna, 1:bernd 2:london ...
    entity_index = ["anna", "bernd", "london", "english", "french"]
    # 0: speaks, 1:livesIn ...
    relation_index = ["speaks", "livesIn", "learns"]

    # set index before loading data and rules
    loader.set_entity_index(index=entity_index)
    loader.set_relation_index(index=relation_index)

    loader.load_data(data=data)
    loader.load_rules(rules=rules, stats=stats)

    triples = np.array([(0,0,3), (0,0,4)])
    scorer = PredictionHandler(options=opts.get("prediction_handler"))
    scorer.calculate_scores(triples=triples, loader=loader)


Retrieving Results
~~~~~~~~~~~~~~~~~~

The handler caches the results until the ``calculate_scores(..)`` function is invoked again. Independent of how data was loaded and how triple inputs were defined (strs or idx's),
outputs can be written to a file or obtained in Python and they can be formatted as idx's or as strings.

**Outputting candidates and scores**

.. code-block:: python

    ...
    scorer.calculate_scores(triples=triples, loader=loader)

    # output strings
    # returns a list of lists
    # each list scores_str[i] contains 4 elements: head, relation, tail, score
    scores_str = scorer.get_scores(as_string=True)
    # output idx's; same as above only as idx's
    scores_idx = scorer.get_scores(as_string=False)
    # write to file as string
    # writes TAB-separated lines: head relation tail score
    scorer.write_scores(path="scores_str.tx", as_string=True)
    # write to file as idx; as above only as idx's
    scorer.write_scores(path="scores_idx.tx", as_string=False)


Triple Explanations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
We loosely define an explanation for a target triple prediction **(head, relation, tail)** as

- a rule that predicted the target triple
- the triples/atoms that ground the body of the rule s.t. the target prediction is the resulting grounded head of the rule

How to retrieve explanations from the ``PredictionHandler`` is explained in section :doc:`triple_explanations`.









