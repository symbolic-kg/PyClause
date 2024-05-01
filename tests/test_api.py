from clause.util.utils import get_ab_dir, get_base_dir, join_u
from clause import Options
import c_clause

import os
from os import path
from subprocess import Popen, PIPE


def test_adaptive_top_k():
    from c_clause import Loader, RankingHandler
    data = [
        ["aaa", "sp", "EE"],
        ["bbb", "sp", "EE"],
        ["ccc", "sp", "EE"],
        ["ddd", "sp", "EE"],
        ["eee", "sp", "EE"],

        ["aaa", "li", "lo"],
        ["bbb", "li", "lo"],
        ["ccc", "li", "we"],
        ["ddd", "li", "we"],
        ["eee", "li", "we"],
    ]

    rules = [
        "sp(X,EE) <= li(X,lo)",
        "sp(X,EE) <= li(X,we)"
    ]

    opts = Options()
    opts.set("ranking_handler.topk", 2)
    opts.set("ranking_handler.adapt_topk", False)
    opts.set("ranking_handler.filter_w_data", False)
    loader = Loader(options=opts.get("loader"))
    ranker = RankingHandler(options=opts.get("ranking_handler"))

    loader.load_data(data=data, filter=[], target=data)
    loader.load_rules(rules=rules, stats=[[5,5], [2,2]])
    ranker.calculate_ranking(loader=loader)
    head_ranking = ranker.get_ranking(direction="head", as_string=True)

    answers = dict(head_ranking["sp"]["EE"])
    assert("ccc" not in answers)
    assert("ddd" not in answers)
    assert("eee" not in answers)

    opts.set("ranking_handler.adapt_topk", True)
    ranker.set_options(opts.get("ranking_handler"))
    ranker.calculate_ranking(loader=loader)
    head_ranking = ranker.get_ranking(direction="head", as_string=True)
    answers = dict(head_ranking["sp"]["EE"])
    assert("ccc" in answers)
    assert("ddd" in answers)
    assert("eee" in answers)

    print("Test adaptive topk successful.")


def test_rules_handler():

    base_dir = get_base_dir()

    data = join_u(base_dir, join_u("data", "wnrr", "train.txt"))


    options = {
    "collect_statistics": "true",
    "collect_predictions": "true"
    }

    loader = c_clause.Loader(options)
    loader.load_data(data)

    entities = loader.get_entity_index()
    relations = loader.get_relation_index()

    rules_list = [
        "_derivationally_related_form(05755883,Y) <= ",
        "_hypernym(X,06355894) <= _synset_domain_topic_of(X,A), _synset_domain_topic_of(06355894,A)",
        "_member_meronym(12998349,Y) <= _hypernym(Y,11590783)",
        "_derivationally_related_form(X,01264336) <= _derivationally_related_form(A,X)",
        "_has_part(X,Y) <= _has_part(X,A), _member_of_domain_region(A,B), _member_of_domain_region(Y,B)", 
    ]

    handler = c_clause.RulesHandler(options)
    handler.calculate_predictions(rules_list, loader)

    str_preds = handler.get_predictions(True)
    idx_preds = handler.get_predictions(False)

    stats = handler.get_statistics()

    assert(len(str_preds[0])==len(idx_preds[0])==16109==stats[0][0])
    assert(len(str_preds[1])==len(idx_preds[1])==97==stats[1][0])
    assert(len(str_preds[2])==len(idx_preds[2])==59==stats[2][0])
    assert(len(str_preds[3])==len(idx_preds[3])==16106==stats[3][0])
    assert(len(str_preds[4])==len(idx_preds[4])==83==stats[4][0])

    ## Pick one rule and check that the idx map back to the correct tokens

    for i in range(len(idx_preds[4])):
        str_head = str_preds[4][i][0]
        str_rel = str_preds[4][i][1]
        str_tail = str_preds[4][i][2]
        assert(entities[str_head] == idx_preds[4][i][0])
        assert(relations[str_rel] == idx_preds[4][i][1])
        assert(entities[str_tail] == idx_preds[4][i][2])
    
    if not os.path.exists(join_u("local", "testing")):
        os.makedirs(join_u("local", "testing"))
        
    ## Test loading rules from file (w and wo stats)
    test_rules_path = join_u(base_dir, join_u("local", "testing", "test_rules.txt"))
    with open(test_rules_path, "w") as outfile:
        outfile.write("\n".join(rules_list))
    handler = c_clause.RulesHandler(options)
    handler.calculate_predictions(test_rules_path, loader)
    str_preds = handler.get_predictions(True)
    idx_preds = handler.get_predictions(False)
    stats = handler.get_statistics()
    assert(len(str_preds[0])==len(idx_preds[0])==16109==stats[0][0])
    assert(len(str_preds[1])==len(idx_preds[1])==97==stats[1][0])
    assert(len(str_preds[2])==len(idx_preds[2])==59==stats[2][0])
    assert(len(str_preds[3])==len(idx_preds[3])==16106==stats[3][0])
    assert(len(str_preds[4])==len(idx_preds[4])==83==stats[4][0])
        
    test_rules_w_stats_path = join_u(base_dir, join_u("local", "testing", "test_rules_w_stats.txt"))
    rules_w_stats = [f"{stat[0]}\t{stat[1]}\t{stat[1] / stat[0]}\t{rule}" for stat, rule in zip(stats, rules_list)]
    with open(test_rules_w_stats_path, "w") as outfile:
        outfile.write("\n".join(rules_w_stats))
    handler = c_clause.RulesHandler(options)
    handler.calculate_predictions(test_rules_w_stats_path, loader)
    str_preds = handler.get_predictions(True)
    idx_preds = handler.get_predictions(False)
    stats = handler.get_statistics()
    assert(len(str_preds[0])==len(idx_preds[0])==16109==stats[0][0])
    assert(len(str_preds[1])==len(idx_preds[1])==97==stats[1][0])
    assert(len(str_preds[2])==len(idx_preds[2])==59==stats[2][0])
    assert(len(str_preds[3])==len(idx_preds[3])==16106==stats[3][0])
    assert(len(str_preds[4])==len(idx_preds[4])==83==stats[4][0])
    
    
    test_rules_stats_output = join_u(base_dir, join_u("local", "testing", "test_rules_stats_output.txt"))
    handler.write_statistics(test_rules_stats_output)
    with open(test_rules_stats_output, "r") as infile:
        rules = [x.strip().split("\t") for x in infile.readlines()]
    assert len(rules) == 5
    for rule in rules:
        assert len(rule) == 4
    
    test_rules_preds_output = join_u(base_dir, join_u("local", "testing",  "test_rules_preds_output.txt"))
    # write int flat
    handler.write_predictions(test_rules_preds_output, flat=True, as_string=False)
    with open(test_rules_preds_output, "r") as infile:
        predictions = [x.strip().split("\t") for x in infile.readlines()]
    assert len(predictions) == 32453
    for prediction in predictions:
        assert len(prediction) == 3
        
    # write string flat
    handler.write_predictions(test_rules_preds_output, flat=True, as_string=True)
    with open(test_rules_preds_output, "r") as infile:
        predictions = [x.strip().split("\t") for x in infile.readlines()]
    assert len(predictions) == 32453
    for prediction in predictions:
        assert len(prediction) == 3
        

        
    # write int json
    test_rules_preds_output = join_u(base_dir, join_u("local", "testing", "test_rules_preds_output.json"))
    handler.write_predictions(test_rules_preds_output, flat=False, as_string=False)
    with open(test_rules_preds_output, "r") as infile:
        # list of dicts each dict corresponds to one rule
        predictions = []
        import json
        # check if valid jsonl is test enough for now
        for line in infile:
            prediction = json.loads(line)
            predictions.append(prediction)
    assert len(predictions) == 5
        
    # write string json
    handler.write_predictions(test_rules_preds_output, flat=False, as_string=True)
    with open(test_rules_preds_output, "r") as infile:
        predictions = []
        import json
        for line in infile:
            prediction = json.loads(line)
            predictions.append(prediction)
    assert len(predictions) == 5

    print("Testing RulesHandler successful")


