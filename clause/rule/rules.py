from clause.config import options

import time
from random import sample

class Rule:

    var_symbols = (
                    ("X","Y"), ("X","A","Y"), ("X","A","B","Y"), ("X","A","B","C","Y"), ("X","A","B","C","D","Y"),
                    ("X","A","B","C","D","E","Y"), ("X","A","B","C","D","E","F","Y")
    )

    id_counter = 0

    def __init__(self, ruleset):
        Rule.id_counter += 1
        self.id = Rule.id_counter
        self.ruleset = ruleset
        self.pred = 0
        self.cpred = 0
        self.hashcode = None
        self.aconfidence = 0.0 # this is the counterpart to the applied confidence

    def store(self):
        self.ruleset.add_rule(self)
        
    def get_confidence(self):
        return self.cpred / self.pred
    
    def get_serialization(self):
        return str(self.pred) + "\t" + str(self.cpred) + "\t" + str(self.get_confidence()) + "\t" + str(self)

class RuleXXuc(Rule):
    """
    A rather specific rule of the form h(X,X) <= b(X,b). Note that this rule was represented in AnyBURL via a hack as a Uc
    rule. Here we implemented it as a rule on its own. However, this rule type still supports the output required to run as AnyBURL
    hack.
    """  
    def __init__(self, ruleset, target, rel, bc, bc_right):
        super().__init__(ruleset)
        if isinstance(rel, int):
            self.target = target
            self.rel = rel
            self.bc = bc
        else:
            self.target = self.ruleset.index.to2id[target]
            self.rel = self.ruleset.index.to2id[rel]
            self.bc = self.ruleset.index.to2id[bc]
        self.bc_right = bc_right

    def __str__(self):
        ii = self.ruleset.index
        rep = ""
        body_left, body_right  = ("X",ii.id2to[self.bc]) if self.bc_right else (ii.id2to[self.bc],"X")
        rep += ii.id2to[self.target] + "(X,X) <= " + ii.id2to[self.rel] + "(" + body_left + "," + body_right + ")"
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

    
class RuleXXud(Rule):
    """
    A rather specific rule of the form h(X,X) <= b(X,A). Note that this rule was represented in AnyBURL via a hack as a Uc
    rule. Here we implemented it as a rule on its own. However, this rule type still supports the output required to run as AnyBURL
    hack.
    """  

    def __init__(self, ruleset, target, rel, dangling_right):
        super().__init__(ruleset)
        self.target = target
        self.rel = rel
        if isinstance(rel, int):
            self.target = target
            self.rel = rel
        else:
            self.target = self.ruleset.index.to2id[target]
            self.rel = self.ruleset.index.to2id[rel]
        self.dangling_right = dangling_right

    def __str__(self):
        ii = self.ruleset.index
        rep = ""
        body_left, body_right  = ("X","A") if self.dangling_right else ("A","X")
        rep += ii.id2to[self.target] + "(X,X) <= " + ii.id2to[self.rel] + "(" + body_left + "," + body_right + ")"
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


class RuleZ(Rule):
    """
    A z-rule or zero rule is a very special unary rule, that has no body. This means that the rule fires always if you check whether it fires.
    It is assumed that these rules are used only in a context where it is clear that the entity of the the completion task is an entity that
    is related to something else via the target relation of the z-rule that is applied in this context.
    """
    def __init__(self, ruleset, target, hc, hc_right):
        super().__init__(ruleset)
        if isinstance(target, int):
            self.target = target
            self.hc = hc
        else:
            self.target = self.ruleset.index.to2id[target]
            self.hc = self.ruleset.index.to2id[hc]
        self.hc_right = hc_right # if true, the rule is an X rule, otherwise it is an Y rule

    def vectorized(self):
        return ("Z", self.target, self.hc_right, self.hc)

    def __str__(self):
        ii = self.ruleset.index
        rep = ""
        (head_left, head_right)  = ("X", ii.id2to[self.hc]) if self.hc_right else (ii.id2to[self.hc],"Y")
        rep += ii.id2to[self.target] + "(" + head_left + "," + head_right + ") <="
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
    

