
Rules for Knowledge Graphs
==========================

This tutorial briefly defines rules and covers how they can be used for knowledge graphs. It does not cover how rules are learned from the data for which we refer to the respective `AMIE <https://luisgalarraga.de/docs/amie3.pdf>`_ or `AnyBURL <https://link.springer.com/article/10.1007/s00778-023-00800-5>`_ publications. 


Symbolic Rules
~~~~~~~~~~~~~~

The focus of PyClause lies on **association rules** or **definite clauses**, which we simply term **rules**. Consider the two example rules:

.. code-block:: bash

   isCitizenOf(X,Y) <= wasBornIn(X,A), hasCapital(Y,A)
   speaks(X,english) <= lives(X, london) 

Here *X, A, Y* are variables, *english*, *london* are entities, and *isCitizenOf, wasBornIn, hasCapital, speaks, lives* are relations.

The first rule can be understood as: If somebody is born at a city which is the capital of a country the person is also citizen of that country. The second one simply says that a person speaks english if she lives in London.

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

A collection of rules is mostly associated with a knowlede graph (KG). We define a KG to be a collection of **(subject, rel, object)** triples, where *subject* and *object* are entities and *rel* is a relation. There is a direct correspondence between the atoms we used in the notation above and the KG triples, e.g., we can think of a ground atom **speaks(anna, english)** as of a knowledge graph triple **(anna, speaks, english)** we will simply refer to *triples* for both notations. 


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
We can use rules and their confidences to make predictions regarding new triples or queries.

.. note::

   Rule application in our context is understood in the context of association rules for handling large datasets. It neither infers model theoretic entailment nor does it perform probabilistic reasoning. Instead, it exclusively involves "grounding the rules once" given a KG.
   In a logic programming context it can be described by using the immediate consequence operator.      


**Triple Classification/Scoring**

Assume we are given a KG, a rule, and the unseen target triple **isCitizenOf(marta,italy)**. We use the first rule from the start example:

.. code-block:: bash

   0.787 isCitizenOf(X,Y) <= wasBornIn(X,A), hasCapital(Y,A)
    

The value 0.787 is the rule confidence (real example from the Yago dataset). Now we want to check if the rule predicts our target triple.

First, we substitute **X=marta** and **Y==italy**. Then we validate if the atoms in the rule body **wasBornIn(marta, A)** and **hasCapital(italy, A)** can be satisfied by the KG. That means, we look for an entity **e** such that the triples **wasBornIn(marta, e)** and **hasCapital(italy, e)** exist in the KG. In other words, given the head variable substitution, we look for a body grounding of the KG. If we find one, we say that the rule predicts the target triple.

We can now assign the rule confidence 0.787 as the confidence of the prediction of the target triple. But what if multiple distinct rules with different confidences predicted the target triple? This will be discussed in the aggregation section below.


**Candidate Prediction for Queries**

Now assume we are given a query **isCitizenOf(?, italy)**, we may interpret this as the question *"Who is citizen of Italy"*. We use the term *head query* here as the **?** is positoned at the head slot and we could also form tail queries. We use the same rule from above as an example:

.. code-block:: bash

   0.787 isCitizenOf(X,Y) <= wasBornIn(X,A), hasCapital(Y,A)

We want to predict candidate entities for the head query, given our rule and the KG. We do this by searching for triples **isCitizenOf(e', italy)** that are predicted by the rule (or some set of rules).

First we only subsitute **Y=italy** and again check if we find body substitutions, but now we also have to search over possible substitutions for X. Therefore, we search for entities **m=X** and  **e=A** such that the triples **wasBornIn(m, e)** and **hasCapital(italy, e)** exist in the KG. If we find such a pair **(m, e)**, we say that **m** is a candidate prediction of our rule.

Again we can assign the rule confidence as the confidence of the candidate prediction. Also in this case the question arises what the prediction confidence will be if multiple rules predict the candidate, which is discussesd in the aggregation section below. 


Confidence Aggregation for Predictions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
We assume now that the triple **isCitizenOf(marta,italy)** was predicted by two rules with two distinct confidences:

.. code-block:: bash

   0.787   isCitizenOf(X,Y) <= wasBornIn(X,A), hasCapital(Y,A)
   0.613   isCitizenOf(X,italy) <= lives(X, rome) 

The following explanations apply likewise to query prediction and to triple prediction.

We want to obtain a final confidence value for the prediction. 

The easiest possibility is to simply assign the highest of the two confidences (max-aggregation). An alternative is the noisy-or product :math:`1-(1-0.787)(1-0.613)=0.918`.

For creating candidate rankings we also can order the candidates lexicographically with respect to the confidences of the predicting rules (maxplus-aggregation). For instance, if **bernd** is predicted to live in **italy** by rules with following confidences **[0.787, 0.543]** than we would order **marta** on top.

.. note::

   Supported aggregation functions in PyClause can be found in the `default-configuration file <https://github.com/symbolic-kg/PyClause/blob/master/clause/config-default.yaml>`_ .
   More details about Rule Application and Aggregation can be found `here <https://arxiv.org/abs/2309.00306>`_ . 



Triple Explanations
~~~~~~~~~~~~~~~~~~~
Now assume we are interested in the question: What are good reasons in the KG why **marta** should live in **italy**?
We can use the mechanics that were introduced above to form an **explanation** for the triple **isCitizenOf(marta,italy)**.
Note that we assume here that the rules that we have are learned from the KG itself, e.g., they reflect regularities of the data.

First, we look for rules that predict the target triple, e.g., the two rules from above. 
Then, we additionally track the body groundings of the rule that lead to the prediction. Assume our KG is given as

.. code-block:: bash

   wasBornIn(marta, rome)
   lives(marta, rome)
   hasCapital(italy, rome)
   speaks(marta, french)
   speaks(marta, italian)
   lives(bernd, paris)
   speaks(bern, german)

We can now pick the rule with the highest confidence and loosely form a (best) **explanation** based on the rule and its body groundings:


.. code-block:: bash

   Reasons why Marta should be citizen of Italy are the triples:

   wasBornIn(marta,rome)
   hasCapital(italy, rome)

   together with the rule:

   isCitizenOf(X,Y) <= wasBornIn(X,A), hasCapital(Y,A)



.. note::

   The core concept is based on abductive reasonining and experimental results regarding the explainability of embedding models can be `found here <https://www.ijcai.org/proceedings/2022/0391.pdf>`_ .
















