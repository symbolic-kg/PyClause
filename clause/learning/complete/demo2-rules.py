import sys


from torm import Torm
from triples import TripleSet

from rules import RuleSet
from ruleparser import RuleReader

import c_clause




if __name__ == '__main__':

    path_train = "E:/exp/data/yago/train.txt"
    path_valid = "E:/exp/data/yago/valid.txt"
    path_test  = "E:/exp/data/yago/test.txt"

    path_rules_in1 = "E:/exp/data/akbc-rules/yago-anyburl-rules-c3-3600"
    path_rules_in2 = "E:/exp/pyclause/yago/rules-pctorm-yago-uc-ud-z-xx"
    
    path_rules_out= "E:/exp/pyclause/yago/rules-anyburl-yago-b"

    triples = TripleSet(path_train)

    # load an anyburl ruleset and remove all rules that are not B rules
    rules_b = RuleSet(triples.index)
    rule_reader = RuleReader(rules_b)
    rule_reader.read_file(path_rules_in1)
    rules_b.retainOnly("B")

    # load an anytorm ruleset that contains not B rules
    rules_other = RuleSet(triples.index)
    rule_reader = RuleReader(rules_other)
    rule_reader.read_file(path_rules_in2)

    # and the anyburl rules to the other rule set
    rules_other.add_ruleset(rules_b)

    # write the combined ruleset to a file
    rules_b.write(path_rules_out)
