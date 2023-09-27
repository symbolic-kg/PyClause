from clause.util.utils import get_ab_dir, get_base_dir
import c_clause

import os
from os import path
from subprocess import Popen, PIPE


def test_uc_b_zero_ranking():
    print(get_ab_dir())

    base_dir = get_base_dir()
    train_path = path.join(base_dir, "data/wnrr/train.txt")
    filter_path = path.join(base_dir, "data/wnrr/valid.txt")
    test_path = path.join(base_dir, "data/wnrr/test.txt")
    rules_path = path.join(base_dir, "data/wnrr/anyburl-rules-c5-3600")

    testing_dir = path.join(base_dir, "local/testing")

    if not path.isdir(testing_dir):
        os.mkdir(testing_dir)


    ranking_path = path.join(testing_dir, "test-ranking")


    conf_path = path.join(testing_dir, "testing-conf")

    #TODO this might work on Windows as path.join should handle it
    eval_config = [
        f"PATH_TRAINING = {train_path}",
        f"PATH_TEST = {test_path}",
        f"PATH_VALID = {filter_path}",
        f"PATH_PREDICTIONS = {ranking_path}"
        ]
    
    with open(conf_path, "w") as f:
        f.write("\n".join(eval_config))
    f.close()

    options = {
    # ranking options
    "aggregation_function": "maxplus",
    "num_preselect": "10000000",
    "topk": "100",
    "filter_w_train": "true",
    "filter_w_target": "true",
    "disc_at_least":"100",
    # rule options 
    "rule_zero_weight":"0.01",
    "rule_b_max_branching_factor": "-1",
    "use_zero_rules": "True",
    "use_u_c_rules": "True",
    "use_b_rules": "True",
    }

    ranker = c_clause.RankingHandler()
    ranker.calculateRanking(test_path, train_path, filter_path, rules_path, ranking_path, options)


    # TODO: this wouldnt work on windows probably
    p = Popen(
        [f"java -cp {get_ab_dir()} de.unima.ki.anyburl.Eval {conf_path}"],
        shell=True,
        stdout=PIPE,
        stderr=PIPE
        )
    stdout, stderr = p.communicate()

    mrr_result = str(stdout)[-9:-3]
    hAt10 = str(stdout)[-18:-12]
    hA1 = str(stdout)[-36:-30]

    expectMRR = "0.4961"
    expecth1 =  "0.4568"
    expecth10 = "0.5728"

    check_all = all([mrr_result==expectMRR, hA1==expecth1, hAt10==expecth10])
    if (not check_all):
        raise Exception(f"Expected test ranking results are {[expecth1, expecth10, expectMRR]} but i found {[hA1, hAt10, mrr_result]}")
        

    print(f"Test ranking of u_c, B and zero rules was successful expected: {[expecth1, expecth10, expectMRR]}")
    print(f"Calculated {[hA1, hAt10, mrr_result]}")






    