def test_rule_loading():
    from clause.rule.ruleparser import RuleReader
    from clause.rule.rules import RuleSet
    from clause.data.triples import TripleSet
    print(get_ab_dir())

    base_dir = get_base_dir()
    train_path = join_u(base_dir, join_u("data", "wnrr", "train.txt"))
    rules_path = join_u(base_dir, join_u("data", "wnrr", "anyburl-rules-c5-3600"))

    # load an anyburl ruleset with frontend
    triples = TripleSet(train_path)
    rules = RuleSet(triples.index)
    rule_reader = RuleReader(rules)
    rule_reader.read_file(rules_path)

    options = {}

    loader = c_clause.Loader(options)
    loader.load_data(train_path)
    ## serialize rules from frontend
    loader.load_rules([rule.get_serialization() for rule in rules.rules])


    loader2 = c_clause.Loader(options)
    loader2.load_data(train_path)
    ## load rules from disk
    loader2.load_rules(rules_path)

    index1 = loader.rule_index()
    index2 = loader2.rule_index()

    assert(index1==index2)
        
    print("Test for rule loading successful.")

def test_multi_thr_rule_loading():
    from clause import Options
    from c_clause import Loader

    base_dir = get_base_dir()
    train_path = join_u(base_dir, join_u("data", "fb15k-237", "train.txt"))
    rules_path = join_u(base_dir, join_u("data", "fb15k-237", "anyburl-rules-c3-3600"))

    opts = Options()
    opts.set("loader.num_threads", 1)
     # discard one rule type to check if indexing is correct
    opts.set("loader.load_b_rules", False)
    loader = Loader(options=opts.get("loader"))

    loader.load_data(train_path)
    loader.load_rules(rules=rules_path)

    rules_idx_single = loader.rule_index()
    rules_single = loader.get_rules()

    opts.set("loader.num_threads", -1)
   
    
    loader.set_options(options=opts.get("loader"))
    loader.load_rules(rules=rules_path)

    rules_idx_multi = loader.rule_index()
    rules_multi = loader.get_rules()

    assert(rules_idx_single==rules_idx_multi)
    assert(rules_single==rules_multi)

    print("Test loading multithreaded successful.")


