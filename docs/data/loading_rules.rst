
Loading Rules
=============
After data is loaded into the ``c_clause.Loader``, it can load a ruleset which is required when using ``c_clause.QAHandler, c_clause.RankingHandler, c_clause.PredictionHandler``.
A conceptual overview of how rules are understood and used in the scope of PyClause can be found in the tutorial :doc:`../conceptual/rules`. Supported rule types and the required syntax
can be found in the section :doc:`rule_types`.


Load Rules into Loader
~~~~~~~~~~~~~~~~~~~~~~

There exist three possibilities of loading rules into the the loader with the ``Loader.load_rules(...)`` function. All of them require rules to be represented in a human understandable string format.
Additionally, rules need to be assigned with their support and number of body groundings (num_predictions) and in two cases also with their confidence.
If the rules are only given in integer idx form, PyClause provides a Python utility to translate them into their string format. 

**1) Loading from list of rule strings and list of stats**

.. code-block:: python

   from c_clause import Loader
   from clause import Options

   opts = Options()
   loader = Loader(options=opts.get("loader"))

   dataset = [["lisa", "knows", "max"], ["max", "likes", "john"]]
   loader.load_data(data=dataset)

   rules = [
      "knows(X,Y) <= knows(Y,X)",
      "knows(X,lisa) <= likes(X,lisa)"
   ]
   # for each rule in the list rules, a list or tuple containing num_predictions, support
   # num_predictions: number of all body groundings
   # support: body groundings with a correct head grounding 
   stats = [
      [20, 10],
      [25, 20],
   ]

   loader.load_rules(rules=rules, stats=stats)

**2) Loading from path**

Rules can also be loaded from a file:

.. code-block:: python

   loader.load_rules(rules="path/to/rules.txt")

The file format should correspond with the AnyBURL output format. This means, each line is **tab separated** like this ``f"{num_pred}\t{support}\t{conf}\t{rule string}"``.
For example, *rules.txt*:

.. code-block:: bash

   20   10  0.5 knows(X,Y) <= knows(Y,X)
   25   20  0.8 knows(X,lisa) <= likes(X,lisa)

Note that the confidence is recomputed by PyClause and therefore the third column does not have any internal effect.

**3) Loading from rule lines**

The identical line format of a rule file can also be directly passed from Python:

.. code-block:: python

   rules = [
      "20\t10\t0.5\tknows(X,Y) <= knows(Y,X)",
      "25\t20\t0.8\tknows(X,lisa) <= likes(X,lisa)"
   ]

   loader.load_rules(rules=rules)

.. note::

   **Loading a new ruleset.** While data can only be loaded once, when you want to load a new ruleset you can invoke the ``load_rules()`` function again. The old ruleset is deleted.


**Rules in idx representation**


Translating rules from idx's to string format requires an entity and relation index that maps integers to strings.
See an example below for translating B-rules. An example for all rule types is given `here <https://github.com/symbolic-kg/PyClause/blob/master/examples/demo-idx-rules.py>`_ .

.. code-block:: python

   from clause import RuleTranslator

   entity_index = ["ent_1", "ent_2", "ent_3"]
   relation_index = ["rel_1", "rel_2", "rel_3", "rel_4", "rel_5", "rel_6"]

   translator = RuleTranslator(idx_to_ent=entity_index, idx_to_rel=relation_index)

   # specify 2 cyclical (b-rules) rules
   b_rels = [[0,1,2,3], [3,2]]
   # first direction element is always True
   b_dirs = [[True, False, False, False], [True, False]]

   rules = translator.translate_b_rules(relations=b_rels, directions=b_dirs)
   print(rules)
   # out:
   # ['rel_1(X,Y) <= rel_2(A,X), rel_3(B,A), rel_4(Y,B)', 'rel_4(X,Y) <= rel_3(Y,X)']
   




Writing Rules and Retrieving Rule Index
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


The loader can also write back the ruleset to a file with  ``loader.write_rules(path)``. This can be used to store subsets of rules. For instance, the loader could only load one parcticular rule type (see below)
and subsequently writing the rules will only contain this rule type in the output file. Likewise ``loader.get_rules()`` returns the loaded rulset with the rule statistics. It can be processed and loaded back with the loader.
The function ``loader.rule_index()`` provides a mapping that assigns each string rule a numeric idx. Both function can be used to obtain the global index that assigns integer idx's to string rules, i.e., the ordering is the same.


Loading Options
~~~~~~~~~~~~~~~

**Loading constraints**

