
Feature Overview
================

By exclusively using Python, with PyClause you can:

- Learn rules with **AMIE**, **AnyBURL**, or our custom mining module **Torm** (``clause.Miner``)
- Calculate explanations for a given triple prediction (``c_clause.PredictionHandler``)
- Calculate query answering on the fly (``c_clause.QAHandler``)
- Perform triple scoring/classification on the fly (``c_clause.PredictionHandler``)
- Calculate ranking files for knowledge graph completion (``c_clause.RankingHandler``)
- Evaluate ranking files for knowledge graph completion
- Materialize input rules and calculate rule confidences/precistion (``c_clause.RulesHandler``)

All the handlers can either retrieve and send data and rules from and to python or load and store them from and to disk to avoid data serialization between C++ and Python.
The handlers can handle process (sets of knowledge graph triples) as raw string tokens for easy example generation and data understanding. Additionally, data can be handled 
as numerical idx's, e.g., coming from numpy arrays.
PyClause uses **yaml** configuration files that apply to rule learning and all the handlers. Every possible option is well documented in a `default-configuration file <https://github.com/symbolic-kg/PyClause/blob/master/clause/config-default.yaml>`.







