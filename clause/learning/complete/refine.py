from torm import Torm
from triples import Triple,TripleSet,PredictionSet,CompletionSet,id2to
import triples as ttt

from rules import RuleB, RuleReader,RuleSet, RuleUc, RuleZ
import math
import random


import c_clause

IG_MIN = 0.01

# only a sample of all predictions is used for computing the quality of the refinement
SAMPLE_SIZE = 2000

MIN_COVERAGE = 10

def mylog(num, denom):
    if num == 0 or denom == 0: return 0
    else: return math.log(num/denom, 2)  

def entropy(p, n):
    # print(str(p) + " | " + str(n))
    if p == 0 and n == 0: return 0
    e = p + n
    return -p/e * mylog(p, e) - n/e * mylog(n, e)

def ig(pl, nl, pr, nr):
    p = pl + pr
    n = nl + nr
    e = p + n
    el = pl + nl
    er = pr + nr
    return entropy(p,n) - el/e * entropy(pl,nl) - er/e * entropy(pr,nr)

def check_if_z_object_exists(triples,x,y,r):
    if x in triples.sub_rel_2_obj:
        if r in triples.sub_rel_2_obj[x]:
            if y in triples.sub_rel_2_obj[x][r]:
                if len(triples.sub_rel_2_obj[x][r]) > 1: return True
                else: return False
            else: return True
        else: return False
    return False

def check_if_z_subject_exists(triples,x,y,r):
    if x in triples.obj_rel_2_sub:
        if r in triples.obj_rel_2_sub[x]:
            if y in triples.obj_rel_2_sub[x][r]:
                if len(triples.obj_rel_2_sub[x][r]) > 1: return True
                else: return False
            else: return True
        else: return False
    return False

def get_best_refinement(triples, rule, groundings):
    pos,neg = [],[]
    random.shuffle(groundings)
    del groundings[SAMPLE_SIZE:]
    for (x,y) in groundings:
        if triples.is_known(x,rule.target,y): pos.append((x,y))
        else: neg.append((x,y))
    best_gain = IG_MIN
    # if nop split is chosen the pos / neg ratio is put into the right branch for updating the original sampled rule
    (best_pl, best_nl, best_pr, best_nr) = (0,0,len(pos),len(neg)) 
    best_split = (None, None, None)
    for relation in triples.r2r_ss[rule.target]:
        # case 1: (X,Z)
        (pl, nl, pr, nr) = (0,0,0,0) 
        for (x,y) in pos:
            if check_if_z_object_exists(triples, x, y, relation): pl += 1
            else: pr +=1
        for (x,y) in neg:
            if check_if_z_object_exists(triples, x, y, relation): nl += 1
            else: nr +=1
        gain = ig(pl, nl, pr, nr)
        if gain > best_gain:
            best_gain = gain
            (best_pl, best_nl, best_pr, best_nr) = (pl, nl, pr, nr)
            best_split = ('X', True, relation)
    for relation in triples.r2r_so[rule.target]:
        # case 2 (Z,X)
        (pl, nl, pr, nr) = (0,0,0,0) 
        for (x,y) in pos:
            if check_if_z_subject_exists(triples, x, y, relation): pl += 1
            else: pr +=1
        for (x,y) in neg:
            if check_if_z_subject_exists(triples, x, y, relation): nl += 1
            else: nr +=1
        gain = ig(pl, nl, pr, nr)
        if gain > best_gain:
            best_gain = gain
            (best_pl, best_nl, best_pr, best_nr) = (pl, nl, pr, nr)
            best_split = ('X', False, relation)
    #if best_gain > IG_MIN:
        # relation = best_split[2]
        # var_order = "(X,Z)" if best_split[1] else "(Z,X)"
        #print(rule)
        #print(ttt.id2to[relation] + var_order + " " + str(ig(best_pl, best_nl, best_pr, best_nr)))
        #print(str(best_pr+best_pl) + "|" + str(best_nr+best_nl) + " => L:" + str(best_pl) + "|" + str(best_nl) + " R:" + str(best_pr) + "|" + str(best_nr))
        #print("---")
    return (best_split, (best_pl, best_nl, best_pr, best_nr))
    # else:
        # return None

def get_refined_rule_serialization(pos_examples, neg_examples, rule, split, positive_atom):
    var, dir, relation = split
    var_order = "(" + var + ",Z)" if dir else "(Z," + var + ")"
    atom = ttt.id2to[relation] + var_order
    if not positive_atom: atom = "!" + atom 
    preds = pos_examples + neg_examples
    cpreds = pos_examples
    conf = cpreds / preds
    # return str(preds) + "\t" + str(cpreds) + "\t" + str(conf) + "\t" + str(rule) + "\t" + ratom
    return str(preds) + "\t" + str(cpreds)+ "\t" + str(conf) + "\t" + str(rule) + ", " + atom


if __name__ == '__main__':
    
    train_path = "E:/code/eclipse-workspace/AnyBURL/data/wn18rr/train.txt"
    # rule_input_path  = "E:/exp/pyclause/rules/rules-anyburl-wn18rr-Uc"
    rule_input_path  = "E:/exp/pyclause/rules/rules-pctorm-wn18rr-b3"

    # rule_output_path = "E:/exp/pyclause/rules/rules-anyburl-wn18rr-Uc-refined-ig0.01"
    rule_output_path  = "E:/exp/pyclause/rules/rules-pctorm-wn18rr-b3-ig0.01-RO"

    triples = TripleSet(train_path)
    
    rr = RuleReader()
    rules = rr.read_file(rule_input_path)
    print("read " + str(len(rules)) + " rules from file")
    

    handler = c_clause.RuleHandler(train_path)

    done = False
    out = open(rule_output_path, "w")

    brules = []
    for rule in rules:
        if isinstance(rule, RuleB): 
            brules.append(rule)
        else:
            print("ups")
    num_of_brules = len(brules)


    rfcounter = 0
    while done == False:
        rules_selected = []
        for i in range(1000):
            if len(brules) == 0:
                done = True
                break
            rules_selected.append(brules.pop(0))

        result = handler.calcRulesPredictions(list(map(str, rules_selected)), True, True)
        predictions_list = result[0]

        index = 0
        for predictions in predictions_list:
            rule = rules_selected[index]
            index += 1
            # print(str(rule))
            groundings = list(map(lambda g : (ttt.to2id[g[0]],ttt.to2id[g[1]]), predictions))
            #if len(groundings) > MIN_COVERAGE:
            refinement = get_best_refinement(triples, rule, groundings)
            #else:
                # refinement = None
            if refinement[0][0] == None:
                rule.cpred = refinement[1][2]
                rule.pred = refinement[1][2] + refinement[1][3]
                out.write(rule.get_serialization() + "\n")
            else:
                rfcounter += 1
                split = refinement[0]
                best_pl, best_nl, best_pr, best_nr = refinement[1]
                out.write(get_refined_rule_serialization(best_pl, best_nl, rule, split, True) + "\n")
                out.write(get_refined_rule_serialization(best_pr, best_nr, rule, split, False) + "\n")
        print(">>> brules that need to be refined " + str(len(brules)))
    print(">>> rules checked: "  + str(num_of_brules))
    print(">>> rules refined: "  + str(rfcounter))
    out.close()


