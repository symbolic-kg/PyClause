import config
import sys
from triples import id2to,to2id
import time

from random import sample

from multiprocessing import Array



class Rule:

    id_counter = 0

    def __init__(self):
        Rule.id_counter += 1
        self.id = Rule.id_counter
        self.pred = 0
        self.cpred = 0
        self.hashcode = None
        self.aconfidence = 0.0 # this is the counterpart to the applied confidence
        
    def get_confidence(self):
        return self.cpred / self.pred

class RuleXXuc(Rule):
    """
    A rather specific rule of the form h(X,X) <= b(X,b). Note that this rule was represented in AnyBURL via a hack as a Uc
    rule. Here we implemented it as a rule on its own. However, this rule type still supports the output required to run as AnyBURL
    hack.
    """  

    def __init__(self, target, rel, bc, bc_right):
        super().__init__()
        self.target = target
        self.rel = rel
        self.bc = bc
        self.bc_right = bc_right

    def __str__(self):
        rep = ""
        body_left, body_right  = ("X",id2to[self.bc]) if self.bc_right else (id2to[self.bc],"X")
        rep += id2to[self.target] + "(X,X) <= " + id2to[self.rel] + "(" + body_left + "," + body_right + ")"
        return rep

    def __eq__(self, other):
        if type(self) != type(other): return False
        if self.target != other.target: return False
        if self.rel != other.rel: return False
        if self.bc != other.bc: return False
        if self.bc_right != other.bc_right: return False
        return True

    def __hash__(self):
        if (self.hashcode == None):
            self.hashcode = hash(str(self))
        return self.hashcode

    def materialize(self, triples, predictions): # TODO XXX
        x_r_2_y = triples.obj_rel_2_sub if self.bc_right else triples.sub_rel_2_obj
        predicted_sub_obj = []
        for value in x_r_2_y[self.bc][self.rel]:
            self.pred += 1
            predicted_sub_obj.append((value,value))
            if triples.is_known(value, self.target, value):
                self.cpred += 1
        if self.get_confidence() < config.rules['uxx']['confidence']: return False
        if self.cpred < config.rules['uxx']['support']: return False
        for predicted in predicted_sub_obj:
            predictions.add_prediction(predicted[0], self.target, predicted[1], self)
        return True
    
class RuleXXud(Rule):
    """
    A rather specific rule of the form h(X,X) <= b(X,A). Note that this rule was represented in AnyBURL via a hack as a Uc
    rule. Here we implemented it as a rule on its own. However, this rule type still supports the output required to run as AnyBURL
    hack.
    """  

    def __init__(self, target, rel, dangling_right):
        super().__init__()
        self.target = target
        self.rel = rel
        self.dangling_right = dangling_right

    def __str__(self):
        rep = ""
        body_left, body_right  = ("X","A") if self.dangling_right else ("A","X")
        rep += id2to[self.target] + "(X,X) <= " + id2to[self.rel] + "(" + body_left + "," + body_right + ")"
        return rep

    def __eq__(self, other):
        if type(self) != type(other): return False
        if self.target != other.target: return False
        if self.rel != other.rel: return False
        if self.dangling_right != other.dangling_right: return False
        return True

    def __hash__(self):
        if (self.hashcode == None):
            self.hashcode = hash(str(self))
        return self.hashcode

    def materialize(self, triples, predictions): # TODO XXX
        x_r_2_y = triples.obj_rel_2_sub if self.bc_right else triples.sub_rel_2_obj
        predicted_sub_obj = []
        for value in x_r_2_y[self.bc][self.rel]:
            self.pred += 1
            predicted_sub_obj.append((value,value))
            if triples.is_known(value, self.target, value):
                self.cpred += 1
        if self.get_confidence() < config.rules['uxx']['confidence']: return False
        if self.cpred < config.rules['uxx']['support']: return False
        for predicted in predicted_sub_obj:
            predictions.add_prediction(predicted[0], self.target, predicted[1], self)
        return True 