def test_update_rules():
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

    options = Options()


    options.set("ranking_handler.disc_at_least", 100)

    loader = c_clause.Loader(options.get("loader"))
    # only before data is loaded
    loader.set_entity_index(["mustnotbreakanything"])
    loader.set_relation_index(["mustnotbreaktoo"])
    loader.load_data(train_path, filter_path, test_path)

    # load rules twice to test interference
    options.set("loader.load_b_rules", False)
    loader.set_options(options.get("loader"))
    loader.load_rules(rules_path)

    options.set("loader.load_b_rules", True)
    loader.set_options(options.get("loader"))
    loader.load_rules(rules_path)

    # all rules have been loaded, now subset
    options.set("loader.load_u_d_rules", False)
    options.set("loader.load_u_d_rules", False)
    options.set("loader.load_u_xxc_rules", False)
    options.set("loader.load_u_xxd_rules", False)

    loader.set_options(options.get("loader"))
    loader.update_rules()

    # write and load subset
    loader.write_rules(testing_dir + "/written_rules")
    loader.load_rules(testing_dir + "/written_rules")

    ranker = c_clause.RankingHandler(options.get("ranking_handler"))
    ranker.calculate_ranking(loader)
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

    expectMRR = "0.4958"
    expecth1 =  "0.4569"
    expecth10 = "0.5718"

    check_all = all([mrr_result==expectMRR, hA1==expecth1, hAt10==expecth10])
    if (not check_all):
        raise Exception(f"Update rules test: Expected test ranking results are {[expecth1, expecth10, expectMRR]} but i found {[hA1, hAt10, mrr_result]}")
        
    print(f"Test of update rules was successful expected: {[expecth1, expecth10, expectMRR]}")
    print(f"Calculated {[hA1, hAt10, mrr_result]}")

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

    options = Options()

    options.set("loader.load_u_d_rules", False)
    options.set("loader.load_u_d_rules", False)
    options.set("loader.load_u_xxc_rules", False)
    options.set("loader.load_u_xxd_rules", False)

    options.set("ranking_handler.disc_at_least", 100)

    loader = c_clause.Loader(options.get("loader"))
    # only before data is loaded
    loader.set_entity_index(["mustnotbreakanything"])
    loader.set_relation_index(["mustnotbreaktoo"])
    loader.load_data(train_path, filter_path, test_path)

    # load rules twice to test interference
    options.set("loader.load_b_rules", False)
    loader.set_options(options.get("loader"))
    loader.load_rules(rules_path)

    options.set("loader.load_b_rules", True)
    loader.set_options(options.get("loader"))
    loader.load_rules(rules_path)

    # write and load subset
    loader.write_rules(testing_dir + "/written_rules")
    loader.load_rules(testing_dir + "/written_rules")

    ranker = c_clause.RankingHandler(options.get("ranking_handler"))
    ranker.calculate_ranking(loader)
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

    expectMRR = "0.4958"
    expecth1 =  "0.4569"
    expecth10 = "0.5718"

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

    options = Options()

    loader = c_clause.Loader(options.get("loader"))
    loader.set_entity_index(["mustnotbreakanything"])
    loader.set_relation_index(["mustnotbreaktoo"])
    loader.load_data(train_path, filter_path, test_path)
    loader.load_rules(rules_path)

    ranker = c_clause.RankingHandler(options.get("ranking_handler"))
    ranker.calculate_ranking(loader)
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

    expectMRR = "0.3322"
    expecth1 =  "0.2465"
    expecth10 = "0.5062"

    check_all = all([mrr_result==expectMRR, hA1==expecth1, hAt10==expecth10])
    if (not check_all):
        raise Exception(f"Expected test ranking results are {[expecth1, expecth10, expectMRR]} but i found {[hA1, hAt10, mrr_result]}")
        
    print(f"Test ranking on Fb15k-237 was successful expected: {[expecth1, expecth10, expectMRR]}")
    print(f"Calculated {[hA1, hAt10, mrr_result]}")


