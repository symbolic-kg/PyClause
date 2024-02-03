from clause import RuleTranslator
from c_clause import Loader

# ***Example for translating rules in idx format to strings***

entity_index = ["ent_1", "ent_2", "ent_3"]
relation_index = ["rel_1", "rel_2", "rel_3", "rel_4", "rel_5", "rel_6"]

loader = Loader(options={})

# you don't need to this when you load proper data
loader.set_relation_index(relation_index)
loader.set_entity_index(entity_index)
loader.load_data(data=[["ent_1", "rel_1", "ent_2"]])

translator = RuleTranslator(idx_to_ent=entity_index, idx_to_rel=relation_index)

all_rules = []

# specify 2 cyclical (b-rules) rules
b_rels = [[0,1,2,3], [3,2]]
b_dirs = [[True, False, False, False], [True, False]]

rules = translator.translate_b_rules(relations=b_rels, directions=b_dirs)
print(rules)

all_rules.extend(rules)

# specify some rules with 2 constants (uc-rules) rules
c_rels = [[0,3], [4,4,0,2]]
c_dirs = [[True, False], [True, True, True, False]]
c_const =  [[0, 0], [1, 0]]
c_leftC =  [0,1]

rules = translator.translate_c_rules(relations=c_rels, directions=c_dirs, constants=c_const, leftC=c_leftC)
print(rules)
all_rules.extend(rules)


# specify some rules with 1 constants (ud-rules) rules
d_rels = [[0,3], [4,4,0,2]]
d_dirs = [[True, False], [True, False, True, False]]
d_const =  [0, 2]
d_leftC =  [0, 1]
rules = translator.translate_d_rules(relations=d_rels, directions=d_dirs, constants=d_const, leftC=d_leftC)
print(rules)
all_rules.extend(rules)


# specify some z(ero) rules 
z_rels = [0, 3]
z_const = [2, 0]
z_leftC = [True, False]
rules = translator.translate_z_rules(relations=z_rels, constants=z_const, leftC=z_leftC)
print(rules)
all_rules.extend(rules)


# specify some u_xxd rules 
uxxd_rels = [[0, 1], [4, 0]]
uxxd_dirs = [[True, True], [True, False]]
rules = translator.translate_u_xxd_rules(relations=uxxd_rels, directions=uxxd_dirs)
print(rules)
all_rules.extend(rules)

# specify some u_xxc rules 
uxxc_rels = [[0, 1], [4, 0]]
uxxc_dirs = [[True, True], [True, False]]
uxxc_constants = [0, 1]
rules = translator.translate_u_xxc_rules(relations=uxxc_rels,  directions=uxxc_dirs, constants=uxxc_constants)
print(rules)
all_rules.extend(rules)

# just use 1,1 for stats here
loader.load_rules(rules=all_rules, stats=[[1,1]]*len(all_rules))

for rule in loader.get_rules():
    print(rule.split("\t")[3])








