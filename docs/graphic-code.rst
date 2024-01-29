.. code-block:: python
   :linenos:

   from clause import Options, Learner

   # learn rules with AMIE
   opts = Options("my-config.yaml")
   learner = Learner(opts.get("learner"))
   learner.learn_rules("kg.txt", "rules.txt")

   ##########################################

   from c_clause import QAHandler, Loader

   # perform query answering
   qa = QAHandler(opts.get("qa_handler"))
   loader = Loader(opts.get("loader"))
   loader.load_data("kg.txt")
   loader.load_rules("rules.txt")
   query = [("anna", "worksFor")]
   qa.calculate_answers(query, loader)


.. code-block:: yaml
   :linenos:

   learner:
     type: amie
     raw:
        maxad: 3
        minc: 0.1
     ...
   qa_handler:
     num_top_rules: 5
     topk: 100
     ...


