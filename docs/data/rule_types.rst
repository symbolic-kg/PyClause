
Rule Types and Syntax 
=====================
PyClause uses rule type names and rule formatting from `this publication <https://link.springer.com/article/10.1007/s00778-023-00800-5>`_


Rule Syntax
~~~~~~~~~~~
To load rule strings they have to be represented in a particular format. An example B-rule string looks as follows:

.. code-block:: bash

   has_part(X,Y) <= member_meronym(X,A), hypernym(A,B), derivationally_related_form(Y,B)

First note that head and body are separated by **" <= "** (spaces are required). The head consists of one atom **rel(X,Y)** where **rel** is a relation string in the knowledge graph.
The head atom always contains **X** in the left slot and **Y** in the right slot. Alternatively for some rule types **X** or **Y** (not both) can be substituted with an entity string.


The atoms in the body are separated by **", "** (space required).

Supported Rule Types
~~~~~~~~~~~~~~~~~~~~

Let ``h, b1, b2, b3`` be relation tokens (strings).
Let ``cc,dd`` be entity tokens (strings).
Let ``X A B C D E F G H .. Y`` be variables.

**B Rules**

Examples:

.. code-block:: bash

   h(X,Y) <= b1(X,A), b2(A,Y)
   h(X,Y) <= b1(A,X), b2(A,Y)
   h(X,Y) <= b1(X,A), b2(A,B), b3(B,Y)

**U_c Rules**

Examples:

.. code-block:: bash

   h(X,cc) <= b1(X,dd)
   h(cc,Y) <= b1(Y,cc)
   h(cc,Y) <= b1(cc,Y)

   h(X,cc) <= b1(A,X), b2(A,dd)
   h(cc,Y) <= b1(Y,A), b2(dd,A)
   h(cc,Y) <= b1(A,Y), b2(A,dd)

   h(cc,Y) <= b1(A,Y), b2(A,B), b3(B,dd)

Note that it is allowed that **cc** is the same entity as **dd**.

**U_d Rules**

Examples:

.. code-block:: bash

   h(X,cc) <= b1(X,A)
   h(cc,Y) <= b1(Y,A)
   h(cc,Y) <= b1(A,Y)

   h(X,cc) <= b1(A,X), b2(A,B)
   h(cc,Y) <= b1(Y,A), b2(B,A)
   h(cc,Y) <= b1(A,Y), b2(A,B)

   h(c,Y) <= b1(A,Y), b2(A,B), b3(B,C)



**Zero (Z) Rules**
This rule type is used directed. The first rule only makes prediction in tail direction given a head=X and vice versa for the second rule.

Examples:

.. code-block:: bash

   h(X,cc) <= 
   h(cc,Y) <= 

**U_xxc Rules**
This rule type only has one body atom.

Examples:

.. code-block:: bash

   h(X,X) <= b1(X,dd) 
   h(X,X) <= b1(dd,X) 

**U_xxd Rules**
This rule type only has one body atom.

Examples:

.. code-block:: bash

   h(X,X) <= b1(X,A) 
   h(X,X) <= b1(A,X) 






