def test_qa_handler():
    import c_clause
    import numpy as np

    base_dir = get_base_dir()

    train = join_u(base_dir, join_u("data", "wnrr", "train.txt"))
    filter = join_u(base_dir, join_u("data", "wnrr", "valid.txt"))

    target = join_u(base_dir, join_u("data", "wnrr", "test.txt"))
    rules = join_u(base_dir, join_u("data", "wnrr", "anyburl-rules-c5-3600"))

    options = Options()

    options.set("qa_handler.filter_w_data", False)
    loader = c_clause.Loader(options.get("loader"))
    loader.load_data(train, filter)
    loader.load_rules(rules)

    qa_handler = c_clause.QAHandler(options.get("qa_handler"))

    ent_map = loader.get_entity_index()
    rel_map = loader.get_relation_index()

    # tail query example where we know that one true answer is in train 
    t_q_source = "00037919"
    t_q_rel = "_derivationally_related_form"
    t_answer_in_train = "02808440"


    qa_handler.calculate_answers([(t_q_source, t_q_rel)], loader, "tail")
    t_answers_str = qa_handler.get_answers(True)

    qa_handler.calculate_answers([(ent_map[t_q_source], rel_map[t_q_rel])], loader, "tail")
    t_answers_idx = qa_handler.get_answers(False)

    # == 1 both contain the true answer in train, note that by no means the rules must predict a triple in train
    # if you filter with train however they must not predict the answer
    assert(1==sum(np.array(t_answers_idx[0])[:,0] == ent_map[t_answer_in_train]))
    assert(1==sum(np.array(t_answers_str[0])[:,0] == t_answer_in_train))

    ## after not filtering with train the true answer from train must not be contained
    options.set("qa_handler.filter_w_data", True)
    qa_handler = c_clause.QAHandler(options.get("qa_handler"))

    qa_handler.calculate_answers([(t_q_source, t_q_rel)], loader, "tail")
    t_answers_str = qa_handler.get_answers(True)

    qa_handler.calculate_answers([(ent_map[t_q_source], rel_map[t_q_rel])], loader, "tail")
    t_answers_idx = qa_handler.get_answers(False)

    assert(0==sum(np.array(t_answers_idx[0])[:,0] == ent_map[t_answer_in_train]))
    assert(0==sum(np.array(t_answers_str[0])[:,0] == t_answer_in_train))

    # outputs must be exactly identical
    for i, answer in enumerate(t_answers_str[0]):
        # answer[0]:str entity
        # answer[1]: float conf
        assert(t_answers_idx[0][i][0] == ent_map[answer[0]])
        assert(t_answers_idx[0][i][1] == answer[1])

        
    options.set("qa_handler.filter_w_data", False)    
    qa_handler = c_clause.QAHandler(options.get("qa_handler"))
    h_q_source = "01051956"
    h_q_rel = "_also_see"
    h_q_answer_in_train = "00941990"

    # the one true answer from train is in the answers
    qa_handler.calculate_answers([[h_q_source, h_q_rel]], loader, "head")
    answers_str = qa_handler.get_answers(True)
    answers_idx = qa_handler.get_answers(False)

    assert(1==sum(np.array(answers_str[0])[:,0]==h_q_answer_in_train))
    assert (1==sum(np.array(answers_idx[0])[:,0]==ent_map[h_q_answer_in_train]))

    options.set("qa_handler.filter_w_data", True)    
    qa_handler = c_clause.QAHandler(options.get("qa_handler"))

    qa_handler.calculate_answers([[ent_map[h_q_source], rel_map[h_q_rel]]], loader, "head")
    answers_str = qa_handler.get_answers(True)
    answers_idx = qa_handler.get_answers(False)


    assert(0==sum(np.array(answers_str[0])[:,0]==h_q_answer_in_train))
    assert (0==sum(np.array(answers_idx[0])[:,0]==ent_map[h_q_answer_in_train]))

    for i in range(len(answers_str)):
        for j in range(len(answers_str[i])):
            answers_str[i][j][1] == answers_idx[i][j][1]

    print("Test for QA Handler idx and string version successful.")


def test_triple_scoring_B_237():
    import c_clause
    import numpy as np

    base_dir = get_base_dir()
    train = join_u(base_dir, join_u("data", "fb15k-237", "train.txt"))
    filter = join_u(base_dir, join_u("data", "fb15k-237", "valid.txt"))

    target = join_u(base_dir, join_u("data", "fb15k-237", "test.txt"))
    rules = join_u(base_dir, join_u("data", "fb15k-237", "anyburl-rules-c3-3600"))

    options = Options()

    options.set("loader.load_zero_rules", False)
    options.set("loader.load_u_c_rules", False)
    options.set("loader.load_u_d_rules", False)
    options.set("loader.load_u_xxc_rules", False)
    options.set("loader.load_u_xxd_rules", False)
   
    options.set("ranking_handler.disc_at_least", -1)
    options.set("ranking_handler.topk", 1)
    options.set("ranking_handler.hard_stop_at", 1)

    options.set("prediction_handler.collect_explanations", True)
    options.set("prediction_handler.num_top_rules", 1)

    loader = c_clause.Loader(options.get("loader"))
    loader.load_data(train, filter, target)
    loader.load_rules(rules)

    scorer = c_clause.PredictionHandler(options.get("prediction_handler"))
    print(target)
    scorer.calculate_scores(target, loader)

    idx_scores = scorer.get_scores(False)
    str_scores = scorer.get_scores(True)


    ranker = c_clause.RankingHandler(options.get("ranking_handler"))
    ranker.calculate_ranking(loader)

    tails = ranker.get_ranking("tail", False)
    heads = ranker.get_ranking("head", False)

    for i in range(len(idx_scores)):
        # same scores; string scores are rounded to 6 decimals in backend
        assert(round(idx_scores[i][3], 6)==float(str_scores[i][3]))
        head = idx_scores[i][0]
        rel = idx_scores[i][1]
        tail = idx_scores[i][2]
    
    ## extract all triples from ranker and score
    triples = []
    for rel, head_dict in tails.items():
        for head, tail_list in head_dict.items():
            for tail_tup in tail_list:
                triple = [head, rel, tail_tup[0]]
                triples.append(triple)

    scorer.calculate_scores(triples, loader)
    idx_scores = scorer.get_scores(False)
    explanations = scorer.get_explanations(True)

    for i in range(len(idx_scores)):
        head = idx_scores[i][0]
        rel = idx_scores[i][1]
        tail = idx_scores[i][2]

        idx_scores[i][3]
        tails_dict = dict(tails[rel][head])
        assert(idx_scores[i][3]==tails_dict[tail])
    print("Test Triple scoring for B 237 successful.")      

    
