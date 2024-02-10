from c_clause import QAHandler, Loader
from clause.util.utils import get_base_dir, read_map
from clause import Options

## ** Example using external entity / relation names ***

entity_names_f = f"{get_base_dir()}/data/wnrr/entity_strings.txt"
train = f"{get_base_dir()}/data/wnrr/train.txt"
rules = f"{get_base_dir()}/data/wnrr/anyburl-rules-c5-3600"
entity_names = read_map(entity_names_f)

opts = Options()
loader = Loader(options=opts.get("loader"))
loader.load_data(train)
loader.load_rules(rules)
loader.replace_ent_strings(entity_names)
# if a relation map is also provided
# loader.replace_rel_strings(rel_names)

opts.set("qa_handler.filter_w_data", False)
qa_handler = QAHandler(opts.get("qa_handler"))

query = [("italy", "_has_part")]
# head direction: answer query (?, _has_part, italy)
qa_handler.calculate_answers(queries=query, loader=loader, direction="head")

answers = qa_handler.get_answers(True)
answers = answers[0]
for i in range(len(answers)):
    print(answers[i][0], answers[i][1])

# note: you now cannot load the ruleset with the loader anymore
# as it is based on the raw strings of the data