.. image:: https://raw.githubusercontent.com/symbolic-kg/PyClause/master/docs/logo/PyClause/proc/PyClause-transparent.png
   :align: left
   :width: 35%

.. raw:: html

   <br clear="all"/>


Welcome to the documentation of PyClause. PyClause allows you to use and learn symbolic rules (clauses) in a simple yet efficient manner by using Python. All usable options are well documented and can be stored in user defined configuration files. 


Feature Overview
================

By exclusively using Python, with PyClause you can:

- Learn rules with **AMIE**, **AnyBURL**, or our custom mining module **Torm** (``clause.Learner``)
- Calculate explanations for a given triple prediction (``c_clause.PredictionHandler``)
- Calculate query answering on the fly (``c_clause.QAHandler``)
- Perform triple scoring/classification on the fly (``c_clause.PredictionHandler``)
- Materialize input rules and calculate rule confidences/precision (``c_clause.RulesHandler``)
- Calculate ranking files for knowledge graph completion (``c_clause.RankingHandler``)
- Retrieve the target rules that predicted triples or candidates with ``c_clause.QAHandler``, ``c_clause.PredictionHandler`` or ``c_clause.RankingHandler``
- Evaluate ranking files for knowledge graph completion

Possible workflows with PyClause allow to:

- Load data KGs and rules solely from Python and serialize the results back to Python
- Load data KGs and rules from disk and write back the results to disk for efficiency
- Handle KG data and inputs as indexes, e.g., from numpy arrays, or with string names for the generation of readable examples
- Work with and store user defined configuration files, based on a `default-configuration file <https://github.com/symbolic-kg/PyClause/blob/master/clause/config-default.yaml>`_


.. toctree::
   :maxdepth: 2
   :hidden:
   :caption: Start

   Overview<self>
   start/installation
   start/quickstart
   start/downloads

.. toctree::
   :maxdepth: 2
   :hidden:
   :caption: Conceptual

   conceptual/rules

.. toctree::
   :maxdepth: 2
   :hidden:
   :caption: Preparation
   
   data/config
   data/loading_data
   data/loading_rules
   data/rule_types
   
   

.. toctree::
   :maxdepth: 2
   :hidden:
   :caption: Features
   
   feature/learning
   feature/query_answering
   feature/triple_classification
   feature/triple_explanations
   feature/ranking_generation
   feature/rule_materialization
   feature/rule_confidence
   

.. toctree::
   :maxdepth: 2
   :hidden:
   :caption: Special Topics
   
   advanced/entity_relation_names
   advanced/evaluation
   

.. toctree::
   :maxdepth: 2
   :hidden:
   :caption: Reference Manual

   reference
   