def test_loader():
    import c_clause
    
    options = Options()

    num_ent = 10
    num_rel = 5

    entity_index = [str(i) for i in range(num_ent)]
    relation_index = [str(j) for j in range(num_rel)]
    
    loader = c_clause.Loader(options.get("loader"))
    loader.set_entity_index(entity_index)
    loader.set_relation_index(relation_index)

    triples = [[1, 0, num_ent-1], [1, num_rel-1, 3], [1,0,num_ent-2], [1, num_rel-2, 3]]
    loader.load_data(triples)
    
    assert len(loader.get_entity_index()) == num_ent
    assert len(loader.get_relation_index()) == num_rel
    for key, val in loader.get_entity_index().items():
        assert int(key) == val
        assert val < num_ent
        
    assert len(loader.get_relation_index()) == num_rel
    for key, val in loader.get_relation_index().items():
        assert int(key) == val
        assert val < num_rel
    
    loader = c_clause.Loader(options.get("loader"))
    triples = [["a", "r1", "b"], ["a", "r1", "c"], ["c", "r2", "d"]]
    loader.load_data(triples)
    assert len(loader.get_entity_index()) == 4
    assert len(loader.get_relation_index()) == 2
    
    # test behaviour of unseen entities (e, f in test, g in valid)
    train = [["a", "r1", "b"], ["a", "r1", "c"], ["c", "r2", "d"]]
    test = [["c", "r2", "b"], ["d", "r1", "a"], ["e", "r1", "f"]]
    valid = [["g", "r1", "a"]]
    loader = c_clause.Loader(options.get("loader"))
    loader.load_data(data=train, target=test)
    assert len(loader.get_entity_index()) == 6
    assert len(loader.get_relation_index()) == 2
    loader = c_clause.Loader(options.get("loader"))
    loader.load_data(data=train, filter=valid, target=test)
    assert len(loader.get_entity_index()) == 7
    assert len(loader.get_relation_index()) == 2
    
    print("Test for loading idx's successful")


def test_triple_scoring():
    """
    Test if the triple scores with string output match the idx version.
    Also test if the candidate scores calculated from query ranking match the triple scores.
    
    """
    import c_clause
    import numpy as np

    base_dir = get_base_dir()
    train = join_u(base_dir, join_u("data", "wnrr", "train.txt"))
    filter = join_u(base_dir, join_u("data", "wnrr", "valid.txt"))
    rules = join_u(base_dir, join_u("data", "wnrr", "anyburl-rules-c5-3600"))
    target = join_u(base_dir, join_u("data", "wnrr-sample", "test-wnrr-small.txt"))

    options = Options()
    options.set("ranking_handler.topk", 40000)

    # both set to -1 to not apply any stopping critertion (otherwise scores might vary a bit)
    options.set("ranking_handler.disc_at_least", -1)

    options.set("prediction_handler.num_top_rules", -1)
    options.set("prediction_handler.collect_explanations", True)
    
    options.set("loader.load_u_xxd_rules", False)
    options.set("loader.load_u_xxc_rules", False)
    options.set("loader.load_zero_rules", False)

    loader = c_clause.Loader(options.get("loader"))
    loader.load_data(train, filter, target)
    loader.load_rules(rules)

    scorer = c_clause.PredictionHandler(options.get("prediction_handler"))
    scorer.calculate_scores(target, loader)

    idx_scores = scorer.get_scores(False)
    str_scores = scorer.get_scores(True)

    ranker = c_clause.RankingHandler(options.get("ranking_handler"))
    ranker.calculate_ranking(loader)

    tails = ranker.get_ranking("tail", False)
    heads = ranker.get_ranking("head", False)

    for i in range(len(idx_scores)):
        # same scores; string scores are rounded to 6 decimals in backend
        assert(round(idx_scores[i][3], 6)==float(str_scores[i][3]))
        head = idx_scores[i][0]
        rel = idx_scores[i][1]
        tail = idx_scores[i][2]

        
        # the score must match when calculated in a ranking from queries
        if (idx_scores[i][3]!=0):
            tail_ranking = tails[rel][head]
            tail_ranking = dict(tail_ranking)
            assert(idx_scores[i][3]==tail_ranking[tail])
            
            head_ranking = heads[rel][tail]
            head_ranking = dict(head_ranking)
            assert(idx_scores[i][3]==head_ranking[head])


    # now without the track grounding option
    options.set("prediction_handler.collect_explanations", False)
    scorer = c_clause.PredictionHandler(options.get("prediction_handler"))
    scorer.calculate_scores(target, loader)

    idx_scores = scorer.get_scores(False)
    str_scores = scorer.get_scores(True)


    for i in range(len(idx_scores)):
        # same scores; string scores are rounded to 6 decimals in backend
        assert(round(idx_scores[i][3], 6)==float(str_scores[i][3]))
        head = idx_scores[i][0]
        rel = idx_scores[i][1]
        tail = idx_scores[i][2]

        
        # the score must match when calculated in a ranking from queries
        if (idx_scores[i][3]!=0):
            tail_ranking = tails[rel][head]
            tail_ranking = dict(tail_ranking)
            assert(idx_scores[i][3]==tail_ranking[tail])
            
            head_ranking = heads[rel][tail]
            head_ranking = dict(head_ranking)
            assert(idx_scores[i][3]==head_ranking[head])

    print("Test Triple scoring successful.")


