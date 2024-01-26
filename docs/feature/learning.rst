
Learning Rules
==============
The rules that can be processed with PyClause can originate from anywhere as long as they follow the corresponding syntax for rule loading.
PyClause provides nevertheless wrapper functionalities that allows to access the ``Amie`` and ``AnyBURL`` rule miner/learner from Python.
The learned rules can be readily loaded with the ``c_clause.Loader``. Both learners can be fully configured with their original parameters by the PyClause config files or from Python with the ``clause.Options`` class.


PyClause ships the respective ``AnyBURL`` and ``Amie`` binaries. If you have any trouble with using the rule learning module, read under **Java requirements** below.


Rule Learning from Python
~~~~~~~~~~~~~~~~~~~~~~~~~

**Amie example**

.. code-block:: python

    from clause import Learner, Options
    from clause.util.utils import get_base_dir
    from c_clause import Loader

    path_train = f"path/to/train.txt"
    path_rules_output = f"path/rule-file.txt"

    options = Options()

    options.set("learner.mode", "amie")
    # example parameters - choose any supported AMIE parameter under "...raw."
    options.set("learner.amie.raw.maxad", 4)
    options.set("learner.amie.raw.minc", 0.0001)
    options.set("learner.amie.raw.minpca", 0.0001)
    options.set("learner.amie.raw.minhc", 0.0001)
    options.set("learner.amie.raw.mins", 2)
    # special syntax for enforcing -const to be used as flag without value
    options.set("learner.amie.raw.const", "*flag*")
    options.set("learner.amie.raw.maxadc", 2) 

    learner = Learner(options=options.get("learner"))
    learner.learn_rules(path_data=path_train, path_output=path_rules_output)

    # load rules with loader
    loader = Loader(options.get("loader"))
    loader.load_data(data=path_train)
    loader.load_rules(rules=path_rules_output)


**AnyBURL example**

.. code-block:: python

    from clause import Learner, Options
    from clause.util.utils import get_base_dir
    from c_clause import Loader

    path_train = f"path/to/train.txt"
    path_rules_output = f"path/rule-file.txt"

    options = Options()

    options.set("learner.mode", "anyburl")
    # learning time
    options.set("learner.anyburl.time", 30)
    # set any raw AnyBURL parameter under "... .raw"
    # max body atoms of B-rules
    options.set("learner.anyburl.raw.MAX_LENGTH_CYCLIC", 5)

    learner = Learner(options=options.get("learner"))
    learner.learn_rules(path_data=path_train, path_output=path_rules_output)

    # load rules with loader
    loader = Loader(options.get("loader"))
    loader.load_data(data=path_train)
    loader.load_rules(rules=path_rules_output)


Config .yaml Example
~~~~~~~~~~~~~~~~~~~~


Java Requirements
~~~~~~~~~~~~~~~~~


Amie vs AnyBURL Rule types
~~~~~~~~~~~~~~~~~~~~~~~~~~