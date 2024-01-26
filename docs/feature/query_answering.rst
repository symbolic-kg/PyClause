
Query Answering
===============

The ``c_clause.QAHandler`` can be used to answer head queries of the form **(?, rel, source_entity)** and tail queries  **(source_entity, rel, ?)**. It can also 
output for each candidate entity, the rules that predicted the candidate.


We assume in the following a loader is created and data and rules are loaded:

.. code-block:: python

    from c_clause import QAHandler, Loader
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



Calculating Candidate Answers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The handler is initialized with options and answers are calculated with ``c_clause.QAHandler.calculate_answers(..)``.
The function takes the query input, the loader and one of two possible query directions "head" or "tail".

.. code-block:: python

    qa = QAHandler(options=opts.get("qa_handler"))
    queries = [("anna", "speaks"), ("bernd", "speaks")]
    # direction == "head" or "tail"
    qa.calculate_answers(queries=queries, loader=loader, direction="tail")


**Query input types**

1. **From Python as strings**: **queries** can be a list of string queries (tuples or list) containing the source entity at the first and the relation
at the second position (see above)


2. **From file as strings**:  **queries** can be a file path where every line contains a tab separated query, *my-queries.txt*:


.. code-block:: bash

    anna    speaks
    bernd   speaks

.. note::

    Queries are always specified with the source entity first and subsequently with the relation. Even when **direction** is specified to be **head**.

3. **From Python as idx's**: **queries** can be a list or 2d np.array of idx's queries. This requires the user to know
the relation and entity index. Either by retrieving them from the loader **after** data is loaded or by setting them **before** data is loaded. See also in the section :doc:`../data/loading_data`.
The following example sets the entity and relation index before loading data and rules.


.. code-block:: python

    from c_clause import QAHandler, Loader
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
    relation_index = ["speaks", "livesIn", "learns"]

    # set index before loading data and rules
    loader.set_entity_index(index=entity_index)
    loader.set_relation_index(index=relation_index)

    loader.load_data(data=data)
    loader.load_rules(rules=rules, stats=stats)

    queries = np.array([(0,0), (1,0)])
    qa = QAHandler(options=opts.get("qa_handler"))
    qa.calculate_answers(queries=queries, loader=loader, direction="tail")


Retrieving Results
~~~~~~~~~~~~~~~~~~

The handler caches the results until the ``calculate_answers(..)`` function is invoked again.
The QAHandler can output the calculated candidates and their aggregated scores, depending on the selected aggregation function with ``qa_handler.aggregation_function`` .
It can also output, for each candidate answer, the rules that predicted the candidate.


Independent of how data was loaded and how queries were defined (strs or idx's), outputs can be written to a file or obtained in Python and they can be formatted as idx's or strings.

**Outputting candidates and scores**

.. code-block:: python

    ...
    qa.calculate_answers(queries=queries, loader=loader, direction="tail")

    # output strings
    answers_str = qa.get_answers(as_string=True)
    # output idx's
    answers_idx = qa.get_answers(as_string=False)
    # write to file as string
    qa.write_answers(path="tail-answers_str.jsonl", as_string=True)
    # write to file as idx
    qa.write_answers(path="tail-answers_idx.jsonl", as_string=False)

Here **answers_str** and **answers_idx** are lists where **answers_str[i]** returns an ordered list of tuples for query **i**.
The tuples contain as first element the candidate entity (str or idx) and as second element the aggregated prediction score.


The files are of **jsonl** format (each line is a valid json) where each line corresponds to one query. They can be read line-wise and each line can be dumped with the Python json module.


**Outputting predicting rules**


If you want to output the predicting rules for every candidate you have to first set the ``qa_handler.collect_rules`` option to true.


.. code-block:: python

    ...
    opts.set("qa_handler.collect_rules", True)
    qa.set_options(options=opts.get("qa_handler"))
    qa.calculate_answers(queries=queries, loader=loader, direction="tail")

    # output strings
    rules_str = qa.get_rules(as_string=True)
    # output idx's
    rules_idx = qa.get_rules(as_string=False)
    # write to file as string
    qa.write_rules(path="tail-answers_str.jsonl", as_string=True)
    # write to file as idx
    qa.write_rules(path="tail-answers_idx.jsonl", as_string=False)

Here **rules_str** and **rules_idx** are lists of lists where **rules_str[i][j][k]** returns the k'th rule (idx or str, sorted according to confidences) for the j'th candidate answer
of the i'th query.

The files are in **jsonl** format and can be read as described above.

Filtering
~~~~~~~~~
The ``QAHandler`` can be configured with various options as described in the `config-default.yaml <https://github.com/symbolic-kg/PyClause/blob/master/clause/config-default.yaml>`_ .
The option ``qa_handler.filter_w_data`` will suppress any candidate that forms a true answer in the base dataset **data** of the loader.
If the loader additionally uses the filter argument while loading, this will likewise be used for filtering candidates.

.. code-block:: python

    ...
    # define data1, data2, etc..
    ...
   
    opts = Options()
    loader = Loader(opts.get("loader"))
    # filters with data2 automatically when "filter" is specified
    loader.load_data(data=data1, filter=data2)
    ...
    # filter with data1 when qa_handler option is activated; default: True
    opts.set("qa_handler.filter_w_data", True)