class RuleUd(Rule):
    """
    A unary rule with a constant in head and a dangling (free) variabale in the body. The body consists of exactly one atom.
    A rule of this type has the form h(X,a) <= b1(X,A).
    """
    def __init__(self, ruleset, target, rels, dirs, hc, hc_right):
        super().__init__(ruleset)
        if isinstance(rels[0], int):
            self.target = target
            self.rels = rels
            self.hc = hc
        else:
            self.target = self.ruleset.index.to2id[target]
            self.rels = tuple(map(lambda x : self.ruleset.index.to2id[x], rels))
            self.hc = self.ruleset.index.to2id[hc]
        self.dirs = dirs
        self.hc_right = hc_right # if true, the rule is an X rule, otherwise it is an Y rule
        self.bc_right = dirs[-1]

    def vectorized(self):
        if self.hc_right:
            return ("Ud", self.target, (self.rels, self.dirs, self.hc_right, self.hc))
        else:
            return ("Ud", self.target, (self.rels[::-1], tuple(map(lambda x : not x, self.dirs))[::-1], self.hc_right, self.hc))

    def __str__(self):
        ii = self.ruleset.index
        rep = ""
        (head_left, head_right)  = ("X",ii.id2to[self.hc]) if self.hc_right else (ii.id2to[self.hc],"Y")
        rep += ii.id2to[self.target] + "(" + head_left + "," + head_right + ") <="
        my_vars = Rule.var_symbols[len(self.rels)] # on purpose we are chosing a size that is on ehigher
        for i in range(len(self.rels)):
            (v1, v2) = (my_vars[i], my_vars[i+1]) if self.dirs[i] else (my_vars[i+1], my_vars[i])
            if i == 0 and not self.hc_right:
                if v1 == "X": v1 = "Y"
                else: v2 = "Y"
            rep += " " + ii.id2to[self.rels[i]] + "(" + v1 + "," + v2 + ")"
            if i < len(self.rels)-1: rep += ","
        return rep

    def __eq__(self, other):
        if type(self) != type(other): return False
        if self.target != other.target: return False
        if self.rels != other.rels: return False
        if self.dirs != other.dirs: return False
        if self.hc != other.hc: return False
        if self.hc_right != other.hc_right: return False
        return True
    
    def __hash__(self):
        if (self.hashcode == None):
            self.hashcode = self.hc
            self.hashcode += self.target if self.hc_right else -self.target
            i = 0
            for dir in self.dirs:
                rel = self.rels[i]
                self.hashcode += rel if dir else -rel
                i += 1
        return self.hashcode
    

class RuleUc(Rule): 
    """
    A unary rule with a constant in head and body. The body consists of exactly one atom.
    A rule of this type has the form h(X,a) <= b1(X,b).
    """    

    def __init__(self, ruleset, target, rels, dirs, hc, hc_right, bc):
        super().__init__(ruleset)
        if isinstance(rels[0], int):
            self.target = target
            self.rels = rels
            self.hc = hc
            self.bc = bc
        else:
            self.target = self.ruleset.index.to2id[target]
            self.rels = tuple(map(lambda x : self.ruleset.index.to2id[x], rels))
            self.hc = self.ruleset.index.to2id[hc]
            self.bc = self.ruleset.index.to2id[bc]
        self.dirs = dirs
        self.hc_right = hc_right # if true, the rule is an X rule, otherwise it is an Y rule
        self.bc_right = dirs[-1] 

    def vectorized(self):
        if self.hc_right:
            return ("Uc", self.target, (self.rels, self.dirs, self.hc_right, self.hc, self.bc))
        else:
            return ("Uc", self.target, (self.rels[::-1], tuple(map(lambda x : not x, self.dirs))[::-1], self.hc_right, self.hc, self.bc))
    
    def __str__(self):
        ii = self.ruleset.index
        rep = ""
        (head_left, head_right)  = ("X",ii.id2to[self.hc]) if self.hc_right else (ii.id2to[self.hc],"Y")
        rep += ii.id2to[self.target] + "(" + head_left + "," + head_right + ") <="
        my_vars = Rule.var_symbols[len(self.rels) - 1]
        for i in range(len(self.rels)):
            (v1, v2) = (my_vars[i], my_vars[i+1]) if self.dirs[i] else (my_vars[i+1], my_vars[i])
            if i == 0 and not self.hc_right:
                if v1 == "X": v1 = "Y"
                else: v2 = "Y"
            if i == len(self.rels) - 1:
                if self.bc_right: v2 = ii.id2to[self.bc]
                else: v1 = ii.id2to[self.bc]
            rep += " " + ii.id2to[self.rels[i]] + "(" + v1 + "," + v2 + ")"
            if i < len(self.rels)-1: rep += ","
        return rep

    def __eq__(self, other):
        if type(self) != type(other): return False
        if self.target != other.target: return False
        if self.rels != other.rels: return False
        if self.dirs != other.dirs: return False
        if self.hc != other.hc: return False
        if self.bc != other.bc: return False
        if self.hc_right != other.hc_right: return False
        return True

    def __hash__(self):
        if (self.hashcode == None):
            self.hashcode = self.hc
            self.hashcode += self.target if self.hc_right else -self.target
            i = 0
            for dir in self.dirs:
                rel = self.rels[i]
                self.hashcode += rel if dir else -rel
                i += 1
            self.hashcode += self.bc
        return self.hashcode


