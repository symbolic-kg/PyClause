from clause.util.utils import get_ab_dir, get_base_dir, join_u
import c_clause

import os
from os import path
from subprocess import Popen, PIPE


def test_uc_b_zero_ranking():
    print(get_ab_dir())

    base_dir = get_base_dir()
    train_path = join_u(base_dir, join_u("data", "wnrr", "train.txt"))
    filter_path = join_u(base_dir, join_u("data", "wnrr", "valid.txt"))

    test_path = join_u(base_dir, join_u("data", "wnrr", "test.txt"))
    rules_path = join_u(base_dir, join_u("data", "wnrr", "anyburl-rules-c5-3600"))

    testing_dir = join_u(base_dir, join_u("local", "testing"))

    if not path.isdir(testing_dir):
        os.mkdir(testing_dir)


    ranking_path = join_u(testing_dir, "test-ranking")


    conf_path = join_u(testing_dir, "testing-conf")

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
    "tie_handling": "frequency",
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
    "use_u_d_rules": "False",
    "use_b_rules": "True",
    "rule_num_unseen": "5",
    }

    loader = c_clause.DataHandler(options)
    loader.load_datasets(test_path, train_path, filter_path)
    loader.load_rules(rules_path)



    ranker = c_clause.RankingHandler(options)
    ranker.calc_ranking(loader)
    ranker.write_ranking(ranking_path, loader)

    p = Popen(
        f"java -cp {get_ab_dir()} de.unima.ki.anyburl.Eval {conf_path}",
        shell=True,
        stdout=PIPE,
        stderr=PIPE
        )
    stdout, stderr = p.communicate()

    mrr_result = str(stdout)[-9:-3]
    hAt10 = str(stdout)[-18:-12]
    hA1 = str(stdout)[-36:-30]

    expectMRR = "0.4965"
    expecth1 =  "0.4577"
    expecth10 = "0.5720"

    check_all = all([mrr_result==expectMRR, hA1==expecth1, hAt10==expecth10])
    if (not check_all):
        raise Exception(f"Expected test ranking results are {[expecth1, expecth10, expectMRR]} but i found {[hA1, hAt10, mrr_result]}")
        

    print(f"Test ranking of u_c, B and zero rules was successful expected: {[expecth1, expecth10, expectMRR]}")
    print(f"Calculated {[hA1, hAt10, mrr_result]}")



def test_237_all_ranking():
    print(get_ab_dir())

    base_dir = get_base_dir()
    train_path = join_u(base_dir, join_u("data", "fb15k-237", "train.txt"))
    filter_path = join_u(base_dir, join_u("data", "fb15k-237", "valid.txt"))

    test_path = join_u(base_dir, join_u("data", "fb15k-237", "test.txt"))
    rules_path = join_u(base_dir, join_u("data", "fb15k-237", "anyburl-rules-c3-3600"))

    testing_dir = join_u(base_dir, join_u("local", "testing"))

    if not path.isdir(testing_dir):
        os.mkdir(testing_dir)


    ranking_path = join_u(testing_dir, "test-ranking")


    conf_path = join_u(testing_dir, "testing-conf")

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
    "tie_handling": "frequency",
    "num_preselect": "10000000",
    "topk": "100",
    "filter_w_train": "true",
    "filter_w_target": "true",
    "disc_at_least":"10",
    # rule options 
    "rule_zero_weight":"0.01",
    "rule_b_max_branching_factor": "-1",
    "use_zero_rules": "True",
    "use_u_c_rules": "True",
    "use_u_d_rules": "True",
    "use_b_rules": "True",
    "rule_u_d_weight":"0.01",
    "use_u_xxc_rules": "True",
    "use_u_xxd_rules": "True",
    "rule_num_unseen":  "5",
    }

    loader = c_clause.DataHandler(options)
    loader.load_datasets(test_path, train_path, filter_path)
    loader.load_rules(rules_path)


    ranker = c_clause.RankingHandler(options)
    ranker.calc_ranking(loader)
    ranker.write_ranking(ranking_path, loader)


    p = Popen(
        f"java -cp {get_ab_dir()} de.unima.ki.anyburl.Eval {conf_path}",
        shell=True,
        stdout=PIPE,
        stderr=PIPE
        )
    stdout, stderr = p.communicate()

    mrr_result = str(stdout)[-9:-3]
    hAt10 = str(stdout)[-18:-12]
    hA1 = str(stdout)[-36:-30]

    expectMRR = "0.3318"
    expecth1 =  "0.2467"
    expecth10 = "0.5049"

    check_all = all([mrr_result==expectMRR, hA1==expecth1, hAt10==expecth10])
    if (not check_all):
        raise Exception(f"Expected test ranking results are {[expecth1, expecth10, expectMRR]} but i found {[hA1, hAt10, mrr_result]}")
        

    print(f"Test ranking on Fb15k-237 was successful expected: {[expecth1, expecth10, expectMRR]}")
    print(f"Calculated {[hA1, hAt10, mrr_result]}")


