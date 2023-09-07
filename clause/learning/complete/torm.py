from triples import Triple,TripleSet,PredictionSet,to2id,id2to
from rules import RuleZ,RuleXXuc,RuleXXud,RuleUc,RuleUd,RuleB,RuleSet
import time
import math
import config
import c_clause

class Torm():

    def __init__(self, targets, triples, rules):
        self.targets = targets
        self.triples = triples
        self.rules = rules
        self.c_clause_handler = None

    def set_c_clause_handler(self, train_path):
        self.c_clause_handler = c_clause.RuleHandler(train_path)

    def mine_z_rules(self, rules):
        print(">>> mining z-rules ...")
        for hr in self.triples.rels:
            for sub in self.triples.r2_sub[hr]:
                num_obj = len(self.triples.sub_rel_2_obj[sub][hr])
                preds = len(self.triples.r2_obj[hr])
                if num_obj >= config.rules["z"]["support"] and num_obj / preds >= config.rules["z"]["confidence"]:
                    rule = RuleZ(hr, sub, False)
                    rule.pred = preds
                    rule.cpred = num_obj
                    rules.add_rule(rule)
            for obj in self.triples.r2_obj[hr]:
                num_sub = len(self.triples.obj_rel_2_sub[obj][hr])
                preds = len(self.triples.r2_sub[hr])
                if num_sub >= config.rules["z"]["support"] and num_sub / preds >= config.rules["z"]["confidence"]:
                    rule = RuleZ(hr, obj, True)
                    rule.pred = preds
                    rule.cpred = num_sub
                    rules.add_rule(rule)                


    def mine_ud_rules(self, rules):
        print(">>> mining ud-rules ...")
        for br in self.triples.rels:
            if len(self.triples.r2_obj[br]) >= config.rules["ud"]["support"]:
                preds = len(self.triples.r2_obj[br])
                (statsx, statsy) = self.mine_u_rule_heads(self.triples.r2_obj[br])
                self.create_ud_rules_from_stats(rules, statsx, br, preds, True, False)
                self.create_ud_rules_from_stats(rules, statsy, br, preds, False, False)
        for br in self.triples.rels:
            if len(self.triples.r2_sub[br]) >= config.rules["ud"]["support"]:
                preds = len(self.triples.r2_sub[br])
                (statsx, statsy) = self.mine_u_rule_heads(self.triples.r2_sub[br])
                self.create_ud_rules_from_stats(rules, statsx, br, preds, True, True)
                self.create_ud_rules_from_stats(rules, statsy, br, preds, False, True)


    def mine_uc_rules(self, rules):
        print(">>> mining uc-rules ...")
        for bc in self.triples.sub_rel_2_obj:
            for br in self.triples.sub_rel_2_obj[bc]:
                if len(self.triples.sub_rel_2_obj[bc][br]) >= config.rules["uc"]["support"]:
                    preds = len(self.triples.sub_rel_2_obj[bc][br])
                    (statsx, statsy) = self.mine_u_rule_heads(self.triples.sub_rel_2_obj[bc][br])
                    self.create_uc_rules_from_stats(rules, statsx, br, bc, preds, True, False)
                    self.create_uc_rules_from_stats(rules, statsy, br, bc, preds, False, False)
        for bc in self.triples.obj_rel_2_sub:
            for br in self.triples.obj_rel_2_sub[bc]:
                if len(self.triples.obj_rel_2_sub[bc][br]) >= config.rules["uc"]["support"]:
                    preds = len(self.triples.obj_rel_2_sub[bc][br])
                    (statsx, statsy) = self.mine_u_rule_heads(self.triples.obj_rel_2_sub[bc][br])
                    self.create_uc_rules_from_stats(rules, statsx, br, bc, preds, True, True)
                    self.create_uc_rules_from_stats(rules, statsy, br, bc, preds, False, True)


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

    def create_uc_rules_from_stats(self, rules, stats, br, bc, preds, hc_right, bc_right):
        for hr in stats:
            for hc in stats[hr]:
                if stats[hr][hc] >= config.rules["uc"]["support"] and stats[hr][hc] / preds >= config.rules["uc"]["confidence"] :
                    rule = RuleUc(hr, br, hc, hc_right, bc, bc_right)
                    rule.pred = preds
                    rule.cpred = stats[hr][hc]
                    rules.add_rule(rule)

    def create_ud_rules_from_stats(self, rules, stats, br, preds, hc_right, dangling_right):
        for hr in stats:
            for hc in stats[hr]:
                if stats[hr][hc] >= config.rules["ud"]["support"] and stats[hr][hc] / preds >= config.rules["ud"]["confidence"] :
                    rule = RuleUd(hr, br, hc, hc_right, dangling_right)
                    rule.pred = preds
                    rule.cpred = stats[hr][hc]
                    rules.add_rule(rule)


    def mine_xx_uc_rules(self, rules):
        print(">>> mining xx-rules (with uc bodies) ...")
        for bc in self.triples.sub_rel_2_obj:
            for br in self.triples.sub_rel_2_obj[bc]:
                if len(self.triples.sub_rel_2_obj[bc][br]) >= config.rules["xx_uc"]["support"]:
                    preds = len(self.triples.sub_rel_2_obj[bc][br])
                    stats = self.mine_xx_u_rule_heads(self.triples.sub_rel_2_obj[bc][br])
                    self.create_xx_uc_rules_from_stats(rules, stats, br, bc, preds, False)
        for bc in self.triples.obj_rel_2_sub:
            for br in self.triples.obj_rel_2_sub[bc]:
                if len(self.triples.obj_rel_2_sub[bc][br]) >= config.rules["xx_uc"]["support"]:
                    preds = len(self.triples.obj_rel_2_sub[bc][br])
                    stats = self.mine_xx_u_rule_heads(self.triples.obj_rel_2_sub[bc][br])
                    self.create_xx_uc_rules_from_stats(rules, stats, br, bc, preds, True)


    def mine_xx_ud_rules(self, rules):
        print(">>> mining xx-rules (with ud bodies) ...")
        for br in self.triples.rels:
            if len(self.triples.r2_obj[br]) >= config.rules["xx_ud"]["support"]:
                preds = len(self.triples.r2_obj[br])
                stats = self.mine_xx_u_rule_heads(self.triples.r2_obj[br])
                self.create_xx_ud_rules_from_stats(rules, stats, br, preds, False)
        for br in self.triples.rels:
            if len(self.triples.r2_sub[br]) >= config.rules["xx_ud"]["support"]:
                preds = len(self.triples.r2_sub[br])
                stats = self.mine_xx_u_rule_heads(self.triples.r2_sub[br])
                self.create_xx_ud_rules_from_stats(rules, stats, br, preds, True)


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
                rule = RuleXXuc(hr, br, bc, bc_right)
                rule.pred = preds
                rule.cpred = stats[hr]
                rules.add_rule(rule)


    def create_xx_ud_rules_from_stats(self, rules, stats, br, preds, dangling_right):
        for hr in stats:
            if stats[hr] >= config.rules["xx_ud"]["support"] and stats[hr] / preds >= config.rules["xx_ud"]["confidence"]:
                pass
                rule = RuleXXud(hr, br, dangling_right)
                rule.pred = preds
                rule.cpred = stats[hr]
                rules.add_rule(rule)

    # -ruleid sub1 obj1 subj obj1  
    def mine_b_rules(self, pid, lock, prediction_data, candidates, train_path, id2to, sender):
        self.set_c_clause_handler(train_path)
        lock.acquire()
        info_steps = (len(candidates) // 200) + 1
        counter = 0
        data_index = 0
        for candidate in candidates:
            if pid == 0 and counter % info_steps == 0:
                sender.send(counter/len(candidates))
            data_index, success = candidate.score(pid, self.triples, lock, prediction_data, data_index, self.c_clause_handler, id2to)
            counter += 1
        if data_index < len(prediction_data):
            prediction_data[data_index] = 0   
        lock.release()

    def mine_b_rule_candidates(self, num_of_atoms):
        candidates = RuleSet()
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
                    # print(str(pattern))
                    # self.search(candidates, (target,), 0, pattern)
                    # --- new code ---
                    self.searchX(candidates, (target,), 0, pattern)
                    # --- new code ---
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
                    rule = RuleB(relations, pattern)
                    if not(rule.trivial()): candidates.add_rule(rule)
            else:
                if relations[0] in self.triples.r2r_so[relations[-1]]:
                    rule = RuleB(relations, pattern)
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
            rule = RuleB(relations, pattern)