def test_noisy_triple_scoring():
    """
    Test if the triple scores with string output match the idx version.
    Also test if the candidate scores calculated under noisy or from query ranking match the triple scores.
    
    """


    import c_clause
    import numpy as np

    base_dir = get_base_dir()
    train = join_u(base_dir, join_u("data", "wnrr", "train.txt"))
    filter = join_u(base_dir, join_u("data", "wnrr", "valid.txt"))
    rules = join_u(base_dir, join_u("data", "wnrr", "anyburl-rules-c5-3600"))
    target = join_u(base_dir, join_u("data", "wnrr-sample", "test-wnrr-small.txt"))



    options = Options()
    options.set("ranking_handler.topk", 40000)

    # both set to -1 to not apply any stopping critertion (otherwise scores might vary a bit)
    options.set("ranking_handler.disc_at_least", -1)
    options.set("prediction_handler.num_top_rules", -1)
    options.set("ranking_handler.aggregation_function", "noisyor")
    options.set("prediction_handler.aggregation_function", "noisyor")
    options.set("prediction_handler.collect_explanations", True)
    
    options.set("loader.load_u_xxd_rules", False)
    options.set("loader.load_u_xxc_rules", False)
    options.set("loader.load_zero_rules", False)

    loader = c_clause.Loader(options.get("loader"))
    loader.load_data(train, filter, target)
    loader.load_rules(rules)

    scorer = c_clause.PredictionHandler(options.get("prediction_handler"))
    scorer.calculate_scores(target, loader)

    idx_scores = scorer.get_scores(False)
    str_scores = scorer.get_scores(True)

    ranker = c_clause.RankingHandler(options.get("ranking_handler"))
    ranker.calculate_ranking(loader)

    tails = ranker.get_ranking("tail", False)
    heads = ranker.get_ranking("head", False)

    for i in range(len(idx_scores)):
        # same scores; string scores are rounded to 6 decimals in backend
        assert(round(idx_scores[i][3], 6)==float(str_scores[i][3]))
        head = idx_scores[i][0]
        rel = idx_scores[i][1]
        tail = idx_scores[i][2]

        
        # the score must match when calculated in a ranking from queries
        if (idx_scores[i][3]!=0):
            tail_ranking = tails[rel][head]
            tail_ranking = dict(tail_ranking)
            assert(idx_scores[i][3]==tail_ranking[tail])
            
            head_ranking = heads[rel][tail]
            head_ranking = dict(head_ranking)
            assert(idx_scores[i][3]==head_ranking[head])


    # now without the track grounding option
    options.set("prediction_handler.collect_explanations", False)
    scorer = c_clause.PredictionHandler(options.get("prediction_handler"))
    scorer.calculate_scores(target, loader)

    idx_scores = scorer.get_scores(False)
    str_scores = scorer.get_scores(True)

    for i in range(len(idx_scores)):
        # same scores; string scores are rounded to 6 decimals in backend
        assert(round(idx_scores[i][3], 6)==float(str_scores[i][3]))
        head = idx_scores[i][0]
        rel = idx_scores[i][1]
        tail = idx_scores[i][2]

        # the score must match when calculated in a ranking from queries
        if (idx_scores[i][3]!=0):
            tail_ranking = tails[rel][head]
            tail_ranking = dict(tail_ranking)
            assert(idx_scores[i][3]==tail_ranking[tail])
            
            head_ranking = heads[rel][tail]
            head_ranking = dict(head_ranking)
            assert(idx_scores[i][3]==head_ranking[head])

    print("Test Triple scoring successful.")


