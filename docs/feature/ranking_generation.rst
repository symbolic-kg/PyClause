
Ranking Generation
==================

The ``c_clause.RankingHandler`` can be used to create full rankings in the context of knowledge graph completion.
Different from the other handlers, rule application is not performed on the fly. Instead, The target KG has to be loaded additionally into the loader with the **target** argument.

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

   
The loader can load an additional filter set (commonly the valid split of a KG). If it is not empty proposed candidates will be filtered always with this filter set.
In our example,  **enrico**  will not be provided as answer to the query **(?, citizenOf, italy)** even if it is predicted by one or more rules.

.. code-block:: python

    opts = Options()
    loader = Loader(opts.get("loader"))
    loader.load_data(data=data, fiter=valid, target=test)
    loader.load_rules(rules=rules, stats=stats)


The ranking handler can calculate a ranking with the 