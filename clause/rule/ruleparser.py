import re

from clause.rule.rules import RuleUc, RuleB, RuleUd, RuleXXuc, RuleXXud, RuleZ


class RuleTranslator():
    """Translates rules represented with integer idx's to string format."""

    def __init__(self, idx_to_ent, idx_to_rel):
        self.idx_to_ent = idx_to_ent
        self.idx_to_rel = idx_to_rel
        self.vars = "XABCDEFGHIJKLMNOPQRSTUVWY"
        self.left_vars = "YABCDEFGHIJKLMNOPQRSTUVWX"
        #head body separator
        self.h_b_sep = " <= "
        #atom separator
        self.atom_sep = ", "
        self.atom_template = ("()")
    

    def _make_atom(self, rel_string, term_1, term_2):
        return f"{rel_string}({term_1},{term_2})"


    def translate_b_rules(self, relations, directions):
        """Translates B-rules in idx format to string format.

            param: (list[list[int]]) relations: head and body relations
            param: (list[list[bool]]) directions: provides the direction within an atom. First element must be 1.

        """
        
        if not len(relations)==len(directions):
            raise Exception("Each rule must have a directions vector.")
        
        rule_strings = []
        
        for i in range(len(relations)):
            curr_r_string = ""
            rels = relations[i]
            dirs = directions[i]
            head_rel = self.idx_to_rel[rels[0]]
            curr_r_string += f"{self._make_atom(head_rel, self.vars[0], self.vars[-1])}{self.h_b_sep}"

            for j in range(1,len(rels)):
                rel = self.idx_to_rel[rels[j]]
                if j<len(rels)-1:
                    curr_r_string += \
                        (self._make_atom(rel, self.vars[j-1], self.vars[j]) if dirs[j] else self._make_atom(rel, self.vars[j], self.vars[j-1]))
                    curr_r_string += self.atom_sep
                else:
                    curr_r_string += \
                        (self._make_atom(rel, self.vars[j-1], self.vars[-1]) if dirs[j] else self._make_atom(rel, self.vars[-1], self.vars[j-1]))
            
            rule_strings.append(curr_r_string)
        return rule_strings
    

    def translate_c_rules(self, relations, directions, constants, leftC):
        """Translates Uc-rules in idx format to string format.

            :param (list[list[int]]) relations: head and body relations
            :param (list[list[bool]]) directions: provides the direction within an atom. First element must be 1.
            :param (list[list[int, int]]) constants: each list contains 2 entity idx's the first el corresponds to head constant, second to body constant
            :param (list[bool]) leftC: denotes if the left (X) or right (Y) variable of the head is grounded

        """
        if not len(relations)==len(directions)==len(leftC)==len(constants):
            raise Exception("All inputs needs to have same length.")
        
        rule_strings = []
        
        for i in range(len(relations)):
            curr_r_string = ""
            rels = relations[i]
            dirs = directions[i]
            consts = constants[i]
            is_leftC = leftC[i]
            head_rel = self.idx_to_rel[rels[0]]
            head_ent = self.idx_to_ent[consts[0]]
            body_ent = self.idx_to_ent[consts[1]]
            
            curr_r_string += (
                                f"{self._make_atom(head_rel, head_ent, self.vars[-1])}{self.h_b_sep}" if is_leftC else
                                f"{self._make_atom(head_rel, self.vars[0], head_ent)}{self.h_b_sep}"
            )
            vars = self.vars
            if is_leftC:
                vars = self.left_vars
            for j in range(1,len(rels)):
                rel = self.idx_to_rel[rels[j]]
                if j<len(rels)-1:
                    curr_r_string += \
                        (self._make_atom(rel, vars[j-1], vars[j]) if dirs[j] else self._make_atom(rel, vars[j], vars[j-1]))
                    curr_r_string += self.atom_sep
                else:
                    curr_r_string += \
                        (self._make_atom(rel, vars[j-1], body_ent) if dirs[j] else self._make_atom(rel, body_ent, vars[j-1]))
            rule_strings.append(curr_r_string)
        return rule_strings


    def translate_d_rules(self, relations, directions, constants, leftC):
        """Translates Ud-rules from idx format to string format.

            :param (list[list[int]]) relations: head and body relations
            :param (list[list[bool]]) directions: provides the direction within an atom. First element must be 1.
            :param (list[int]) constants: head constants
            :param (list[bool]) leftC: denotes if the left (X) or right (Y) variable of the head is grounded

        """
        if not len(relations)==len(directions)==len(leftC)==len(constants):
            raise Exception("All inputs needs to have same length.")
        
        rule_strings = []
        
        for i in range(len(relations)):
            curr_r_string = ""
            rels = relations[i]
            dirs = directions[i]
            const = constants[i]
            is_leftC = leftC[i]
            head_rel = self.idx_to_rel[rels[0]]
            head_ent = self.idx_to_ent[const]
            
            curr_r_string += (
                                f"{self._make_atom(head_rel, head_ent, self.vars[-1])}{self.h_b_sep}" if is_leftC else
                                f"{self._make_atom(head_rel, self.vars[0], head_ent)}{self.h_b_sep}"
            )
            vars = self.vars
            if is_leftC:
                vars = self.left_vars
            for j in range(1,len(rels)):
                rel = self.idx_to_rel[rels[j]]
                curr_r_string += \
                    (self._make_atom(rel, vars[j-1], vars[j]) if dirs[j] else self._make_atom(rel, vars[j], vars[j-1]))
                if (j<len(rels)-1):
                    curr_r_string += self.atom_sep
            rule_strings.append(curr_r_string)
        return rule_strings


    def translate_z_rules(self, relations, constants, leftC):
        """Translates zero-rules from idx format to string format.

            :param (list[int]) relations: head relations
            :param (list[int]) constants: head constants
            :param  (list[bool]) leftC: denotes if the left (X) or right (Y) variable of the head is grounded

        """

        if not len(relations)==len(leftC)==len(constants):
            raise Exception("All inputs needs to have same length.")
        
        rule_strings = []
        
        for i in range(len(relations)):
            curr_r_string = ""
            rel = self.idx_to_rel[relations[i]]
            const = self.idx_to_ent[constants[i]]
            is_leftC = leftC[i]
            
            if (is_leftC):
                rule_strings.append(self._make_atom(rel, const, self.vars[-1]) + self.h_b_sep)
            else:
                rule_strings.append(self._make_atom(rel, self.vars[0], const) + self.h_b_sep)
        return rule_strings


    def translate_u_xxd_rules(self, relations, directions):
        """Translates u_xxd rules from idx format to string format.

            :param (list[list[int,int]]) relations: head and body relations
            :param (list[list[bool,bool]]) directions: first bool must be true, second is body atom direction

        """

        if not len(relations)==len(directions):
            raise Exception("All inputs needs to have same length.")
        
        rule_strings = []
        
        for i in range(len(relations)):
            curr_r_string = ""
            rels = relations[i]
            dirs = directions[i] 
            head_rel = self.idx_to_rel[rels[0]]
            body_rel =self.idx_to_rel[rels[1]]

            if dirs[1]:
                rule_strings.append(
                    self._make_atom(head_rel, self.vars[0], self.vars[0]) + self.h_b_sep + self._make_atom(body_rel, self.vars[0], self.vars[1])
                )
            else:
                 rule_strings.append(
                    self._make_atom(head_rel, self.vars[0], self.vars[0]) + self.h_b_sep + self._make_atom(body_rel, self.vars[1], self.vars[0])
                )
        return rule_strings


    def translate_u_xxc_rules(self, relations, directions, constants):
        """Translates u_xxc rules from idx format to string format.

            :param (list[list[int,int]]) relations: head and body relations
            :param (list[list[bool, bool]]) directions: first bool must be true, second is body atom direction
            :param (list[int]) constants: the body constants

        """

        if not len(relations)==len(directions):
            raise Exception("All inputs needs to have same length.")
        
        rule_strings = []
        
        for i in range(len(relations)):
            curr_r_string = ""
            rels = relations[i]
            dirs = directions[i] 
            head_rel = self.idx_to_rel[rels[0]]
            body_rel =self.idx_to_rel[rels[1]]
            const = self.idx_to_ent[constants[i]]

            if dirs[1]:
                rule_strings.append(
                    self._make_atom(head_rel, self.vars[0], self.vars[0]) + self.h_b_sep + self._make_atom(body_rel, self.vars[0], const)
                )
            else:
                 rule_strings.append(
                    self._make_atom(head_rel, self.vars[0], self.vars[0]) + self.h_b_sep + self._make_atom(body_rel, const, self.vars[0])
                )
        return rule_strings


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
        print(">>> read  " + str(count_rules) + " rules from file " + filepath)
        file.close()

            
    def read_line(self, line):
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
            htoken = re.split("\(|,|\)",hb[0])
            target = htoken[0]
            # *************** RuleZ ***********************
            if (len(hb) == 1 or (len(hb) == 2 and len(hb[1]) < 2)):
                if re.search("me_myself_i", hb[0]): return None           
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