
from triples import TripleSet,TripleIndex
from rules import Rule,RuleZ,RuleXXuc,RuleXXud,RuleUc,RuleUd,RuleB,RuleSet

import config
import c_clause

import time
import math

import multiprocessing as mp

class Torm():

    def __init__(self, targets, triples):
        self.targets = targets
        self.triples = triples
        self.rules = RuleSet(triples.index)
        self.c_clause_handler = None

    def set_c_clause_handler(self, train_path):
        self.c_clause_handler = c_clause.RuleHandler(train_path)

    def mine_z_rules(self):
        print(">>> mining z-rules ...")
        for hr in self.triples.rels:
            for sub in self.triples.r2_sub[hr]:
                num_obj = len(self.triples.sub_rel_2_obj[sub][hr])
                preds = len(self.triples.r2_obj[hr])
                if num_obj >= config.rules["z"]["support"] and num_obj / preds >= config.rules["z"]["confidence"]:
                    rule = RuleZ(self.rules, hr, sub, False)
                    rule.pred = preds
                    rule.cpred = num_obj
            for obj in self.triples.r2_obj[hr]:
                num_sub = len(self.triples.obj_rel_2_sub[obj][hr])
                preds = len(self.triples.r2_sub[hr])
                if num_sub >= config.rules["z"]["support"] and num_sub / preds >= config.rules["z"]["confidence"]:
                    rule = RuleZ(self.rules, hr, obj, True)
                    rule.pred = preds
                    rule.cpred = num_sub

    def mine_ud_rules(self):
        print(">>> mining ud-rules ...")
        for br in self.triples.rels:
            if len(self.triples.r2_obj[br]) >= config.rules["ud"]["support"]:
                preds = len(self.triples.r2_obj[br])
                (statsx, statsy) = self.mine_u_rule_heads(self.triples.r2_obj[br])
                self.create_ud_rules_from_stats(statsx, br, preds, True, False)
                self.create_ud_rules_from_stats(statsy, br, preds, False, False)
        for br in self.triples.rels:
            if len(self.triples.r2_sub[br]) >= config.rules["ud"]["support"]:
                preds = len(self.triples.r2_sub[br])
                (statsx, statsy) = self.mine_u_rule_heads(self.triples.r2_sub[br])
                self.create_ud_rules_from_stats(statsx, br, preds, True, True)
                self.create_ud_rules_from_stats(statsy, br, preds, False, True)


    def mine_uc_rules(self):
        print(">>> mining uc-rules ...")
        for bc in self.triples.sub_rel_2_obj:
            for br in self.triples.sub_rel_2_obj[bc]:
                if len(self.triples.sub_rel_2_obj[bc][br]) >= config.rules["uc"]["support"]:
                    preds = len(self.triples.sub_rel_2_obj[bc][br])
                    (statsx, statsy) = self.mine_u_rule_heads(self.triples.sub_rel_2_obj[bc][br])
                    self.create_uc_rules_from_stats(statsx, br, bc, preds, True, False)
                    self.create_uc_rules_from_stats(statsy, br, bc, preds, False, False)
        for bc in self.triples.obj_rel_2_sub:
            for br in self.triples.obj_rel_2_sub[bc]:
                if len(self.triples.obj_rel_2_sub[bc][br]) >= config.rules["uc"]["support"]:
                    preds = len(self.triples.obj_rel_2_sub[bc][br])
                    (statsx, statsy) = self.mine_u_rule_heads(self.triples.obj_rel_2_sub[bc][br])
                    self.create_uc_rules_from_stats(statsx, br, bc, preds, True, True)
                    self.create_uc_rules_from_stats(statsy, br, bc, preds, False, True)


    def mine_u_rule_heads(self, xys):
        statsx = {}
        statsy = {}
        for xy in xys:
            if xy in self.triples.sub_rel_2_obj:
                for hr in self.triples.sub_rel_2_obj[xy]:
                    if not(hr in statsx): statsx[hr] = {}
                    for hc in self.triples.sub_rel_2_obj[xy][hr]:
                        if not(hc in statsx[hr]): statsx[hr][hc] = 1
                        else: statsx[hr][hc] = statsx[hr][hc] + 1
            if xy in self.triples.obj_rel_2_sub:
                for hr in self.triples.obj_rel_2_sub[xy]:
                    if not(hr in statsy): statsy[hr] = {}
                    for hc in self.triples.obj_rel_2_sub[xy][hr]:
                        if not(hc in statsy[hr]): statsy[hr][hc] = 1
                        else: statsy[hr][hc] = statsy[hr][hc] + 1
        return (statsx, statsy)

    def create_uc_rules_from_stats(self, stats, br, bc, preds, hc_right, bc_right):
        for hr in stats:
            for hc in stats[hr]:
                if stats[hr][hc] >= config.rules["uc"]["support"] and stats[hr][hc] / preds >= config.rules["uc"]["confidence"] :
                    rule = RuleUc(self.rules, hr, br, hc, hc_right, bc, bc_right)
                    rule.pred = preds
                    rule.cpred = stats[hr][hc]

    def create_ud_rules_from_stats(self, stats, br, preds, hc_right, dangling_right):
        for hr in stats:
            for hc in stats[hr]:
                if stats[hr][hc] >= config.rules["ud"]["support"] and stats[hr][hc] / preds >= config.rules["ud"]["confidence"] :
                    rule = RuleUd(self.rules, hr, br, hc, hc_right, dangling_right)
                    rule.pred = preds
                    rule.cpred = stats[hr][hc]


    def mine_xx_uc_rules(self):
        print(">>> mining xx-rules (with uc bodies) ...")
        for bc in self.triples.sub_rel_2_obj:
            for br in self.triples.sub_rel_2_obj[bc]:
                if len(self.triples.sub_rel_2_obj[bc][br]) >= config.rules["xx_uc"]["support"]:
                    preds = len(self.triples.sub_rel_2_obj[bc][br])
                    stats = self.mine_xx_u_rule_heads(self.triples.sub_rel_2_obj[bc][br])
                    self.create_xx_uc_rules_from_stats(self.rules, stats, br, bc, preds, False)
        for bc in self.triples.obj_rel_2_sub:
            for br in self.triples.obj_rel_2_sub[bc]:
                if len(self.triples.obj_rel_2_sub[bc][br]) >= config.rules["xx_uc"]["support"]:
                    preds = len(self.triples.obj_rel_2_sub[bc][br])
                    stats = self.mine_xx_u_rule_heads(self.triples.obj_rel_2_sub[bc][br])
                    self.create_xx_uc_rules_from_stats(self.rules, stats, br, bc, preds, True)


    def mine_xx_ud_rules(self):
        print(">>> mining xx-rules (with ud bodies) ...")
        for br in self.triples.rels:
            if len(self.triples.r2_obj[br]) >= config.rules["xx_ud"]["support"]:
                preds = len(self.triples.r2_obj[br])
                stats = self.mine_xx_u_rule_heads(self.triples.r2_obj[br])
                self.create_xx_ud_rules_from_stats(self.rules, stats, br, preds, False)
        for br in self.triples.rels:
            if len(self.triples.r2_sub[br]) >= config.rules["xx_ud"]["support"]:
                preds = len(self.triples.r2_sub[br])
                stats = self.mine_xx_u_rule_heads(self.triples.r2_sub[br])
                self.create_xx_ud_rules_from_stats(self.rules, stats, br, preds, True)


    def mine_xx_u_rule_heads(self, xys):
        stats = {}
        for xy in xys:
            if xy in self.triples.sub_rel_2_obj:
                for hr in self.triples.sub_rel_2_obj[xy]:
                    if not(hr in stats): stats[hr] = 0
                    if xy in self.triples.sub_rel_2_obj[xy][hr]:
                        stats[hr] = stats[hr] + 1
        return stats
    
    def create_xx_uc_rules_from_stats(self, rules, stats, br, bc, preds, bc_right):
        for hr in stats:
            if stats[hr] >= config.rules["xx_uc"]["support"] and stats[hr] / preds >= config.rules["xx_uc"]["confidence"]:
                rule = RuleXXuc(rules, hr, br, bc, bc_right)
                rule.pred = preds
                rule.cpred = stats[hr]

    def create_xx_ud_rules_from_stats(self, rules, stats, br, preds, dangling_right):
        for hr in stats:
            if stats[hr] >= config.rules["xx_ud"]["support"] and stats[hr] / preds >= config.rules["xx_ud"]["confidence"]:
                pass
                rule = RuleXXud(rules, hr, br, dangling_right)
                rule.pred = preds
                rule.cpred = stats[hr]

    # -ruleid sub1 obj1 subj obj1  
    def mine_b_rules(self, pid, lock, prediction_data, candidates, train_path, index, sender):
        self.set_c_clause_handler(train_path)
        lock.acquire()
        info_steps = (len(candidates) // 200) + 1
        counter = 0
        data_index = 0
        for candidate in candidates:
            if pid == 0 and counter % info_steps == 0:
                sender.send(counter/len(candidates))
            data_index, success = candidate.score(self.triples, lock, prediction_data, data_index, self.c_clause_handler)
            counter += 1
        if data_index < len(prediction_data):
            prediction_data[data_index] = 0   
        lock.release()

    def mine_b_rule_candidates(self, num_of_atoms):
        candidates = RuleSet(self.triples.index)
        ltf = self.directions_combinations(num_of_atoms)
        count = 0
        current = time.time()
        for target in self.targets:
            count += 1
            # print(">>> [" + str(count) + "] constructing b-rules for " + str(id2to[target]) + " ... so far " +  str(len(candidates.rules)) + " > " + str(time.time()))
            if time.time() - 10 > current:
                current = time.time()
                print(">>> ... still constructing b-rule candidates, " + str(count) + " out of " + str(len(self.targets)) + " relations processed so far ...") 
            for pattern_l in ltf:
                for pattern in pattern_l:
                    self.searchX(candidates, (target,), 0, pattern)
        return candidates

    def directions_combinations(self, num_body_atoms):
        ltf = [[(True,), (False,)]]
        for i in range(1,num_body_atoms):
            self.increase_length(ltf)
        return ltf

    def increase_length(self, ltf):
        ln = []
        for p in ltf[-1]:
            ln.append((*p, True))
            ln.append((*p, False))
        ltf.append(ln)

    def searchX(self, candidates, relations, index, pattern):
        i = 0
        j = 0
        if index % 2 == 0:
            i = index // 2
            j = i+1
        else:
            i = -(index // 2)
            j = i-1
        if i > 0: i -= 1
        if j > 0: j -= 1
        # standard case where you have to go deeper
        r = relations[i]
        r2r = None
        if index == 0 or index == 1:
            if j == 0:
                if pattern[j]: r2r =self.triples.r2r_ss[r]
                if not(pattern[j]): r2r =self.triples.r2r_so[r]
            else:
                if pattern[j]: r2r =self.triples.r2r_oo[r]
                if not(pattern[j]): r2r =self.triples.r2r_os[r]
        else:
            if j > 0:
                if pattern[i] and pattern[j]: r2r = self.triples.r2r_os[r]
                elif pattern[i] and not(pattern[j]): r2r = self.triples.r2r_oo[r]
                elif not(pattern[i]) and pattern[j]: r2r = self.triples.r2r_ss[r]
                elif not(pattern[i]) and not(pattern[j]): r2r = self.triples.r2r_so[r]
            if j < 0:
                if pattern[i] and pattern[j]: r2r = self.triples.r2r_so[r]
                elif pattern[i] and not(pattern[j]): r2r = self.triples.r2r_ss[r]
                elif not(pattern[i]) and pattern[j]: r2r = self.triples.r2r_oo[r]
                elif not(pattern[i]) and not(pattern[j]): r2r = self.triples.r2r_os[r]
        if index < len(pattern):
            for r_next in r2r:
                self.search(candidates, (*relations, r_next), index+1, pattern)
        else:
            if relations[-1] in r2r:
                rels_ordered = [relations[0]]
                rels_ordered.extend(relations[1::2])
                back_rels = relations[2::2]
                back_rels.reverse()
                rels_ordered.extend(back_rels)
                rule = RuleB(rels_ordered, pattern)
                if not(rule.trivial()): candidates.add_rule(rule)

    def search(self, candidates, relations, index, pattern):
        if index == len(pattern):
            if pattern[-1]:
                if relations[0] in self.triples.r2r_oo[relations[-1]]:
                    rule = RuleB(candidates, relations, pattern)
                    if not(rule.trivial()): candidates.add_rule(rule)
            else:
                if relations[0] in self.triples.r2r_so[relations[-1]]:
                    rule = RuleB(candidates, relations, pattern)
                    if not(rule.trivial()):  candidates.add_rule(rule)
            return
        # standard case where you have to go deeper
        r = relations[index]
        r2r = False
        if index == 0:
            if pattern[index]: r2r =self.triples.r2r_ss[r]
            if not(pattern[index]): r2r =self.triples.r2r_so[r]
        else:
            if pattern[index-1] and pattern[index]: r2r = self.triples.r2r_os[r]   ## check if all os have to be reversed !!!
            if pattern[index-1] and not(pattern[index]): r2r = self.triples.r2r_oo[r]
            if not(pattern[index-1]) and pattern[index]: r2r = self.triples.r2r_ss[r]
            if not(pattern[index-1]) and not(pattern[index]): r2r = self.triples.r2r_so[r]
        for r_next in r2r:
            #if self.triples.get_1to1_score(r_next) < 0.05:
            #    continue
            self.search(candidates, (*relations, r_next), index+1, pattern)
            rule = RuleB(candidates, relations, pattern)


    def mine_rules(self):
        try:
            mp.set_start_method(config.multithreading["start_method"])
        except:
            print(">>> you are running on a plaform that does not support " + str(config.multithreading["start_method"]) + " to start a process, using spawn as fallback")
            mp.set_start_method('spawn')

        num_of_processes = config.multithreading["worker"]

        # ***********************************
        if config.rules['b']['active']:
        
            print(">>> mining b-rules ...")
            candidates = self.mine_b_rule_candidates(config.rules['b']['length'])
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
                for pid in range(num_of_processes):
                    prediction_data.append(mp.Array('i', config.multithreading["brules_dataarray_size"]))
                    locks.append(mp.Lock())
                procs = []
                for pid in range(num_of_processes):
                    if pid == 0:
                        proc = mp.Process(target=self.mine_b_rules, args=(pid, locks[pid], prediction_data[pid], candidate_lists[pid], self.triples.path, self.triples.index, sender))
                    else:
                        proc = mp.Process(target=self.mine_b_rules, args=(pid, locks[pid], prediction_data[pid], candidate_lists[pid], self.triples.path, self.triples.index, None))
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
                                self.rules.transfer_prediction_data(pid, prediction_data[pid], candidates)
                                print(">>>  ~> rule scores computed by worker #" + str(pid) + " transferred to main process (buffer was full)")
                                locks[pid].release()
                                time.sleep(0.1)
                        else:
                            if not(pid in fetched):
                                self.rules.transfer_prediction_data(pid, prediction_data[pid], candidates)
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
                    self.rules.transfer_prediction_data(pid, prediction_data[pid], candidates)
            end = time.time()
            print(">>> elapsed time for materialization of " + str(self.rules.size()) + " b-rules: " + str(math.floor(end-start)) + "s") 


        if config.rules['uc']['active']:
            start = time.time()
            prev_rule_count = self.rules.size()
            self.mine_uc_rules()
            uc_rule_count = self.rules.size() - prev_rule_count
            end = time.time()
            print(">>> elapsed time for materialization of " + str(uc_rule_count) + " uc-rules: " + str(math.floor(end-start)) + "s") # time in seconds

        if config.rules['ud']['active']:
            start = time.time()
            prev_rule_count = self.rules.size()
            self.mine_ud_rules()
            uc_rule_count = self.rules.size() - prev_rule_count
            end = time.time()
            print(">>> elapsed time for materialization of " + str(uc_rule_count) + " ud-rules: " + str(math.floor(end-start)) + "s") # time in seconds

        if config.rules['z']['active']:
            start = time.time()
            prev_rule_count = self.rules.size()
            self.mine_z_rules()
            uc_rule_count = self.rules.size() - prev_rule_count
            end = time.time()
            print(">>> elapsed time for materialization of " + str(uc_rule_count) + " ud-rules: " + str(math.floor(end-start)) + "s") # time in seconds

        if config.rules['xx_uc']['active']:
            start = time.time()
            prev_rule_count = self.rules.size()
            self.mine_xx_uc_rules()
            xx_rule_count = self.rules.size() - prev_rule_count
            end = time.time()
            print(">>> ... elapsed time for materialization of " + str(xx_rule_count) + " xx_uc-rules: " + str(math.floor(end-start)) + "s") 

        if config.rules['xx_ud']['active']:
            start = time.time()
            prev_rule_count = self.rules.size()
            self.mine_xx_ud_rules()
            xx_rule_count = self.rules.size() - prev_rule_count
            end = time.time()
            print(">>> ... elapsed time for materialization of " + str(xx_rule_count) + " xx_ud-rules: " + str(math.floor(end-start)) + "s") 

        print(">>> ~~~ DONE with mining rules! ~~~")
        print(">>> rules collected " +  str(self.rules.size()))

