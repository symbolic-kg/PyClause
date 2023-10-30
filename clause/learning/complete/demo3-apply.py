from torm import Torm
from triples import TripleSet

from rules import RuleSet
from ruleparser import RuleReader

import c_clause




if __name__ == '__main__':

    path_train = "E:/exp/data/yago/train_masked.txt"
    path_valid = "E:/exp/data/yago/valid_masked.txt"
    path_test  = "E:/exp/data/yago/test_masked.txt"

    path_rules_input = "E:/exp/pyclause/yago/rules-nanytormx-b-uc2-ud-z-xx"

    path_preds_output = "E:/exp/pyclause/yago/preds-nanytormx-b-uc2-ud-z-xx"



    # create a ranking with pyclause core components
    # determine the setting to be used for creating the ranking
    options = {
        # ranking options
        "aggregation_function": "maxplus",
        "num_preselect": "10000000",
        "topk": "100",
        "filter_w_train": "true",
        "filter_w_target": "true",
        "disc_at_least":"10", ## -1 for off, must not be bigger than topk
        # rule options 
        "rule_b_max_branching_factor": "-1",
        "use_zero_rules": "true",
        "rule_zero_weight":"0.01",
        "use_u_c_rules": "true",
        "use_b_rules": "true",
        "use_u_d_rules": "true",
        "rule_u_d_weight":"0.1",
        "use_u_xxc_rules": "true",
        "use_u_xxd_rules": "true",
        "tie_handling": "frequency"
    }

    loader = c_clause.DataHandler(options)
    loader.load_datasets(path_test, path_train, path_valid)
    loader.load_rules(path_rules_input)

    ranker = c_clause.RankingHandler(options)
    ranker.calc_ranking(loader)
    ranker.write_ranking(path_preds_output, loader)
