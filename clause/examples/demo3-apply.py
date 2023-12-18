import sys
import os
sys.path.append(os.getcwd())


import c_clause

from clause.config.options import Options




if __name__ == "__main__":


    path_train = "data/wnrr/train.txt"
    path_valid = "data/wnrr/valid.txt"
    path_test  = "data/wnrr/test.txt"

    path_rules_input = "local/rules-wn18rr-anyburl.txt"

    path_preds_output = "local/preds-wn18rr-anyburl.txt"



    options = Options()
    # change on specific parameter of the default setting
    options.set("ranking_handler.disc_at_least", 50)

    loader = c_clause.Loader(options.flatS("loader"))
    loader.load_data(path_train, path_valid, path_test)
    loader.load_rules(path_rules_input)

    ranker = c_clause.RankingHandler(options.flatS("ranking_handler"))
    ranker.calculate_ranking(loader)
    ranker.write_ranking(path_preds_output, loader)
