
import c_clause

from clause.config.options import Options
from clause.util.utils import get_base_dir




if __name__ == "__main__":

    path_train = f"{get_base_dir()}/data/wnrr/train.txt"
    path_valid = f"{get_base_dir()}/data/wnrr/valid.txt"
    path_test = f"{get_base_dir()}/data/wnrr/test.txt"

    path_rules_input = f"{get_base_dir()}/local/rules-wn18rr-anyburl.txt"
    path_preds_output = f"{get_base_dir()}/local/preds-wn18rr-anyburl.txt"



    options = Options()
    # change on specific parameter of the default setting
    options.set("ranking_handler.disc_at_least", 50)

    loader = c_clause.Loader(options.get("loader"))
    loader.load_data(path_train, path_valid, path_test)
    loader.load_rules(path_rules_input)

    ranker = c_clause.RankingHandler(options.get("ranking_handler"))
    ranker.calculate_ranking(loader)
    ranker.write_ranking(path_preds_output, loader)