class RuleZ(Rule):
    """
    A z-rule or zero rule is a very special unary rule, that has no body. This means that the rule fires always if you check whether it fires.
    It is assumed that these rules are used only in a context where it is clear that the entity of the the completion task is an entity that
    is related to something else via the target relation of the z-rule that is applied in this context.
    """
    def __init__(self, target, hc, hc_right):
        super().__init__()
        self.target = target
        self.hc = hc
        self.hc_right = hc_right # if true, the rule is an X rule, otherwise it is an Y rule

    def __str__(self):
        rep = ""
        (head_left, head_right)  = ("X",id2to[self.hc]) if self.hc_right else (id2to[self.hc],"Y")
        rep += id2to[self.target] + "(" + head_left + "," + head_right + ") <="
        return rep

    def __eq__(self, other):
        if type(self) != type(other): return False
        if self.target != other.target: return False
        if self.hc != other.hc: return False
        if self.hc_right != other.hc_right: return False
        return True
    
    def __hash__(self):
        if (self.hashcode == None):
            self.hashcode = self.hc
            self.hashcode += self.target if self.hc_right else -self.target
        return self.hashcode
    
    def materialize(self, triples, predictions):
        # todo, implement if required
        pass


class RuleUd(Rule):
    """
    A unary rule with a constant in head and a dangling (free) variabale in the body. The body consists of exactly one atom.
    A rule of this type has the form h(X,a) <= b1(X,A).
    """
    def __init__(self, target, rel, hc, hc_right, dangling_right):
        super().__init__()
        self.target = target
        self.rel = rel
        self.hc = hc
        self.hc_right = hc_right # if true, the rule is an X rule, otherwise it is an Y rule
        self.dangling_right = dangling_right

    def __str__(self):
        rep = ""
        (head_left, head_right)  = ("X",id2to[self.hc]) if self.hc_right else (id2to[self.hc],"Y")
        v = "X" if self.hc_right else "Y"
        body_left, body_right  = (v,"A") if self.dangling_right else ("A",v)
        rep += id2to[self.target] + "(" + head_left + "," + head_right + ") <= " + id2to[self.rel] + "(" + body_left + "," + body_right + ")"
        return rep
    
    def __eq__(self, other):
        if type(self) != type(other): return False
        if self.target != other.target: return False
        if self.rel != other.rel: return False
        if self.hc != other.hc: return False
        if self.hc_right != other.hc_right: return False
        if self.dangling_right != other.dangling_right: return False
        return True
    
    def __hash__(self):
        if (self.hashcode == None):
            self.hashcode = self.hc
            self.hashcode += self.target if self.hc_right else -self.target
            self.hashcode += self.rel if self.dangling_right else -self.rel
        return self.hashcode
    
    def materialize(self, triples, predictions):
        # todo, implement if required
        pass

class RuleUc(Rule):
    """
    A unary rule with a constant in head and body. The body consists of exactly one atom.
    A rule of this type has the form h(X,a) <= b1(X,b).
    """    

    def __init__(self, target, rel, hc, hc_right, bc, bc_right):
        super().__init__()
        self.target = target
        self.rel = rel
        self.hc = hc
        self.hc_right = hc_right # if true, the rule is an X rule, otherwise it is an Y rule
        self.bc = bc
        self.bc_right = bc_right

    def __str__(self):
        rep = ""
        (head_left, head_right)  = ("X",id2to[self.hc]) if self.hc_right else (id2to[self.hc],"Y")
        v = "X" if self.hc_right else "Y"
        body_left, body_right  = (v,id2to[self.bc]) if self.bc_right else (id2to[self.bc],v)
        rep += id2to[self.target] + "(" + head_left + "," + head_right + ") <= " + id2to[self.rel] + "(" + body_left + "," + body_right + ")"
        return rep

    def __eq__(self, other):
        if type(self) != type(other): return False
        if self.target != other.target: return False
        if self.rel != other.rel: return False
        if self.hc != other.hc: return False
        if self.bc != other.bc: return False
        if self.hc_right != other.hc_right: return False
        if self.bc_right != other.bc_right: return False
        return True

    def __hash__(self):
        if (self.hashcode == None):
            self.hashcode = self.hc + self.bc
            self.hashcode += self.target if self.hc_right else -self.target
            self.hashcode += self.rel if self.bc_right else -self.rel
        return self.hashcode

    def materialize(self, triples, predictions):
        if (self.target == self.rel and self.hc == self.bc and self.hc_right == self.bc_right):
            return False
        x_r_2_y = triples.obj_rel_2_sub if self.bc_right else triples.sub_rel_2_obj
        predicted_sub_obj = []
        for value in x_r_2_y[self.bc][self.rel]:
            self.pred += 1
            (x,y) = (value,self.hc) if self.hc_right else (self.hc,value)
            predicted_sub_obj.append((x,y))
            if triples.is_known(x, self.target, y):
                self.cpred += 1
        if self.get_confidence() < config.rules['uc']['confidence']: return False
        if self.cpred < config.rules['uc']['support']: return False
        for predicted in predicted_sub_obj:
            predictions.add_prediction(predicted[0], self.target, predicted[1], self)
        return True

