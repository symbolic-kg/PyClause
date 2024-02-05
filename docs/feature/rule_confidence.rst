Rule Confidence/Precision
=========================


The ``c_clause.RulesHandler`` can also be used for calculating and writing the statistics of the input rules, that is, the rule support and the
overall number of body groundings, which is termed num_predictins within PyClause. Together they can be used to calculate the confidence of a rule
which is also termed the precision in some contexts.

The explanations of the last section apply, i.e., the handlers ``calculate_predictions(rules, loader)`` function is invoked
after loading the data with the laoder. The calculations are based on the loader's **data** argument. Then 
the statistics can be obtained: 

.. code-block:: python

    stats = rh.get_statistics()

The variable **stats** at position **i** contains a list with two elements, the first element is the number of predictions
and the second element is the support, for input rule **i**.

They can also be written to a file. This will create a standard rule input file, i.e., tab separated statistics followed by the rule string
on each line. It can directly be laoded into the loader as an input rule set. 

.. code-block:: python

    rh.write_statistics(path="my-calculated-rules.txt")






