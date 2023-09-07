from torm import Torm
from triples import Triple,TripleSet,PredictionSet,CompletionSet,id2to
import triples as ttt

from rules import RuleSet
import config

import multiprocessing as mp
import time
import math


if __name__ == '__main__':

    try:
        mp.set_start_method(config.multithreading["start_method"])
    except:
        print(">>> you are running on a plaform that does not support " + str(config.multithreading["start_method"]) + " to start a process, using spawn as fallback")
        mp.set_start_method('spawn')

    rules = RuleSet()
    triples = TripleSet(config.paths["input"]["train"])
    task_triples = TripleSet(config.paths["input"]["tasks"])

    targets = triples.rels
    # targets = list(map(lambda x : ttt.to2id[x], ["P37"]))
    num_of_processes = config.multithreading["worker"]
    torm = Torm(targets, triples, rules)

    # ***********************************
    if config.rules['b']['active']:
       
        print(">>> mining b-rules ...")
        candidates = torm.mine_b_rule_candidates(config.rules['b']['length'])
        print(">>> constructed " + str(candidates.size()) + " b-rules candidates, materialization started using " + str(num_of_processes) + " processes ...")
       
        start = time.time()
        
        
        candidate_lists = []
        for i in range(num_of_processes): candidate_lists.append([])
        j = 0
        for c in candidates.rules:
            j += 1
            index = j % num_of_processes
            candidate_lists[index].append(c)
    
        with mp.Manager() as manager:
            reciever, sender = mp.Pipe()
            prediction_data = []
            locks = []
            # xxx rules_by_process = manager.list()
            for pid in range(num_of_processes):
                prediction_data.append(mp.Array('i', config.multithreading["brules_dataarray_size"]))
                locks.append(mp.Lock())
                # rules_by_process.append(-1)
            procs = []
            for pid in range(num_of_processes):
                if pid == 0:
                    proc = mp.Process(target=torm.mine_b_rules, args=(pid, locks[pid], prediction_data[pid], candidate_lists[pid], config.paths["input"]["train"], ttt.id2to, sender))
                else:
                    proc = mp.Process(target=torm.mine_b_rules, args=(pid, locks[pid], prediction_data[pid], candidate_lists[pid], config.paths["input"]["train"], ttt.id2to, None))
                procs.append(proc)
                proc.start()

            still_running = True
            fetched = set()
            progress = 0.0
            previous_time = 0
            while still_running:
                time.sleep(0.1)
                alive_counter = 0
                pid = 0
                for proc in procs:
                    if proc.is_alive():
                        alive_counter += 1
                        if locks[pid].acquire(block=False):
                            rules.transfer_prediction_data(pid, prediction_data[pid], candidates)
                            print(">>>  ~> rule scores computed by worker #" + str(pid) + " transferred to main process (buffer was full)")
                            locks[pid].release()
                            time.sleep(0.1)
                    else:
                        if not(pid in fetched):
                            rules.transfer_prediction_data(pid, prediction_data[pid], candidates)
                            fetched.add(pid)
                            print(">>>  ~> rule scores computed by worker #" + str(pid) + " transferred to main process")
                    pid += 1
                current_time = time.time()
                if current_time - previous_time > 1:
                    previous_time = current_time
                    if 0 in fetched:
                        print(">>> ... running for " + str(math.floor(current_time - start)) + "s, " + str(alive_counter)+ " subprocesses alive")
                    else:
                        while reciever.poll():
                            progress = reciever.recv()
                        print(">>> ... running for " + str(math.floor(current_time - start)) + "s, " + str(alive_counter)+ " subprocesses alive, process #0 has materialized " + "{0:.1%}".format(progress) +" of its rules")
                if alive_counter == 0:
                    still_running = False
            for pid in range(num_of_processes):
                rules.transfer_prediction_data(pid, prediction_data[pid], candidates)
        end = time.time()
        print(">>> elapsed time for materialization of " + str(rules.size()) + " b-rules: " + str(math.floor(end-start)) + "s") 
    # ***********************************
    

    targets_uc = []
    targets_uc.extend(targets)

    targets_uc.sort(key=lambda x:triples.r2_count[x], reverse=True)

    if config.rules['uc']['active']:
        start = time.time()
        prev_rule_count = rules.size()
        torm.mine_uc_rules(rules)
        uc_rule_count = rules.size() - prev_rule_count
        end = time.time()
        print(">>> elapsed time for materialization of " + str(uc_rule_count) + " uc-rules: " + str(math.floor(end-start)) + "s") # time in seconds


    if config.rules['ud']['active']:
        start = time.time()
        prev_rule_count = rules.size()
        torm.mine_ud_rules(rules)
        uc_rule_count = rules.size() - prev_rule_count
        end = time.time()
        print(">>> elapsed time for materialization of " + str(uc_rule_count) + " ud-rules: " + str(math.floor(end-start)) + "s") # time in seconds

    if config.rules['z']['active']:
        start = time.time()
        prev_rule_count = rules.size()
        torm.mine_z_rules(rules)
        uc_rule_count = rules.size() - prev_rule_count
        end = time.time()
        print(">>> elapsed time for materialization of " + str(uc_rule_count) + " ud-rules: " + str(math.floor(end-start)) + "s") # time in seconds

    if config.rules['xx_uc']['active']:
        start = time.time()
        prev_rule_count = rules.size()
        torm.mine_xx_uc_rules(rules)
        xx_rule_count = rules.size() - prev_rule_count
        end = time.time()
        print(">>> ... elapsed time for materialization of " + str(xx_rule_count) + " xx_uc-rules: " + str(math.floor(end-start)) + "s") 

    if config.rules['xx_ud']['active']:
        start = time.time()
        prev_rule_count = rules.size()
        torm.mine_xx_ud_rules(rules)
        xx_rule_count = rules.size() - prev_rule_count
        end = time.time()
        print(">>> ... elapsed time for materialization of " + str(xx_rule_count) + " xx_ud-rules: " + str(math.floor(end-start)) + "s") 

    print(">>> ~~~ DONE with mining rules! ~~~")
    print(">>> rules collected " +  str(rules.size()))

    rules.write(config.paths["output"]["rules"])
    print(">>> ~~~ rules written to " + config.paths["output"]["rules"])

    # **************************************

    #t1 = time.time()
    #print(">>> sorting rules that explain the predictions ...")
    #predictions.sort_explaining_rules()
    #t2 = time.time()
    #print(">>> ... sorting finished, it required " + str(t2 -t1) + " seconds")


    #completions = CompletionSet(task_triples, predictions)
    #print(">>> ... created completion set, now collecting")
    #completions.collect()
    #print(">>> ... collected, now filtering ...")
    #completions.filter(triples)
    #print(">>> ... filtered, now writing ...")

    #completions.write_ranking(config.paths["output"]["ranking"])
    #print(">>> ... cccc")