class RuleB(Rule):
    """
    A binary rule of the form h(X,Y) <= b1(X,A), b2(A,B), b3(B,Y). This rule can have between one and five body atoms.
    """

    var_symbols = (("X","Y"), ("X","A","Y"), ("X","A","B","Y"), ("X","A","B","C","Y"), ("X","A","B","C","D","Y"))

    branches_at_zero     = config.rules["b"]["branches_per_level"][0][0]
    branches_at_zero_rev = config.rules["b"]["branches_per_level"][1][0]
    branches_per_level     = config.rules["b"]["branches_per_level"][0][1]
    branches_per_level_rev = config.rules["b"]["branches_per_level"][1][1]


    def __init__(self, rels, dirs):
        super().__init__()
        if isinstance(rels[0], int):
            self.target = rels[0]
            self.rels = tuple(rels[1:])
            self.dirs = tuple(dirs)
        else:
            self.target = to2id[rels[0]]
            self.rels = tuple(map(lambda x : to2id[x], rels[1:]))
            self.dirs = tuple(dirs)

    
    def __str__(self):
        return self.get_rep(id2to)
    
    def get_rep(self, id2to):
        rep = ""
        rep += id2to[self.target] + "(X,Y) <="
        my_vars = RuleB.var_symbols[len(self.rels) - 1]
        for i in range(len(self.rels)):
            (v1, v2) = (my_vars[i], my_vars[i+1]) if self.dirs[i] else (my_vars[i+1], my_vars[i])
            rep += " " + id2to[self.rels[i]] + "(" + v1 + "," + v2 + ")"
            if i < len(self.rels)-1: rep += ","
        return rep
        
    def __eq__(self, other):
        if type(self) != type(other): return False        
        if self.target != other.target: return False
        if len(self.rels) != len(other.rels): return False
        for i in range(len(self.rels)):
            if self.rels[i] != other.rels[i] or self.dirs[i] != other.dirs[i]: return False
        return True

    def __hash__(self):
        if (self.hashcode == None):
            self.hashcode = self.target
            for i in range(len(self.rels)):
                self.hashcode += self.rels[i] if self.dirs[i] else -self.rels[i]
        return self.hashcode    

    def score(self, pid, triples, lock, prediction_data, data_index, c_clause_handler, id2to):
        scores = c_clause_handler.calcStats(self.get_rep(id2to))
        self.cpred = int(scores[1])
        self.pred = int(scores[0])
        if self.pred == 0: return data_index, False
        # confidence threshold
        if (self.cpred / self.pred) < config.rules['b']['confidence']: return data_index, False
        # support threshold
        if self.cpred < config.rules['b']['support']: return data_index, False
        # the rule is accepted, its predictions will now be stored
        prediction_data[data_index] = -self.id
        prediction_data[data_index + 1] = self.pred
        prediction_data[data_index + 2] = self.cpred
        data_index = self.maybe_transfer_prediction_data(prediction_data, data_index, lock)
        return data_index, True



    def scoreOLD(self, pid, triples, lock, prediction_data, data_index):
        start = time.time()
        count = 0
        # from x to y
        # SPECIFIC
        anchors = self.normalize_anchor_list(triples.r2_sub[self.target], RuleB.branches_at_zero)
        # STANDARD anchors = self.normalize_anchor_list(triples.r2_sub[self.rels[0]] if self.dirs[0] else triples.r2_obj[self.rels[0]], RuleB.branches_at_zero)
        blocked_anchors = set()
        pred_global = 0.0
        cpred_global = 0.0
        for x in anchors:
            # if x in blocked_anchors: continue
        #for x in (triples.r2_sub[self.rels[0]] if self.dirs[0] else triples.r2_obj[self.rels[0]]):
            if count > RuleB.branches_at_zero: break
            values = set()
            values.add(x)
            y_values = set()
            self.search_grounding(triples, 0, x, values, y_values)
            pred_local = 0
            cpred_local = 0 
            for y in y_values:
                if (x,y) in blocked_anchors: continue
                blocked_anchors.add((x,y))
                # self.pred += 1
                pred_local += 1
                if triples.is_known(x, self.target, y):
                    # self.cpred += 1
                    cpred_local += 1
            if len(y_values) > 0 and pred_local > 0:
                pred_global += 1
                cpred_global += cpred_local / pred_local
            current = time.time()

            count += 1
            if (current - start > config.rules["b"]["timeout"]): return data_index, False
            # if self.cpred < config.rules['b']['pruning'][0] and self.pred > config.rules['b']['pruning'][1]: return data_index, False
        # from y to x
        count_rev = 0
        # SPECIFIC
        anchors = self.normalize_anchor_list(triples.r2_obj[self.target], RuleB.branches_at_zero_rev)
        # STANDARD anchors = self.normalize_anchor_list(triples.r2_obj[self.rels[-1]] if self.dirs[-1] else triples.r2_sub[self.rels[0]], RuleB.branches_at_zero_rev)
        # blocked_anchors = set() 
        for y in anchors:
            # if y in blocked_anchors: continue
        #for y in (triples.r2_obj[self.rels[-1]] if self.dirs[-1] else triples.r2_sub[self.rels[0]]):
            if count_rev > RuleB.branches_at_zero_rev: break
            values = set()
            values.add(y)
            x_values = set()
            self.search_grounding_rev(triples, len(self.rels)-1, y, values, x_values) 
            pred_local = 0
            cpred_local = 0 
            for x in x_values:
                if (x,y) in blocked_anchors: continue
                blocked_anchors.add((x,y))
                # self.pred += 1
                pred_local += 1
                if triples.is_known(x, self.target, y):
                    # self.cpred += 1
                    cpred_local += 1
            if len(x_values) > 0 and pred_local > 0:
                pred_global += 1
                cpred_global += cpred_local / pred_local
            current = time.time()
            count_rev += 1
            if (current - start > config.rules["b"]["timeout"]): break
            # if self.cpred < config.rules['b']['pruning'][0] and self.pred > config.rules['b']['pruning'][1]: return data_index, False
        # at least one prediction should be made
        self.cpred = int(cpred_global)
        self.pred = int(pred_global)
        if self.pred == 0: return data_index, False
        # confidence threshold
        if (self.cpred / self.pred) < config.rules['b']['confidence']: return data_index, False
        # support threshold
        if self.cpred < config.rules['b']['support']: return data_index, False
        # the rule is accepted, its predictions will now be stored
        prediction_data[data_index] = -self.id
        prediction_data[data_index + 1] = self.pred
        prediction_data[data_index + 2] = self.cpred
        data_index = self.maybe_transfer_prediction_data(prediction_data, data_index, lock)
        return data_index, True

    def normalize_anchor_list(self, anchors, expected_length):
        anchors_normalized = []
        while len(anchors_normalized) < expected_length:
             anchors_normalized.extend(anchors)
        anchors_normalized = anchors_normalized[:expected_length]
        return anchors_normalized

    def maybe_transfer_prediction_data(self, prediction_data, data_index, lock):
        data_index += 3
        if data_index == len(prediction_data):
            while not(prediction_data[0] == 0):
                lock.release()
                time.sleep(0.1)
                lock.acquire()
            data_index = 0
        return data_index

    def trivial(self):
        if len(self.rels) == 1 and self.target == self.rels[0] and self.dirs[0]:
            return True
        return False

    def search_grounding(self, triples, index, value, values, y_values):
        next_values = []
        rel = self.rels[index]
        if self.dirs[index]:
            if value in triples.sub_rel_2_obj and rel in triples.sub_rel_2_obj[value]:
                # next_values = triples.sub_rel_2_obj[value][rel]
                next_values = self.sample_if_possible(triples.sub_rel_2_obj[value][rel], RuleB.branches_per_level[index])
        else:
            if value in triples.obj_rel_2_sub and rel in triples.obj_rel_2_sub[value]:
                # next_values = triples.obj_rel_2_sub[value][rel]
                next_values = self.sample_if_possible(triples.obj_rel_2_sub[value][rel], RuleB.branches_per_level[index])
        if index+1 == len(self.rels): # stop here an collect y values    
            for nv in next_values:
                if nv not in values:
                    y_values.add(nv)
                    # return
        else: # lets go deeper ... 
            #if len(next_values) <= RuleB.branches_per_level[index]:
            for nv in next_values:
                if nv not in values:
                    self.search_grounding(triples, index+1, nv, (*values, nv), y_values)
                    # if len(y_values) > 0: return
            #else:
            #    count = 0
            #    for nv in next_values:
            #        if nv not in values:
            #            if count >= RuleB.branches_per_level[index]: break
            #            self.search_grounding(triples, index+1, nv, (*values, nv), y_values)
            #            if len(y_values) > 0: return
            #            count += 1

    def get_all_groundings(self, triples):
            x_values = triples.r2_sub[self.rels[0]] if self.dirs[0] else triples.r2_obj[self.rels[0]]
            groundings = []
            for x in x_values:
                values = set()
                values.add(x)
                y_values = set()
                self.search_grounding_all(triples, 0, x, values, y_values)
                for y in y_values:
                    groundings.append((x,y))
            return groundings


    def search_grounding_all(self, triples, index, value, values, y_values):
        """Searches for all groundings for all groundings (no sampling) in forward direction."""
        next_values = []
        rel = self.rels[index]
        if self.dirs[index]:
            if value in triples.sub_rel_2_obj and rel in triples.sub_rel_2_obj[value]:
                next_values = triples.sub_rel_2_obj[value][rel]
        else:
            if value in triples.obj_rel_2_sub and rel in triples.obj_rel_2_sub[value]:
                next_values = triples.obj_rel_2_sub[value][rel]
        if index+1 == len(self.rels):  
            for nv in next_values:
                if nv not in values:
                    y_values.add(nv)
        else:
            for nv in next_values:
                if nv not in values:
                    self.search_grounding_all(triples, index+1, nv, (*values, nv), y_values)


    def search_grounding_rev(self, triples, index, value, values, x_values):
        next_values = []
        rel = self.rels[index]
        if not(self.dirs[index]):
            if value in triples.sub_rel_2_obj and rel in triples.sub_rel_2_obj[value]:
                # next_values = triples.sub_rel_2_obj[value][rel]
                next_values = self.sample_if_possible(triples.sub_rel_2_obj[value][rel],RuleB.branches_per_level_rev[index])
        else:
            if value in triples.obj_rel_2_sub and rel in triples.obj_rel_2_sub[value]:
                # next_values = triples.obj_rel_2_sub[value][rel]
                next_values = self.sample_if_possible(triples.obj_rel_2_sub[value][rel],RuleB.branches_per_level_rev[index])
        if index == 0: # stop here an collect y values    
            for nv in next_values:
                if nv not in values:
                    x_values.add(nv)
                    # return
        else: # lets go deeper ... 
            #if len(next_values) <= RuleB.branches_per_level[index]:
            for nv in next_values:
                if nv not in values:
                    self.search_grounding_rev(triples, index-1, nv, (*values, nv), x_values)
                    # if len(x_values) > 0: return
            #else:
            #    count = 0
            #    for nv in next_values:
            #        if nv not in values:
            #            if count >= RuleB.branches_per_level[index]: break
            #            self.search_grounding_rev(triples, index-1, nv, (*values, nv), x_values)
            #            if len(x_values) > 0: return
            #            count += 1

    def sample_if_possible(self, sequence, num):
        if len(sequence) > num:
            return sample(sequence, num)
        else:
            return sequence
    
