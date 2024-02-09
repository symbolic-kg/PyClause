
Triple Explanations
===================
The ``c_clause.PredictionHandler`` can be used to calculate explanations for input triples of the form **(head, relation, tail)**. Within
PyClause we say that an explanation for a triple is a rule, that predicts the triple, and the body groundings of the rule that lead to the prediction, given a KG.
There can be many explanations for one target triple. Find more conceptual information under :doc:`../conceptual/rules`.


Using the scoring function
~~~~~~~~~~~~~~~~~~~~~~~~~~~
Before the explanations can be obtained, rule application has to be performed with the ``calculate_scores(...)`` function where all the explanations
from the last section apply. Before that, the handler option ``"prediction_handler.collect_explanations"`` must be set to *True*.

First, we load data and rules.

.. code-block:: python

    from c_clause import PredictionHandler, Loader
    from clause import Options

    data = [
        ("marta", "bornIn", "rome"),
        ("italy", "hasCapital", "rome"),
        ("bernd", "speaks", "french"),
        ("marta", "speaks", "english"),
        ("marta", "speaks", "italian"),
        ("bernd", "teaches", "english"),
        ("english", "languageOf", "england"),
        ("australia", "hasCapital", "canberra"),
        ("enrico", "citizenOf", "germany")
    ]

    rules = [
        "citizenOf(X,Y) <= bornIn(X,A), hasCapital(Y,A)",
        "citizenOf(X,australia) <= teaches(X,A), languageOf(A,england)"
    ]
    stats = [
        [1000, 350],
        [100, 5],
    ]

    opts = Options()
    loader = Loader(opts.get("loader"))
    loader.load_data(data=data)
    loader.load_rules(rules=rules, stats=stats)


Then we create the handler and ensure that explanations are tracked.
Subsequently, we define the target triples and we call the scoring function that performs rule application.

.. code-block:: python

    opts.set("prediction_handler.collect_explanations", True)
    scorer = PredictionHandler(options=opts.get("prediction_handler"))

    targets = [
        ("marta", "citizenOf", "italy"),
        ("bernd", "citizenOf", "australia") 
    ]
    scorer.calculate_scores(triples=targets, loader=loader)


Retrieving Explanations
~~~~~~~~~~~~~~~~~~~~~~~
The handler caches the results until the ``calculate_scores(...)`` function is invoked again. The explanations can be retrieved with ``PredictionHandler.get_explanations(as_string)``.
The argument **as_string**, as for the other features, is a bool and decides if entities and relations are represented as strings or integer idx's. The output is a tuple containing three lists.

.. code-block:: python

    targets, pred_rules, groundings = scorer.get_explanations(as_string=True)


For input target triple **i**

- **targets[i]** is the original **i'th** target triple, this can be useful if they were, e.g., specified as file path.
- **pred_rules[i][j]** is the **j'th** rule predicting target **i**. Note that the number of rules may vary per target.
- **pred_rules[i][j][k]** gives the **k'th** grounding for the **j'th** rule predicting target **i**. A grounding is itself a list of triples. Note that the number of groundings per rule as well as the grounding length varies over different rules.


In our example each target was only predicted by one rule:

.. code-block:: python

    ## Explanation for ["marta", "citizenOf", "italy"]

    # Rule
    print(pred_rules[0][0])
    # out:
    # citizenOf(X,Y) <= bornIn(X,A), hasCapital(Y,A)

    # Grounding
    print(groundings[0][0][0])
    # out:
    # [['marta', 'bornIn', 'rome'], ['italy', 'hasCapital', 'rome']]


    ## Explanation for ["bernd", "citizenOf", "australia"] 

    # Rule
    print(pred_rules[1][0])
    # out:
    # citizenOf(X,australia) <= teaches(X,A), languageOf(A,england)

    # Grounding
    print(groundings[1][0][0])
    # out:
    # [['bernd', 'teaches', 'english'], ['english', 'languageOf', 'england']]