class RuleB(Rule):
    """
    A binary rule of the form h(X,Y) <= b1(X,A), b2(A,B), b3(B,Y). This rule can have between one and seven body atoms.
    """


    def __init__(self, ruleset, target, rels, dirs):
        super().__init__(ruleset)
        if isinstance(target, int):
            self.target = target
            self.rels = tuple(rels)
            self.dirs = tuple(dirs)
        else:
            self.target = self.ruleset.index.to2id[target]
            self.rels = tuple(map(lambda x : self.ruleset.index.to2id[x], rels))
            self.dirs = tuple(dirs)

    def __str__(self):
        return self.get_rep(self.ruleset.index.id2to)
    
    def vectorized(self):
        return ("B", (self.target,) + self.rels, self.dirs)

    def get_rep(self, id2to):
        rep = ""
        rep += id2to[self.target] + "(X,Y) <="
        my_vars = Rule.var_symbols[len(self.rels) - 1]
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
    #id2rules = {}

    def __init__(self, index):
        self.index = index
        self.rules = []

    def add_rule(self, rule):
        #self.id2rules[rule.id] = rule
        self.rules.append(rule)

    def add_ruleset(self, other):
        for orule in other.rules:
           self.add_rule(orule) 

    def add_rules(self, myrules):
        for myrule in myrules:
            self.add_rule(myrule)

    def size(self):
        return len(self.rules)
    
    def retainOnly(self, *args):
        retained_rules = []
        for rule in self.rules:
            retain = False
            for t in args:
                if t == "B" and isinstance(rule, RuleB): retain = True
                if t == "Uc" and isinstance(rule, RuleUc): retain = True
                if t == "Ud" and isinstance(rule, RuleUd): retain = True
                if t == "Z" and isinstance(rule, RuleZ): retain = True
                if t == "XXuc" and isinstance(rule, RuleXXuc): retain = True
                if t == "XXud" and isinstance(rule, RuleXXud): retain = True
            if retain:
                retained_rules.append(rule)
        print(">>> reduced size of ruleset from " + str(len(self.rules)) + " to " + str(len(retained_rules)) + " rules")
        self.rules = retained_rules

    def write(self, path, output_format="PyClause"):
        f = open(path, "w")
        if output_format == "AnyBURL":
            for rule in self.rules:
                confidence = '{:.5f}'.format(rule.cpred / rule.pred)
                if type(rule) is RuleXXud or type(rule) is RuleXXuc:
                    ruleMeMyselfI = str(rule).replace("(X,X)", "(X,me_myself_i)")
                    f.write(str(rule.pred) + "\t" + str(rule.cpred) + "\t" + str(confidence) + "\t" +  ruleMeMyselfI + "\n")
                    ruleMeMyselfI2 = str(rule).replace("(X,X)", "(me_myself_i,Y)")
                    ruleMeMyselfI2 = ruleMeMyselfI2.replace("(X,", "(Y,")
                    ruleMeMyselfI2 = ruleMeMyselfI2.replace(",X)", ",Y)")
                    f.write(str(rule.pred) + "\t" + str(rule.cpred) + "\t" + str(confidence) + "\t" +  ruleMeMyselfI2 + "\n")
                else:
                    f.write(str(rule.pred) + "\t" + str(rule.cpred) + "\t" + str(confidence) + "\t" + str(rule) + "\n")
            print(">>> " + str(len(self.rules)) + " rules written to " +  path + " using AnyBURL format")
            f.close()
            return
            
        if output_format == "PyClause":
            for rule in self.rules:
                confidence = '{:.5f}'.format(rule.cpred / rule.pred)
                f.write(str(rule.pred) + "\t" + str(rule.cpred) + "\t" + str(confidence) + "\t" + str(rule) + "\n")
            print(">>> " + str(len(self.rules)) + " rules written to " +  path + " using PyClause format") 
            f.close()
            return
        
        print(">>> could not write rules to disc, outputformat not available")
        f.close()
