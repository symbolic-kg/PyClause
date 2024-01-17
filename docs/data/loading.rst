
Loading Data and Rules
======================


The ``c_clause.Loader`` class loads datasets (KGs) and rulesets.


Loading KG data
~~~~~~~~~~~~~~~
The loader is initialized with a dict from the options key "loader" and the ``Loader.load_data(..)`` function takes up to three possible arguments. It can receive KGs by loading them from a file, loading collections of string triples, or by loading data containing indices.


**Input datasets**

.. code-block:: python

   from clause import Options
   from c_clause import Loader

   opts = Options()
   loader = Loader(opts.get("loader"))

Any of the three will work but note that you can only load data once. If you want to use multiple dataset specifications you can use multiple loaders.

.. code-block:: python

   # data is the the base KG where rules will be applied on for all handlers
   # often also termed and used as 'train'
   loader.load_data(data="path/to/dataKG")
   loader2.load_data(data="path/to/dataKG", filter="path/to/filter")
   loader3.load_data(data="path/to/dataKG", filter="path/to/filter", target="path/to/target")


The parameter **data** is always required. It is the base dataset on which rules are applied for all the handlers.

**Filter** can be used as an set of triples that automatically filters out candidates calculated by ``c_clause.RankingHandler`` and ``c_clause.QAHandler``.

**Target** is only needed when creating ranking files with the ``c_clause.RankingHandler`` (often termed the test set).


**Data Types**

There are three possibilities of loading the specified input datasets, from path, from python as strings, or from python as indices.

 












