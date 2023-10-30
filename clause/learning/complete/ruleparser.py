
from rules import Rule, RuleUc, RuleB, RuleUd, RuleXXuc, RuleXXud, RuleZ, RuleSet
from triples import TripleSet


import re


# remove from a list the entities wigth index 0, 3, 6, ...
def get_terms(items):
    results = []
    for i, value in enumerate(items):
        if not i % 3 == 0: results.append(value)
    return results

# remove from a list the entities with index not equal to 0, 3, 6, ...
def get_relations(items):
    results = []
    for i, value in enumerate(items):
        if i % 3 == 0: results.append(value)
    return results

def get_dir_and_rels_of_body_atoms(atoms, is_brule):
    dirs = []
    brels = []
    vars = RuleB.var_symbols[len(atoms)-1]
    counter = 0
    for atom in atoms:
        atoken = atom.split("(")
        brels.append(atoken[0])
        if counter == 0 and not is_brule:
            dirs.append(atoken[1].startswith(vars[counter]) or atoken[1].startswith(vars[-1]))
        else:
            dirs.append(atoken[1].startswith(vars[counter]))
            # print(str(vars[counter]) + " :  " + str(atoken[1])  + " : " + str(atoken[1].startswith(vars[counter])))
        counter += 1
    return (brels, dirs)


class RuleReader:

    def __init__(self, rules):
        self.rules = rules
   
    def read_file(self, filepath):
        file = open(filepath, 'r')
        lines = file.readlines()
        rules = []
        count = 1
        count_rules = 0
        for line in lines:
            if count % 1000000 == 0:
                print(">>> ... read " + str(count) + " lines from rule file")
            count += 1
            rule = self.read_line(line.strip())
            
            if not(rule == None):
                rule.store()
                count_rules += 1
                # rules.append(rule)
        print(">>> read  " + str(count_rules) + " rules from file " + filepath)
        file.close()
        return rules

            

    def read_line(self, line):
        # print("reading: " + line)
        token = line.split("\t")
        if len(token) != 4: return None
        pred = int(token[0])
        cpred = int(token[1])
        conf = float(token[2])
        hb = token[3].split(" <= ")
        # *************** RuleB ***********************
        if "(X,Y)" in hb[0] or "(X, Y)" in hb[0]: 
            rels = []
            target = hb[0].split("(")[0]
            atoms = hb[1].split(", ")
            rels, dirs = get_dir_and_rels_of_body_atoms(atoms, True)
            rule = RuleB(self.rules, target, rels, dirs)
            rule.cpred, rule.pred, rule.conf  = cpred, pred, conf
            return rule
        else:
            if not(re.search("\(X,X\)", hb[0])) and not(re.search("\(X, X\)", hb[0])) and not(re.search("me_myself_i", hb[0])):
                htoken = re.split("\(|,|\)",hb[0])
                # print("htoken: " + str(htoken))
                target = htoken[0]
                # *************** RuleZ ***********************
                if len(hb) == 1 or (len(hb) == 2 and len(hb[1]) < 2):
                    if (htoken[1] == "X"):
                        rule = RuleZ(self.rules, target, htoken[2], True)
                    else:
                        rule = RuleZ(self.rules, target, htoken[1], False) 
                    rule.cpred, rule.pred, rule.conf  = cpred, pred, conf
                    return rule
                # *************** RuleUc or Ud ***********************

               



                else:
                    target = hb[0].split("(")[0]
                    atoms = hb[1].split(", ")
                    rels, dirs = get_dir_and_rels_of_body_atoms(atoms, False)


                    
                    btoken = re.split("\(|,|\)",hb[1])

                    btoken = list(filter(lambda x: not (x.isspace() or x == ''), btoken))
                    btoken = list(map(lambda x: x.strip(), btoken))
                    bterms = get_terms(btoken)

                    b_consts = list(filter(lambda x: len(x) > 1, bterms))
                    b_const = b_consts[0] if len(b_consts) > 0 else None
                    
                    # this is a special case of rules learned by AnyBURL that reflect regularities that are casued by a tricky rewriting
                    # these rules make no sense
                    if b_const == "me_myself_i": return None

                     # XX rule
                    if htoken[1] == "me_myself_i" or htoken[2] == "me_myself_i" or (htoken[1] == "X" and htoken[2] == "X"):
                         # *************** XX - RuleUd ***********************
                        if b_const == None:
                            rule = RuleXXud(self.rules, target, rels[0], dirs[0])
                        else:
                            rule = RuleXXuc(self.rules, target, rels[0], b_const, dirs[0])
                        rule.cpred, rule.pred, rule.conf  = cpred, pred, conf
                        return rule
                    else:
                        # *************** RuleUd ***********************
                        if b_const == None:
                            if htoken[1] == "X":
                                rule = RuleUd(self.rules, target, rels, dirs, htoken[2], True)
                            else:
                                rule = RuleUd(self.rules, target, rels, dirs, htoken[1], False)
                            rule.cpred, rule.pred, rule.conf  = cpred, pred, conf
                        # *************** RuleUc ***********************
                        else:
                            if htoken[1] == "X":
                                rule = RuleUc(self.rules, target, rels, dirs, htoken[2], True, b_const)
                            else:
                                rule = RuleUc(self.rules, target, rels, dirs, htoken[1], False, b_const)
                            rule.cpred, rule.pred, rule.conf  = cpred, pred, conf
                        return rule
            return None



if __name__ == '__main__':
    triples = TripleSet("E:/code/eclipse-workspace/AnyBURL/data/wn18rr/train.txt")
    rules = RuleSet(triples.index)
    rr = RuleReader(rules)

    # r1s = "2972	5	1.68	_derivationally_related_form(X,07554856) <= _also_see(X,A), _also_see(A,B), _derivationally_related_form(B,C)"
    r1s = "2972	5	1.68	_derivationally_related_form(07554856,Y) <= _also_see(Y,A), _also_see(A,B), _derivationally_related_form(B,C)"
    r1 = rr.read_line(r1s)
    print("Rule input:  " +  r1s)
    print("Rule check:  " +  str(r1))
    print("Rule vector: " +  str(r1.vectorized()))