By using the ``Loader`` options the loader can be configured to ignore certain rules and rule types. It can also modify the confidence computation of the rules.
It is likewise possible to subset/modify an already loaded ruleset by updating the options with ``loader.set_options(..)`` and subsequently invoking ``loader.update_rules()``.
The full list of options can be found in the `config-default.yaml <https://github.com/symbolic-kg/PyClause/blob/master/clause/config-default.yaml>`_ .



.. code-block:: python

    from c_clause import Loader
    from clause import Options

    dataset = [["lisa", "knows", "max"], ["max", "likes", "john"]]

    rules = [
        "knows(X,Y) <= knows(Y,X)",
        "knows(X,lisa) <= likes(X,lisa)",
        "knows(X,max) <= likes(X,max)"
    ]

    stats = [
       [20, 10],
       [25, 20],
       [25, 5],

    ]
    opts = Options()
    # ignores the first rule when loading
    opts.set("loader.load_b_rules", False)

    opts.set("loader.load_u_c_rules", True)
    ## add 10 false predictions to confidence computation
    opts.set("loader.c_num_unseen", 10)

    # ignores the last rule as 5/25 is smaller than 0.3
    opts.set("loader.c_min_conf", 0.3)
    
    loader = Loader(options=opts.get("loader"))
    loader.load_data(data=dataset)

    loader.load_rules(rules=rules, stats=stats)

**Resetting options**

Using the ``Loader.set_options(...)`` one can also, e.g., after loading data, reset the loader options. This will not affect the already loaded rules. But it can be used to load the same or another ruleset with different constraints.
For updating the **currently** loaded ruleset based on the newly set loader options read below.

.. code-block:: python

    ### 
    ### construct loader with options, load data etc..
    ###

    loader.load_rules(rules=ruleset) ##load rules and do something with it

    # change some options
    opts.set("loader.load_u_c_rules", False)
    opts.write("experiment2.yaml")
    # change loader options
    loader.set_options(opts.get("loader"))
    # load new ruleset ignoring U_c rules; old ruleset in loader is deleted
    loader.load_rules(rules=ruleset)

**Updating the currently loaded ruleset after resetting options**

After the loader options are changed, the updated options can directly applied to the currently loaded rule set. With the ``Loader.update_rules()``.
This will update/filter the currently loaded ruleset in regard to rule application performed with the different handlers. It allows to only load the rules once
but to perform multiple experiments with different rulesets. The original rule set is retrieved, it is also possible to go back to the original rule set
by modifying the options accordingly.

.. code-block:: python

    ### 
    ### construct loader with options, load data etc..
    ###

    loader.load_rules(rules=ruleset)

    # now do something, e.g., calculate a ranking with RankingHandler and the loader
    # see feature section

    # change some option
    opts.set("loader.load_u_c_rules", False)
    opts.set("loader.b_num_unseen", 100)
    loader.set_options(opts.get("loader"))
    # update the rules
    # this will modify b-rules and ignore c-rules when the loader is used for application
    loader.update_rules()
   
   # now calculate a ranking with RankingHandler, which is calculated without c-rules


.. note::

   Writing the rules and retrieving the rules with ``Loader.get_rules()`` or ``Loader.rule_index()`` will always be based on the full rule set that has been loaded independent of the updating.
   Also the inter idx's of the rules when not outputting strings are based on the global index.

Custom Rule Confidences
~~~~~~~~~~~~~~~~~~~~~~~
PyClause internally re-computes rule confidences for each rule type as ``conf=support/ (num_preds+r_num_unseen)`` where **r_num_unseen** is a configurable option in config-default.yaml for some rule type **r**.
The confidence specification in the file/input is not used. If you want to use your own custom confidence you have to specifiy **num_predictions** and **support** when loading the rules. Note that **r_num_unseen** is 5 in the config-default for every rule type.
In cases, where you only have one custom confidence you can do it like in the following example:

.. code-block:: python

   from c_clause import Loader
   from clause import Options

   opts = Options()
   # allow for every rule type (here only B-rules) that custom confidences can be loaded
   opts.set("loader.b_min_preds", -1)
   opts.set("loader.b_min_support", -1)
   opts.set("loader.b_num_unseen", 0)
   opts.set("loader.b_min_conf", -1)

   loader = Loader(opts.get("loader"))
   dataset = [["lisa", "knows", "max"], ["max", "likes", "john"]]
   loader.load_data(data=dataset)

   ## confidence to set is 0.73
   ## set num_pred = X
   ## set support X*0.73 such that the result is an integer if it is smaller than 0 it will be rouned to 0.
   rules = [
        "100\t73\t0.0\tknows(X,Y) <= knows(Y,X)",
    ]

   loader.load_rules(rules=rules)


















