
Loading Data 
============


The ``c_clause.Loader`` class loads datasets (knowledge graphs) and rulesets. At first data has to be loaded which creates an internal index that maps entity and relation string tokens to indices.


Input datasets
~~~~~~~~~~~~~~~

The loader is initialized with a dict from the options key "loader" and the ``Loader.load_data(..)`` function takes up to three possible arguments.

.. code-block:: python

   from clause import Options
   from c_clause import Loader

   opts = Options()
   loader = Loader(opts.get("loader"))

Any of the three will work.

.. code-block:: python

   # data is the the base KG where rules will be applied on for all handlers
   # often also termed and used as 'train'
   loader.load_data(data="path/to/dataKG")

   loader2.load_data(data="path/to/dataKG", filter="path/to/filter")
   loader3.load_data(data="path/to/dataKG", filter="path/to/filter", target="path/to/target")

- The dataset **data** is always required. It is the base knowledge graph on which rules are applied for all the handlers.
- **Filter** can be used as an set of triples that automatically filters out candidates calculated by ``c_clause.RankingHandler`` and ``c_clause.QAHandler``.
- **Target** is only needed when creating ranking files with the ``c_clause.RankingHandler`` (often termed the test set).


Note that you can only load data once. If you want to use multiple dataset specifications you can use multiple loaders.

Data Types
~~~~~~~~~~~~~~~

There are three possibilities of how to specify the input datasets. From path (as shown above), from python as strings, or from python as indices.

**1) From path**

**Data**, **filter** and **target** arguments have to be file paths to files containing **tab separated** triples of string tokens, e.g., **train.txt**:

.. code-block:: bash

   lisa	knows	max
   max	likes	jon


**2) From Python as strings**

**Data**, **filter** and **target** arguments have to be Python lists of string triples. 

.. code-block:: python

   dataset = [["lisa", "knows", "max"], ["max", "likes", "john"]]

.. note::

   For the previous two methods the loader will create an internal index that maps entity and relation strings to integer indices.
   If you have such a mapping already and later want to work with indices according to this mapping you can force the loader to use your index. Simply execute before loading data **loader.set_entity_index(index)** and **loader.set_relation_index(index)**. The argument index is a list of strings that maps idx's to strings by list[i]=string.

**3) From Python as indices**
It is also possible to use lists or numpy arrays containing indices. PyClause will, however, always need an internal index that  maps indices to token strings. Even if the user works with indices, PyClause always allows to output results in human readable string representations.

.. code-block:: python

   from c_clause import Loader
   from clause import Options
   import numpy as np

   options = Options()
   loader = Loader(options.get("loader"))

   # maps entities 
   # 0: "lisa"
   # 1: "max"
   # 2: "john"
   entity_index = ["lisa", "max", "john"]
   # maps relations
   # 0: knows
   # 1: likes
   relation_index = ["knows", "likes"]

   # set entity/relation index; should only be done once
   loader.set_entity_index(entity_index)
   loader.set_relation_index(relation_index)

   # (lisa knows max)
   # (max likes john)
   triples = np.array(
       [
           [0, 0, 1],
           [1, 1, 2]
       ]
   )
   loader.load_data(dat=triples)





 











