
Rule Materialization
====================

The ``c_clause.RulesHandler`` can be used to calculate the materialization of given input rules and to calculate their statistics, that is,
the support and the number of predictions (body groundings in data). Note that the rules do not need to be loaded with the loader but are directly
passed as handler input. The loader only needs to load data. The materialization will be based on the **data** argument of the loader.

.. code-block:: python

    from c_clause import Loader
    from clause import Options

    data = [
        ("anna", "livesIn", "london"),
        ("anna", "learns", "english"),
        ("bernd", "speaks", "french")
    ]

    opts = Options()
    loader = Loader(opts.get("loader"))
    loader.load_data(data=data)


Materialize a rule set
~~~~~~~~~~~~~~~~~~~~~~
First, the handler option ``rules_handler.collect_predictions`` needs to be turned on (default: True) and
the handler is created. Other handler options can be found in the `config-default.yaml <https://github.com/symbolic-kg/PyClause/blob/master/clause/config-default.yaml>`_.

.. note::

    Calculating the materialization of large rule sets, especially for cyclical rules, can be memory and runtime expensive.
    If you only want to calculate statistics (see next section), turn off ``rules_handler.collect_predictions`` for reducing memory footprint and ensure that all threads
    are used (default: all). 

.. code-block:: python

    from c_clause import Loader, RulesHandler
    from clause import Options
    opts.set("rules_handler.collect_predictions", True)
    rh = RulesHandler(options=opts.get("rules_handler"))


Materialization is performed with ``RulesHandler.calculate_predictions(rules, loader)``. The calculations are based on the **data** argument of the loader.
The argument **rules** is either a list with rule strings or a file path. If a file path is specified the file can either 
contain a rule string on every line or each line is tab separated as the standard rule file syntax containing statistics.
In the latter case, the statistics do not have any effect.

.. code-block:: python

    # input from python
    rules = [
        "speaks(X,Y) <= learns(X,Y)",
        "speaks(X,english) <= livesIn(X,london)",
        "speaks(X,english) <= speaks(X,A)"
    ]
    # input from file
    # each line contains (no spaces)
    # either 'num_preds\t support\t conf\t rulestring' or just 'rulestring'
    rules = "my-rules.txt"

    rh.calculate_predictions(rules=rules, loader=loader)


Obtaining the outputs
~~~~~~~~~~~~~~~~~~~~~
Outputs can be retrieved directly:

.. code-block:: python

    # obtain directly as string  
    preds_str = rh.get_predictions(as_string=True)
     # obtain directly as idx
    preds_idx = rh.get_predictions(as_string=False)

Here **preds_str[i]** and **preds_idx[i]** are a list containing the materialized triples for the **i'th** input rule. 

Outputs can also be written to a file:

.. code-block:: python

    # write to file as flat KG
    # the file is a standard tab separated file containing triples
    # duplicates are removed, can be directly loaded as an input set with the loader
    rh.write_predictions(path="mat.txt", flat=True, as_string=True)

    # file is in jsonl format, each line can be read and dumped with Python json module
    # each dict contains as key the rule string and as value the materialized triples
    rh.write_predictions(path="mat.txt", flat=False, as_string=True)