class RuleSet:

    rules = []
    id2rules = {}

    def __init__(self):
        self.rules = []

    def add_rule(self, rule):
        self.id2rules[rule.id] = rule
        self.rules.append(rule)

    def add_rules(self, myrules):
        for myrule in myrules:
            self.add_rule(myrule)

    def get_rule_by_id(self, id):
        return self.id2rules[id]

    def size(self):
        return len(self.rules)

    def transfer_prediction_data(self, pid, prediction_data, candidates):
        for index,rindex in enumerate(prediction_data[::3]):
            if rindex == 0: break
            i0 = index*3
            i1,i2 = i0+1, i0+2
            if rindex < 0:
                rule = candidates.get_rule_by_id(-rindex)
                rule.pred = prediction_data[i1]
                rule.cpred = prediction_data[i2]
                rule.aconfidence = rule.cpred / (rule.pred + config.hyperparams["uce"])
                self.rules.append(rule)
            else:
                rule = candidates.get_rule_by_id(rindex)
                rel = rule.target
                sub = prediction_data[i1]
                obj = prediction_data[i2]
                self.add_prediction(sub, rel, obj, rule)
        prediction_data[0] = 0   

    def write(self, path, output_format="AnyBURL"):
        f = open(path, "w")
        if output_format == "AnyBURL":
            for rule in self.rules:
                confidence = '{:.5f}'.format(rule.cpred / rule.pred)
                if type(rule) is RuleXXud or type(rule) is RuleXXuc:
                    ruleMeMyselfI = str(rule).replace("(X,X)", "(X,me_myself_i)")
                    f.write(str(rule.pred) + "\t" + str(rule.cpred) + "\t" + str(confidence) + "\t" +  ruleMeMyselfI + "\n")
                else:
                    f.write(str(rule.pred) + "\t" + str(rule.cpred) + "\t" + str(confidence) + "\t" + str(rule) + "\n")
            # print(">>> rules written to " +  path + " using AnyBURL format") 
            
        if not(output_format == "AnyBURL"):
            print(">>> could not write rules to disc, outputformat not available")
        f.close()


class RuleReader:

    def __init__(self):
        pass
   
    def read_file(self, filepath):
        file = open(filepath, 'r')
        lines = file.readlines()
        rules = []
        count = 0
        for line in lines:
            count += 1
            rule = self.read_line(line.strip())
            if not(rule == None):
                rules.append(rule)
        file.close()
        return rules

            

    def read_line(self, line):
        token = line.split("\t")
        if len(token) != 4: return None
        pred = int(token[0])
        cpred = int(token[1])
        conf = float(token[2])
        hb = token[3].split(" <= ")
        if len(hb) != 2: return None
        if "(X,Y)" in hb[0]:
            rels = []
            rels.append(hb[0].split("(")[0])
            atoms = hb[1].split(", ")
            dirs = []
            vars = RuleB.var_symbols[len(atoms)-1]
            counter = 0
            for atom in atoms:
                atoken = atom.split("(")
                rels.append(atoken[0])
                dirs.append(atoken[1].startswith(vars[counter]))
                counter += 1
            rule = RuleB(rels, dirs)
            rule.cpred = cpred
            rule.pred = pred
            return rule
        else:
            return None

