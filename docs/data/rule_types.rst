
Rule Types and Syntax 
=====================
PyClause uses conventions for type names and rule formatting from `this publication <https://link.springer.com/article/10.1007/s00778-023-00800-5>`_ .


Rule Syntax
~~~~~~~~~~~

To load rules with the ``c_clause.Loader``  they must be represented in a structured syntax. An illustrative example of a B-rule string is as follows:

.. code-block:: bash

   has_part(X,Y) <= member_meronym(X,A), hypernym(A,B), derivationally_related_form(Y,B)

The **head** and the **body** of a rule are distinguished by the " <= " delimiter, with spaces required on both sides.

**Head**

The head contains a single atom in the format **rel(X,Y)**, where **rel** is a relation string in the knowledge graph.
The head atom may have **X** on the left and **Y** on the right. No other variables appear in the head. Depending on the rule type, **X** or **Y** —but not both— might be replaced with an entity string (a *constant*) or the **Y** is set to **X**.


**Body**

Atoms within the body are divided using a comma ", " with a required space after it. The two terms within an atom are separated by a "," with no spaces included. Body atoms can also include entitiy strings (constants). Possible variables and entities in an atom can be inferred by the following heuristic:

Given the ordering,

.. code-block:: python

   variables = ["A", "B", "C", "D", "E", "F", "G", "H", "I", ...],

the following holds for the body atoms:

- The atom at position **i=0** contains **variables[0]** and **X** or **Y**, depending on the rule type.
- Intermediate atoms, where **0<i<length-1** (**length** being the number of **body atoms**), include **variables[i-1]** and **variables[i]**.
- The final atom, at position **i=length-1**, involves **variables[i-1]** and an entity string or **Y**.

The ordering **within** a particular **body atom** is free and changing the positions will result in a distinct rule. Moreover, the head relation is allowed to be included in the body atoms:

.. code-block:: python

  rel1(X,Y) <= rel2(X,A), rel1(A,Y)
  rel1(X,Y) <= rel2(A,X), rel1(Y,A)

  rel1(X,ent_1) <= rel2(X,ent_2)
  rel1(X,ent_1) <= rel2(ent_2,X)

  rel1(X,Y) <= rel2(Y,X)
  rel1(X,Y) <= rel2(X,Y)

  rel1(X,Y) <= rel1(Y,X)

.. note::

   **Entity string format.** Entities (constants) within rules must exist in the knowledge graph of the loaded data. They should consist of at least two characters.

Supported Rule Types
~~~~~~~~~~~~~~~~~~~~
We show below the supported rule types. The examples follow the correct syntax, e.g., they could be parsed by the ``c_clause.Loader``.

- Let ``rel1, rel2, rel3, rel4`` be relation strings.
- Let ``cc, dd`` be entity strings.
- Let ``X A B C D E F G H .. Y`` be variable strings.

**B-Rules**

The head together with the body form a cycle when directions within the atoms are ignored. Any length supported. Examples:

.. code-block:: bash

   rel1(X,Y) <= rel1(Y,X)
   rel1(X,Y) <= rel1(X,A), rel1(A,Y)
   rel1(X,Y) <= rel2(X,A), rel3(A,Y)
   rel1(X,Y) <= rel2(A,X), rel3(A,Y)
   rel1(X,Y) <= rel2(X,A), rel3(A,B), rel4(B,Y)

**U_c Rules**

One entity constant in the head and one in the body (either for **X** or **Y**). Note that **Y** *must* be contained in the first atom if **X** is substituted. It is allowed that **cc** and **dd** are the same entity. Any length supported. Examples:

.. code-block:: bash

   rel1(X,cc) <= rel2(X,dd)
   rel1(cc,Y) <= rel2(Y,cc)
   rel1(cc,Y) <= rel2(cc,Y)

   rel1(X,cc) <= rel2(A,X), rel3(A,dd)
   rel1(cc,Y) <= rel2(Y,A), rel3(dd,A)
   rel1(cc,Y) <= rel1(A,Y), rel2(A,dd)

   rel1(cc,Y) <= rel2(A,Y), rel3(A,B), rel4(B,dd)

**U_d Rules**

One entity constant in the head no entity constant in the body. Note that the variables in the body follow the generic heuristic above for intermediate atoms without having a **Y** or entitiy in the last atom. Any length supported. Examples:

.. code-block:: bash

   rel1(X,cc) <= rel2(X,A)
   rel1(cc,Y) <= rel2(Y,A)
   rel1(cc,Y) <= rel2(A,Y)

   rel1(X,cc) <= rel2(A,X), rel3(A,B)
   rel1(cc,Y) <= rel2(Y,A), rel3(B,A)
   rel1(cc,Y) <= rel2(A,Y), rel3(A,B)

   rel1(c,Y) <= rel2(A,Y), rel3(A,B), rel4(B,C)



**Zero (Z)-Rules**

This rule type is directed and has no length. The first rule only makes prediction in tail direction given a head=X and vice versa for the second rule. It can not be used for triple scoring.

Examples:

.. code-block:: bash

   rel1(X,cc) <= 
   rel1(cc,Y) <= 

**U_xxc Rules**

This rule type only has one body atom.

Examples:

.. code-block:: bash

   rel1(X,X) <= rel2(X,dd) 
   rel1(X,X) <= rel2(dd,X) 

**U_xxd Rules**

This rule type only has one body atom.

Examples:

.. code-block:: bash

   rel1(X,X) <= rel2(X,A) 
   rel1(X,X) <= rel2(A,X) 






















