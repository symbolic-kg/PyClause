from torm import Torm
from triples import Triple,TripleSet,PredictionSet,CompletionSet,id2to
import triples as ttt

from rules import RuleReader,RuleSet
import config
import math

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

def get_best_refinement(triples, rule, igmin):
    g1 = rule.get_all_groundings(triples)
    pos = []
    neg = []
    target_rel = rule.target
    for (x,y) in g1:
        if triples.is_known(x,target_rel,y): pos.append((x,y))
        else: neg.append((x,y))
    best_gain = 0.01
    (best_pl, best_nl, best_pr, best_nr) = (0,0,0,0) 
    best_split = (None, None)
    for relation in triples.rels:
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
    if best_gain > igmin:
        relation = best_split[2]
        var_order = "(X,Z)" if best_split[1] else "(Z,X)"
        print(rule)
        print(ttt.id2to[relation] + var_order + " " + str(ig(best_pl, best_nl, best_pr, best_nr)))
        print(str(best_pr+best_pl) + "|" + str(best_nr+best_nl) + " => L:" + str(best_pl) + "|" + str(best_nl) + " R:" + str(best_pr) + "|" + str(best_nr))
        print("---")
        return (best_split, (best_pl, best_nl, best_pr, best_nr))
    else:
        return None



if __name__ == '__main__':
    triples = TripleSet(config.paths["input"]["train"])
    rr = RuleReader()
    # rules = rr.read_file("E:/exp/pyclause/rules/rules-pctorm-wn18rr-b3")
    rules = rr.read_file("E:/exp/pyclause/rules/rules-pctorm-fb237-2-bonly")

    print("rules: " +  str(len(rules)))

    frules = []
    for rule in rules:
        if rule.get_confidence() > 0.1 and rule.cpred > 100:
            frules.append(rule)

    #rs = RuleSet()
    #rs.add_rules(frules)
    #rs.write("E:/exp/pyclause/rules/rules-pctorm-fb237-2-bonly")

    print("frules: " +  str(len(frules)))

    for rule in frules:
        get_best_refinement(triples, rule, 0.05)

  
    