def test_noisy_or():
    """
    Test if the triple scores with string output match the idx version.
    Also test if the candidate scores calculated under noisy or from query ranking match the triple scores.
    
    """


    import c_clause
    import numpy as np

    base_dir = get_base_dir()
    train = join_u(base_dir, join_u("data", "wnrr", "train.txt"))
    rules = join_u(base_dir, join_u("data", "wnrr", "anyburl-rules-c5-3600"))

    options = Options()
    options.set("loader.load_u_xxd_rules", False)
    options.set("loader.load_u_xxc_rules", False)
    options.set("loader.load_zero_rules", False)

    loader = c_clause.Loader(options.get("loader"))
    loader.load_data(train)
    loader.load_rules(rules)


    
    options.set("qa_handler.topk", 300)
    options.set("qa_handler.disc_at_least", -1)
    options.set("qa_handler.aggregation_function", "noisyor")
    options.set("qa_handler.num_top_rules", 5)
    qa_5 = c_clause.QAHandler(options.get("qa_handler"))

    options.set("qa_handler.num_top_rules", -1)
    qa_ALL = c_clause.QAHandler(options.get("qa_handler"))

    triples = [
        #("04868748", "_hypernym", "04826235"),
        #("01296462", "_derivationally_related_form", "00379422"),
        ("00538571", "_synset_domain_topic_of", "06084469"),
        #("01466543", "_derivationally_related_form", "06084469"),
        #("04868748", "_hypernym", "04826235"),
    ]
    
    head_queries = [ (tr[2], tr[1]) for tr in triples ]
    tail_queries = [ (tr[0], tr[1]) for tr in triples ]


    options.set("prediction_handler.aggregation_function", "noisyor")
    options.set("prediction_handler.num_top_rules" , -1)
    scorer_all = c_clause.PredictionHandler(options.get("prediction_handler"))

    options.set("prediction_handler.aggregation_function", "noisyor")
    options.set("prediction_handler.num_top_rules" , 5)
    scorer_5 = c_clause.PredictionHandler(options.get("prediction_handler"))


    scorer_all.calculate_scores(triples, loader)
    scores_all = scorer_all.get_scores(False)

    scorer_5.calculate_scores(triples, loader)
    scores_5 = scorer_5.get_scores(False)



    qa_5.calculate_answers(head_queries, loader, "head")
    head_answers_5 = qa_5.get_answers(False)
    qa_5.calculate_answers(tail_queries, loader, "tail")
    tail_answers_5 = qa_5.get_answers(False)

    qa_ALL.calculate_answers(head_queries, loader, "head")
    head_answers_ALL = qa_ALL.get_answers(False)
    qa_ALL.calculate_answers(tail_queries, loader, "tail")
    tail_answers_ALL = qa_ALL.get_answers(False)

    get_entity_index = loader.get_entity_index()


    for i in range(len(scores_all)):

        score_ALL = scores_all[i][3]
        score_5 = scores_5[i][3]
        assert(score_ALL>=score_5)

        head_cand_ALL = dict(head_answers_ALL[i])
        head_cand_5 = dict(head_answers_5[i])

        tail_cand_ALL = dict(tail_answers_ALL[i])
        tail_cand_5 = dict(tail_answers_5[i])

        # every noisy-top5 cand must be a noisy-all cand
        # and the score must not be bigger
        for key in head_cand_5.keys():
            assert(key in head_cand_ALL)
            assert(head_cand_5[key]<=head_cand_ALL[key])
        for key in tail_cand_5.keys():
            assert(tail_cand_5[key]<=tail_cand_ALL[key])
            assert(key in tail_cand_ALL)

        # scores from triple scoring must match score of true cand from qa
        if score_5>0:
            assert(score_5 == tail_cand_5[get_entity_index[triples[i][2]]])
            assert(score_5 == head_cand_5[get_entity_index[triples[i][0]]])
        if score_ALL>0:
            assert(score_ALL == tail_cand_ALL[get_entity_index[triples[i][2]]])
            assert(score_ALL == head_cand_ALL[get_entity_index[triples[i][0]]])
        else:
            assert(score_ALL==score_5)
    print("Test noisy-or successful.")


def test_explanation_tracking():
    """Testing if explanations are consistent. This also tests the rule_index."""


    base_dir = get_base_dir()
    train = join_u(base_dir, join_u("data", "wnrr", "train.txt"))
    filter = join_u(base_dir, join_u("data", "wnrr", "valid.txt"))

    target = join_u(base_dir, join_u("data", "wnrr", "test.txt"))
    rules = join_u(base_dir, join_u("data", "wnrr", "anyburl-rules-c5-3600"))

    options = Options()
    
    options.set("prediction_handler.collect_explanations", True)


    options.set("loader.load_u_xxd_rules", False)
    options.set("loader.load_u_xxc_rules", False)
    options.set("loader.load_zero_rules", False)

    num_top_rules = 10


    options.set("prediction_handler.num_top_rules", num_top_rules)

    loader = c_clause.Loader(options.get("loader"))
    loader.load_data(train, filter, target)
    loader.load_rules(rules)


    scorer = c_clause.PredictionHandler(options.get("prediction_handler"))
    scorer.calculate_scores("./data/wnrr/test.txt", loader)

    idx_scores = scorer.get_scores(False)
    str_scores = scorer.get_scores(True)

    idx_explanations = scorer.get_explanations(False)
    str_explanations = scorer.get_explanations(True)

    rule_idx = loader.rule_index()

    get_entity_index = loader.get_entity_index()
    get_relation_index = loader.get_relation_index()



    assert(
        len(idx_explanations[0])==len(idx_explanations[1])==len(idx_explanations[2])\
        ==len(str_explanations[0])==len(str_explanations[1])==len(str_explanations[2])
    )

    for i in range(len(idx_explanations[0])):
        str_rules = str_explanations[1][i]
        idx_rules = idx_explanations[1][i]

        assert(len(idx_rules)<=num_top_rules and len(idx_rules)==len(str_rules))

        ## check that the rules from idx map back to the outputted string rules
        assert([rule_idx[j] for j in idx_rules] == str_rules)

        #print(f"Target triple: {explanations[0][i]}")

        # all rule groundings
        for j in range(len(str_rules)):
            # list of groundings; a grounding itself is a list with triples
            for l in range(len(str_explanations[2][i][j])):
                ## str_explanations[2][i][j][l]) is ONE GROUNDING, that is, a list of triples where a triple is again a list/tuple
                ## for every triple of the grounding
                for k in range(len(str_explanations[2][i][j][l])):
                    triple = str_explanations[2][i][j][l][k]

                    ## check that every entity/relation string of every grounding triple maps back to the idx representation
                    assert(get_entity_index[triple[0]] == idx_explanations[2][i][j][l][k][0])
                    assert(get_relation_index[triple[1]] == idx_explanations[2][i][j][l][k][1])
                    assert(get_entity_index[triple[2]] == idx_explanations[2][i][j][l][k][2])
    print("Test for explanation consistency successful")


