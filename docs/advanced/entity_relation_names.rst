
Using External Entity/Relation Names
====================================

Some knowledge graphs use as entity and relation strings identifiers that are not human understandable, e.g., on WN18RR, the entity **italy** has the string identifier **"08801678"**.
If a mapping from identifiers to readable strings is provided, the identifiers can be substituted in the loader with the more readable strings.
After this action is performed, whenever any handler returns or writes output with the ``as_string=True`` argument, the readable format will be used.
If the input format to the handler is in string form, the readable versions have to be used.

We show an example here for the WN18RR dataset. The dataset, the rules, and the mapping are included in the library.
Assume the data is loaded in raw form on which also the rules have been learned.


.. code-block:: python

    from c_clause import QAHandler, Loader
    from clause.util.utils import read_map
    from clause import Options
    
    train = "data/wnrr/train.txt"
    rules = "data/wnrr/anyburl-rules-c5-3600"
    
    opts = Options()
    loader = Loader(options=opts.get("loader"))
    loader.load_data(train)
    loader.load_rules(rules)


Now the maps can be loaded and the strings in the index of the loader can be replaced. We show it here only for the entities.


.. code-block:: python

    entity_names_f = "data/wnrr/entity_strings.txt"
    entity_names = read_map(entity_names_f)

    # replace the currently used strings
    loader.replace_ent_strings(entity_names)
    # for relations:
    # loader.replace_rel_strings(rel_names)


This replaced all the strings in the index of the loader with the new strings. However note, there is no going back.
You cannot load the original ruleset anymore, for this you have to create a new loader.

Now, inputs and outputs are based on the new strings.

.. code-block:: python

    opts.set("qa_handler.filter_w_data", False)
    qa_handler = QAHandler(opts.get("qa_handler"))

    query = [("italy", "_has_part")]

    # head direction: answer query (?, _has_part, italy)
    qa_handler.calculate_answers(queries=query, loader=loader, direction="head")
    answers = qa_handler.get_answers(True)[0]

    for i in range(len(answers)):
        print(answers[i][0], answers[i][1])

    # returns:
    # europe 0.657
    # iberian_peninsula 0.333
    # scandinavian 0.333
    # north_america 0.28
    # balkans 0.28
    # ...

