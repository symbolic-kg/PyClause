
Rules for Knowledge Graphs
==========================


Symbolic Rules
~~~~~~~~~~~~~~

The focus of PyClause lies on **association rules** or **definite clauses**, which we simply term **rules**. Consider the two example rules:

.. code-block:: bash

   isCitizenOf(X,Y) <= wasBornIn(X,A), hasCapital(Y,A)
   speaks(X,english) <= lives(X, london) 

Here *X, A, Y* are variables, *english*, *london* are entities, and *isCitizenOf, wasBornIn, hasCapital, speaks, lives* are relations.

.. note::

   From a predicate logic viewpoint, rule variables are assumed to be universally quantified. Furthermore, PyClause treats all terms implicitly under full object identity, i.e., :math:`X \neq A \neq Y` for the first rule and for the second :math:`X \neq london \neq english`.


We say that 

.. code-block:: bash

    diedIn(X,A), isLocatedIn(A,Y)
    lives(X, london) 

are the *rule bodies* and 

.. code-block:: bash

   isCitizenOf(X,Y)
   speaks(X,english)

are the respective rule heads.

A collection of rules is mostly associated with a knowlede graph (KG). We define a KG to be a collection of **(subject, rel, object)** triples, where *subject* and *object* are entities and *rel* is a relation. There is a direct correspondence between the atoms we used in the notation above and the KG triples, e.g., we can think of a ground atom **speaks(Anna, English)** as of a knowledge graph triple **(anna, speaks, english)**. 


Rule Confidences
~~~~~~~~~~~~~~~~
From a given KG we can calculate the **confidence** of a particular rule, sometimes also termed precision. The vanilla confidence of a rule is defined as the number of body groundings resulting in a true head (support) divided by the number of all existing body groundings. 

For the second rule, assume the KG from which we want to calculate the confidence is given as:

.. code-block:: bash

   speaks(lisa, english)
   speaks(marta, french)
   lives(lisa, london)
   lives(bob, london)
   lives(marta, london)
   lives(peter, paris)
   

The existing body groundings of the second rule are **lives(lisa, london), lives(bob, london), lives(marta, london)** but only the first one results in a head that is also true in the KG, **speaks(lisa, english)**, therefore, the confidence of the rule is 1/3. Please note that a body grounding for the first rule in the example would consist of two parts, e.g., **wasBornIn(marta, italy), hasCapital(italy, rome)**.


.. note::

   There exist various confidence adaptations, for instance, for the PCA confidence see the original `AMIE publication <https://resources.mpi-inf.mpg.de/yago-naga/amie/amie.pdf>`_ .



Rule Application/ Making Predictions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
We can use rules and their confidences to make predictions regarding triples or queries.

**Triple Classification/Scoring**

Assume we are given the triple 


.. note::

   More details about Rule Application and Aggregation can be found `here <https://arxiv.org/abs/2309.00306>`_ . 












