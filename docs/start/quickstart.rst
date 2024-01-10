
Quickstart
==========

.. code-block:: python
   from c_clause import QAHandler, Loader
   from clause.config.options import Options

   # ***Example for Query Answering***

   # define a knowledge graph
   # alternatively, specify file path or use arrays + indices
   data = [
      ("anna", "livesIn", "london"),
      ("anna", "learns", "english"),
      ("bernd", "speaks", "french")
   ]
   # define rules, or specify file path
   rules = [
      "speaks(X,Y) <= learns(X,Y)",
      "speaks(X,english) <= livesIn(X,london)",
      "speaks(X,english) <= speaks(X,A)"
   ]
   # define rule stats: num_preds, support
   stats = [
      [20, 10],
      [40, 35],
      [50, 5],
   ]
   # define options, handlers and load data
   opts = Options()
   opts.set("qa_handler.aggregation_function", "noisyor")

   loader = Loader(options=opts.get("loader"))
   loader.load_data(data)
   loader.load_rules(rules=rules, stats=stats)

   qa = QAHandler(options=opts.get("qa_handler"))
   # define query: (anna, speaks, ?); alternatively, use indices
   queries = [("anna", "speaks")]
   qa.calculate_answers(queries=queries, loader=loader, direction="tail")
   # outputs [("english", 0.867 )] 
   print(qa.get_answers(as_string=True)[0])

   # define query: (?, speaks, english); alternatively, use indices
   queries = [("english", "speaks")]
   qa.calculate_answers(queries=queries, loader=loader, direction="head")
   # outputs [('anna', 0.867), ('bernd', 0.001)] 
   print(qa.get_answers(as_string=True)[0])