def test_qa_handler():
    import c_clause
    import numpy as np


    train = "./data/wnrr/train.txt"
    filter = "./data/wnrr/valid.txt"
    target = "./data/wnrr/test.txt"

    rules = "./data/wnrr/anyburl-rules-c5-3600"


    options = {
        # ranking options
        "aggregation_function": "maxplus",
        "num_preselect": "10000000",
        "topk": "100",
        "filter_w_train": "false",
        "filter_w_target": "true",
        "disc_at_least":"100", ## -1 for off, must not be bigger than topk
        # rule options 
        "rule_b_max_branching_factor": "-1",
        "use_zero_rules": "true",
        "rule_zero_weight":"0.01",
        "use_u_c_rules": "true",
        "use_b_rules": "true",
        "use_u_d_rules": "true",
        "rule_u_d_weight":"0.01",
        "use_u_xxc_rules": "true",
        "use_u_xxd_rules": "true",
        "tie_handling": "frequency",
    }


    loader = c_clause.DataHandler(options)
    loader.load_data(train, filter)
    loader.load_rules(rules)


    qa_handler = c_clause.QAHandler(options)

    ent_map = loader.entity_map()
    rel_map = loader.relation_map()

        

    # tail query example where we know that one true answer is in train 
    t_q_source = "00037919"
    t_q_rel = "_derivationally_related_form"
    t_answer_in_train = "02808440"


    t_answers_str = qa_handler.answer_queries([(t_q_source, t_q_rel)], loader, "tail")
    t_answers_idx = qa_handler.answer_queries([(ent_map[t_q_source], rel_map[t_q_rel])], loader, "tail")

    # == 1 both contain the true answer in train, note that by no means the rules must predict a triple in train
    # if you filter with train however they must not predict the answer
    assert(1==sum(np.array(t_answers_idx[0])[:,0] == ent_map[t_answer_in_train]))
    assert(1==sum(np.array(t_answers_str[0])[:,0] == t_answer_in_train))

    ## after not filtering with train the true answer from train must not be contained
    options["filter_w_train"] = "true"
    qa_handler.set_options(options)

    t_answers_str = qa_handler.answer_queries([(t_q_source, t_q_rel)], loader, "tail")
    t_answers_idx = qa_handler.answer_queries([(ent_map[t_q_source], rel_map[t_q_rel])], loader, "tail")

    assert(0==sum(np.array(t_answers_idx[0])[:,0] == ent_map[t_answer_in_train]))
    assert(0==sum(np.array(t_answers_str[0])[:,0] == t_answer_in_train))

    # outputs must be exactly identical
    for i, answer in enumerate(t_answers_str[0]):
        # answer[0]:str entity
        # answer[1]: float conf
        assert(t_answers_idx[0][i][0] == ent_map[answer[0]])
        assert(t_answers_idx[0][i][1] == answer[1])

        

    qa_handler.set_options({"filter_w_train": "false"})
    h_q_source = "01051956"
    h_q_rel = "_also_see"
    h_q_answer_in_train = "00941990"


    # the one true answer from train is in the answers
    assert(1==sum(np.array(qa_handler.answer_queries([[h_q_source, h_q_rel]], loader, "head")[0])[:,0]==h_q_answer_in_train))
    assert (1==sum(np.array(qa_handler.answer_queries([[ent_map[h_q_source], rel_map[h_q_rel]]], loader, "head")[0])[:,0]==ent_map[h_q_answer_in_train]))


    qa_handler.set_options({"filter_w_train": "true"})


    assert(0==sum(np.array(qa_handler.answer_queries([[h_q_source, h_q_rel]], loader, "head")[0])[:,0]==h_q_answer_in_train))
    assert (0==sum(np.array(qa_handler.answer_queries([[ent_map[h_q_source], rel_map[h_q_rel]]], loader, "head")[0])[:,0]==ent_map[h_q_answer_in_train]))


    # repeatedly calculating the answers would be bad in a usage scenario, here just for testing purpose
    for i, answer in enumerate(qa_handler.answer_queries([[h_q_source, h_q_rel]], loader, "head")[0]):
        # answer[0]:str entity
        # answer[1]: float conf
        assert(qa_handler.answer_queries([[ent_map[h_q_source], rel_map[h_q_rel]]], loader, "head")[0][i][0] == ent_map[answer[0]])
        assert(qa_handler.answer_queries([[ent_map[h_q_source], rel_map[h_q_rel]]], loader, "head")[0][i][1] == answer[1])

    print("Test for QA Handler idx and string version successful.")

    

