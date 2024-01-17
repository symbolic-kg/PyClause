
Options and Configs
======================

The ``clause.Options`` class provides functionality to load and write ``yaml`` configuration files as well as modifying options from Python.

Options are set from Python by separating nested parameters with a dot. Only values for keys can be modified that exist in the `config-default.yaml <https://github.com/symbolic-kg/PyClause/blob/master/clause/config-default.yaml>`_. An exception are options under a key "raw" which allows to set completely new key, value pairs. This is used for rule learning with AMIE or AnyBURL and is shown in the learning section.


.. code-block:: python
   
   from clause import Options

   # loads config-default.yaml into opts
   opts = Options()

   # overwrites values
   opts.set("learning.mode", "anyburl")
   opts.set("ranking_handler.topk", 100)

   # returns a dict with string values
   opts.get("ranking_handler")
   # returns value
   opts.get("ranking_handler.topk", as_string=False)

   # writes modified values to a configuration file
   # ignores default values
   opts.write("my-updated-config.yaml") 


Alternatively, a user defined config can directly be loaded.

.. code-block:: python
   
   from clause import Options

   # loads config-default.yaml and overwrites with
   # existing values in "my-config.yaml"
   opts = Options("my-config.yaml")

   opts.set("ranking_handler.topk", 50)

   # writes all values that are different from config-default.yaml
   opts.write("my-config.yaml") 

 


   