def test_rules_collecting():
    from c_clause import RankingHandler, Loader
    from clause.util.utils import get_ab_dir, get_base_dir, join_u
    from clause.config.options import Options
    from c_clause import QAHandler, RankingHandler, PredictionHandler
    from clause.data.triples import TripleSet
    base_dir = get_base_dir()
    train = join_u(base_dir, join_u("data", "wnrr", "train.txt"))
    filter = join_u(base_dir, join_u("data", "wnrr", "valid.txt"))
    rules = join_u(base_dir, join_u("data", "wnrr", "anyburl-rules-c5-3600"))
    target = join_u(base_dir, join_u("data", "wnrr-sample", "test-wnrr-small.txt"))

    ranking_file = "./local/rankingFile.txt"

    options = Options()
    options.set("ranking_handler.disc_at_least", 50)
    options.set("ranking_handler.collect_rules", True)

    options.set("qa_handler.disc_at_least", 50)
    options.set("qa_handler.collect_rules", True)

    loader = Loader(options.get("loader"))
    loader.load_data(train, filter, target)
    loader.load_rules(rules)


    ranker = RankingHandler(options.get("ranking_handler"))
    ranker.calculate_ranking(loader)


    rule_index = loader.rule_index()
    get_entity_index = loader.get_entity_index()
    get_relation_index = loader.get_relation_index()


    headRanking_idx = ranker.get_ranking("head", False)
    tailRanking_idx = ranker.get_ranking("tail", False)

    headRanking_str = ranker.get_ranking("head", True)
    tailRanking_str = ranker.get_ranking("tail", True)



    head_rules_ranker_str = ranker.get_rules("head", True)
    tail_rules_ranker_str = ranker.get_rules("tail", True)

    head_rules_ranker_idx = ranker.get_rules("head", False)
    tail_rules_ranker_idx = ranker.get_rules("tail", False)


    ## load the data from this, form queries from every triple and pass them to the qa handler
    ## collected rules must be the same when retrived from ranking of the ranking handler
    triples = TripleSet(target)
    triple_strings = triples.to_list()

    ## string queries
    tail_queries = [[trip[0], trip[1]] for trip in triple_strings]
    head_queries = [[trip[2], trip[1]] for trip in triple_strings]

    qa = QAHandler(options.get("qa_handler"))

    qa.calculate_answers(head_queries, loader, "head")
    head_answers_str = qa.get_answers(True)
    head_answers_idx = qa.get_answers(False)

    head_rules_str = qa.get_rules(True)
    head_rules_idx = qa.get_rules(False)


    qa.calculate_answers(tail_queries, loader, "tail")
    tail_answers_str = qa.get_answers(True)
    tail_answers_idx = qa.get_answers(False)

    tail_rules_str = qa.get_rules(True)
    tail_rules_idx = qa.get_rules(False)


    for i in range(len(tail_queries)):
        source_str = tail_queries[i][0]
        rel_str = tail_queries[i][1]

        source_idx = get_entity_index[source_str]
        rel_idx = get_relation_index[rel_str]

        for c in range(len(tail_answers_idx[i])):
            cand_idx = tail_answers_idx[i][c][0]
            cand_str = tail_answers_str[i][c][0]

            idx_rules = tail_rules_idx[i][c]
            str_rules = tail_rules_str[i][c]

            ## ranker has to have the exact same results
            ## e.g. every candidate of the qa handler has also to be a candidate of the ranker
            ranker_idx_rules = tail_rules_ranker_idx[rel_idx][source_idx][cand_idx]
            ranker_str_rules = tail_rules_ranker_str[rel_str][source_str][cand_str]

              # check that cand is also in ranking results
            idx_ranking_cands = [cand[0] for cand in tailRanking_idx[rel_idx][source_idx]]
            assert (cand_idx in idx_ranking_cands)
            str_ranking_cands = [ cand[0] for cand in tailRanking_str[rel_str][source_str]]
            assert(cand_str in str_ranking_cands)

            # rules collected from qa and ranker must be same for the query
            assert(idx_rules == ranker_idx_rules)
            assert(str_rules==ranker_str_rules)
            # check that the rule in idx format map back correctly to the strings of string rules
            assert(rule_index[idx_rules[-1]] == str_rules[-1])
        

    for i in range(len(head_queries)):
        source_str = head_queries[i][0]
        rel_str = head_queries[i][1]

        source_idx = get_entity_index[source_str]
        rel_idx = get_relation_index[rel_str]

        for c in range(len(head_answers_idx[i])):
            cand_idx = head_answers_idx[i][c][0]
            cand_str = head_answers_str[i][c][0]

            idx_rules = head_rules_idx[i][c]
            str_rules = head_rules_str[i][c]

            ## ranker has to have the exact same results
            ## e.g. every candidate of the qa handler has also to be a candidate of the ranker
            ranker_idx_rules = head_rules_ranker_idx[rel_idx][source_idx][cand_idx]
            ranker_str_rules = head_rules_ranker_str[rel_str][source_str][cand_str]

            # check that cand is also in ranking results
            idx_ranking_cands = [cand[0] for cand in headRanking_idx[rel_idx][source_idx]]
            assert (cand_idx in idx_ranking_cands)
            str_ranking_cands = [ cand[0] for cand in headRanking_str[rel_str][source_str]]
            assert(cand_str in str_ranking_cands)

            # rules collected from qa and ranker must be same for the query
            assert(idx_rules == ranker_idx_rules)
            assert(str_rules==ranker_str_rules)
            # check that the rule in idx format map back correctly to the strings of string rules
            assert(rule_index[idx_rules[-1]] == str_rules[-1])

    print("Test of consistency of rule tracking between QA and Ranking successful.")