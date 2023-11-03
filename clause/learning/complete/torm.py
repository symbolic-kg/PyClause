
from clause.data.triples import TripleSet,TripleIndex
from clause.rules.rules import Rule,RuleZ,RuleXXuc,RuleXXud,RuleUc,RuleUd,RuleB,RuleSet

import clause.config.config as config
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
        self.c_clause_loader = None

    def set_c_clause_handler(self):
        options = {}
        self.c_clause_loader = c_clause.DataHandler(options)
        self.c_clause_loader.load_data(self.triples.path)
        self.c_clause_handler = c_clause.RulesHandler()

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
                    rule.store()
            for obj in self.triples.r2_obj[hr]:
                num_sub = len(self.triples.obj_rel_2_sub[obj][hr])
                preds = len(self.triples.r2_sub[hr])
                if num_sub >= config.rules["z"]["support"] and num_sub / preds >= config.rules["z"]["confidence"]:
                    rule = RuleZ(self.rules, hr, obj, True)
                    rule.pred = preds
                    rule.cpred = num_sub
                    rule.store()

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
        current_time = time.time()
        bc_count = 0

        for bc in self.triples.sub_rel_2_obj:
            bc_count += 1
            next_time = time.time()
            if (current_time + 5 < next_time):
                current_time = next_time
                frac = bc_count / (len(self.triples.sub_rel_2_obj) + len(self.triples.obj_rel_2_sub))
                print(">>> ...  mined ~" + str(round(frac * 100)) + "%s")
            for br in self.triples.sub_rel_2_obj[bc]:
                if len(self.triples.sub_rel_2_obj[bc][br]) >= config.rules["uc"]["support"]:
                    preds = len(self.triples.sub_rel_2_obj[bc][br])
                    (statsx, statsy) = self.mine_u_rule_heads(self.triples.sub_rel_2_obj[bc][br])
                    self.create_uc_rules_from_stats(statsx, br, bc, preds, True, False)
                    self.create_uc_rules_from_stats(statsy, br, bc, preds, False, False)
        for bc in self.triples.obj_rel_2_sub:
            if (current_time + 5 < next_time):
                current_time = next_time
                frac = bc_count / (len(self.triples.sub_rel_2_obj) + len(self.triples.obj_rel_2_sub))
                print(">>> ...  mined ~" + str(round(frac * 100)) + "%")
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
                    # ef __init__(self, ruleset, target, rels, dirs, hc, hc_right, bc):
                    rule = RuleUc(self.rules, hr, (br,), (bc_right,), hc, hc_right, bc)
                    rule.pred = preds
                    rule.cpred = stats[hr][hc]
                    rule.store()

    def create_ud_rules_from_stats(self, stats, br, preds, hc_right, dangling_right):
        for hr in stats:
            for hc in stats[hr]:
                if stats[hr][hc] >= config.rules["ud"]["support"] and stats[hr][hc] / preds >= config.rules["ud"]["confidence"] :
                    rule = RuleUd(self.rules, hr, (br,), (dangling_right,), hc, hc_right)
                    rule.pred = preds
                    rule.cpred = stats[hr][hc]
                    rule.store()


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
                rule.store()

    def create_xx_ud_rules_from_stats(self, rules, stats, br, preds, dangling_right):
        for hr in stats:
            if stats[hr] >= config.rules["xx_ud"]["support"] and stats[hr] / preds >= config.rules["xx_ud"]["confidence"]:
                pass
                rule = RuleXXud(rules, hr, br, dangling_right)
                rule.pred = preds
                rule.cpred = stats[hr]
                rule.store()

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
                    self.search(candidates, (target,), 0, pattern)
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

    # probably you can delete this method
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

    def search(self, candidates, relations, index, pattern):
        if index == len(pattern):
            if pattern[-1]:
                if relations[0] in self.triples.r2r_oo[relations[-1]]:
                    if (len(pattern) != 1 or relations[0] != relations[1]):
                        rule = RuleB(candidates, relations[0], relations[1:], pattern)
                        if not(rule.trivial()): rule.store()
            else:
                if relations[0] in self.triples.r2r_so[relations[-1]]:
                    rule = RuleB(candidates, relations[0], relations[1:], pattern)
                    if not(rule.trivial()): rule.store()
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
            #rule = RuleB(candidates, relations[0], relations[1:], pattern)


    def mine_rules(self):
       
        # ***********************************
        if config.rules['b']['active']:
        
            print(">>> mining b-rules ...")
            candidates = self.mine_b_rule_candidates(config.rules['b']['length'])
            print(">>> constructed " + str(candidates.size()) + " b-rules candidates, materialization started ...")

            start = time.time()
            
            batches = math.ceil(candidates.size() / config.rules['b']['batchsize'])
            candidate_lists = []

            for i in range(0,batches):
                candidate_lists.append([])

            j = 0
            for c in candidates.rules:
                j += 1
                index = j % batches
                candidate_lists[index].append(c)


            print(">>> divided candidates into " + str(batches) + " batches")
            self.set_c_clause_handler()
            j = 0
            for clist in candidate_lists:
                rlist = list(map(lambda x : str(x), clist))
                preds = self.c_clause_handler.stats_and_predictions(rlist, self.c_clause_loader,  False, True)
                print(">>> ... batch #" + str(j) + " of " + str(batches))
                #print("len clist = " + str(len(clist)))
                #print("len rlist = " + str(len(rlist)))
                #print("len preds[1]= " + str(len(preds[1])))
                for i in range(len(preds[1])):
                    rule = clist[i]
                    (pred,cpred) = preds[1][i]
                    if cpred >= config.rules["b"]["support"] and cpred / pred >= config.rules["b"]["confidence"]:
                        rule.pred = pred
                        rule.cpred = cpred
                        self.rules.add_rule(rule)
                        #if self.cpred == 0:
                        #    print(str(rule))
                        #    exit
                j += 1
            # exit()
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

