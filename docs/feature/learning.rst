
Learning Rules
==============
The rules that can be processed with PyClause can originate from anywhere as long as they follow the corresponding syntax.
PyClause provides nevertheless wrapper functionalities that allow to access ``AMIE`` and ``AnyBURL`` from Python.
The learned rules can be readily loaded with the ``c_clause.Loader``.
Both learners can be fully configured with their original parameters by the PyClause config files or from Python with the ``clause.Options`` class.


PyClause ships the respective ``AnyBURL`` and ``AMIE`` binaries (.jars) which are compiled under Java 8. After installing PyClause the code examples below can 
be used immediately. Alternatively, the jar files can be built from source and they can also be used directly from the command line. We provide a special release for AMIE, see defails below.


Rule Mining with AMIE
~~~~~~~~~~~~~~~~~~~~~~~
For AMIE, we provide a special release version that uses some additional parameters which are set in the PyClause `default config <https://github.com/symbolic-kg/PyClause/blob/master/clause/config-default.yaml>`_ .
Documentation and compile instructions for the release can be found `here <https://github.com/dig-team/amie/tree/pyclause>`_ .


**Using AMIE with PyClause**

.. code-block:: python

    from clause import Learner, Options
    from c_clause import Loader

    path_train = f"path/to/train.txt"
    path_rules_output = f"path/rule-file.txt"

    options = Options()
    options.set("learner.mode", "amie")

    ## example parameters - choose any supported AMIE options under key "raw"
    # rule length (head+body atom)
    options.set("learner.amie.raw.maxad", 4)
    options.set("learner.amie.raw.mins", 2)
    # special syntax for enforcing -const to be used as flag
    options.set("learner.amie.raw.const", "*flag*")
    # rule length (head+body atom) for rules with constants
    options.set("learner.amie.raw.maxadc", 3) 

    learner = Learner(options=options.get("learner"))
    learner.learn_rules(path_data=path_train, path_output=path_rules_output)

    # load rules with loader
    loader = Loader(options.get("loader"))
    loader.load_data(data=path_train)
    loader.load_rules(rules=path_rules_output)


**Outputs and confidences**

The output rule file stored by AMIE complies with the format of PyClause. Note that opposed to the AnyBURL output, the statistics are based on the PCA confidence computations.
Therefore, the statistics and confidences will be different from the ones calculated with AnyBURL.


**Language bias**

The language bias for the used AMIE version within PyClause differs slightly from its original language bias. Consider the following rule:

.. code-block:: bash

   citizenOf(X,Y) <= hasChild(A,X), lives(X,Y), livesIn(A,Y)

This rule can potentially be mined with the standard AMIE version but not with the version used for PyClause. The difference to the syntax of the B-rules within PyClause
(see :doc:`../data/rule_types`) is that there exists two paths from **X** to **Y** within the body of the rule: A direct path and one path going throug **A**.  


Rule Learning with AnyBURL
~~~~~~~~~~~~~~~~~~~~~~~~~~
For AnyBURL we use the most recent AnyBURL release, AnyBURL-23-1. More information can be found on the official `homepage <https://web.informatik.uni-mannheim.de/AnyBURL/>`_ .


**Using AnyBURL with PyClause**


.. code-block:: python

    from clause import Learner, Options
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
    # num threads
     options.set("learner.anyburl.raw.WORKER_THREADS", 2)


    learner = Learner(options=options.get("learner"))
    learner.learn_rules(path_data=path_train, path_output=path_rules_output)

    # load rules with loader
    loader = Loader(options.get("loader"))
    loader.load_data(data=path_train)
    loader.load_rules(rules=path_rules_output)


Rule Mining with TORM
~~~~~~~~~~~~~~~~~~~~~
TORM is an **experimental** mining module that is built with the goal to efficiently mine short rules with constants. As it is currently in experimental state,
its usage behavior deviates slightly from the remaining parts of the library. Usable options can be found in the `default config <https://github.com/symbolic-kg/PyClause/blob/master/clause/config-default.yaml>`_ .


.. code-block:: python

    from clause import TormLearner, Options
    from clause import TripleSet
   
    path_train = "train.txt"
    path_rules_output = "torm-rules.txt"

    
    triples = TripleSet(path_train)

    options = Options()
    options.set("torm_learner.mode", "hybrid")

    ## some example options
    options.set("torm_learner.torm.b.length", 1)
    options.set("torm_learner.torm.uc.support", 15)
    options.set("torm_learner.torm.xx_ud.support", 15)
    options.set("torm_learner.torm.xx_uc.support", 15)
    options.set("torm_learner.torm.ud.support", 15)
    options.set("torm_learner.torm.z.support", 15)

    learner = TormLearner(options=options, targets=triples.rels, triples=triples)

    # mine rules
    learner.mine_rules(path_rules_output)

    # write the rules that have been mined to a file
    learner.rules.write(path_rules_output)

    loader = Loader(options.get("loader"))
    loader.load_data(data=path_train)
    loader.load_rules(rules=path_rules_output)
