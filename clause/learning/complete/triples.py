import sys
import config
import math
import time


# global dictionaries for mapping between ids and token
to2id = {}
id2to = {}
my_id = 1 


class Triple:
   """
   A triple from a knowledge graph which consist of subject, relation, object.
   """


   def __init__(self, sub, rel, obj):
      s = sys.intern(sub)
      r = sys.intern(rel)
      o = sys.intern(obj)
      self.sub = self.get_or_create_id(s)
      self.rel = self.get_or_create_id(r)
      self.obj = self.get_or_create_id(o)
      self.hashcode = None

   def get_or_create_id(self, to):
      global my_id
      if to in to2id:
         id_ = to2id[to]
         return id_
      else:
         id_ = my_id
         # if id_ % 100 == 0: print(str(id_))
         to2id[to]  = id_
         id2to[id_] = to
         my_id += 1
         return id_


   def __str__(self):
      return f"{id2to[self.sub]} {id2to[self.rel]} {id2to[self.obj]}"
        
   def __eq__(self, other):
      return (self.sub == other.sub) and (self.rel == other.rel) and (self.obj == other.obj)

   def __hash__(self):
      if self.hashcode == None:
         self.hashcode = self.sub + self.rel + self.obj
      return self.hashcode


class TripleSet:
   """
    A triple set is a list of triples with several index structures to support
    fast graph traversal of the knowledge graph defined by the triple set.
   """
   
   def __init__(self, path):
      print(">>> loading triple set from path " + str(path) + " ...")
      self.path = path
      self.triples = []
      self.sub_rel_2_obj = {}
      self.obj_rel_2_sub = {}

      self.sub_rel_2_obj_list = {}
      self.obj_rel_2_sub_list = {}

      self.r2r_ss = {} # informs if there is r1(x,y),r2(x,z) in the triple set
      self.r2r_so = {} # informs if there is r1(x,y),r2(z,x) in the triple set
      self.r2r_os = {} # informs if there is r1(x,y),r2(y,z) in the triple set
      self.r2r_oo = {} # informs if there is r1(x,y),r2(z,y) in the triple set

      self.r2_obj = {}
      self.r2_sub = {}
      
      self.rels = set()

      self.r2_count = {} # maps a relation to the number of triples using that relation

      self.r2_count_reflexiv = {} # counts for each relation how many triples of the form r(x,x) instantiate relation r 

      f = open(path, "r")
      for line in f:
         token = line.split()
         if len(token) == 3:
            t = Triple(token[0], token[1], token[2])
            self.triples.append(t)
            self.index_triple(self.sub_rel_2_obj, self.sub_rel_2_obj_list, t.sub, t.rel, t.obj)
            self.index_triple(self.obj_rel_2_sub, self.obj_rel_2_sub_list, t.obj, t.rel, t.sub)
            self.rels.add(t.rel)
            if not(t.rel in self.r2_count): self.r2_count[t.rel] = 1
            else: self.r2_count[t.rel] += 1
            if t.sub == t.obj:
               if not(t.rel in self.r2_count_reflexiv): self.r2_count_reflexiv[t.rel] = 1
               else: self.r2_count_reflexiv[t.rel] += 1

      f.close()
      self.index_paths()
      self.index_sub_obj()
      print(">>> ... read and indexed " +  str(len(self.triples)) + " triples.")

   def index_sub_obj(self):
      for rel in self.rels:
         self.r2_sub[rel] = set()
         self.r2_obj[rel] = set()
      for x in self.sub_rel_2_obj:
         for rel in self.sub_rel_2_obj[x]:
            self.r2_obj[rel].update(self.sub_rel_2_obj[x][rel])
      for x in self.obj_rel_2_sub:
         for rel in self.obj_rel_2_sub[x]:
            self.r2_sub[rel].update(self.obj_rel_2_sub[x][rel])

   def index_paths(self):
      for rel in self.rels:
         self.r2r_ss[rel] = set()
         self.r2r_so[rel] = set()
         self.r2r_os[rel] = set()
         self.r2r_oo[rel] = set()
      for x in self.sub_rel_2_obj:
         for rel1 in self.sub_rel_2_obj[x]:
            for rel2 in self.sub_rel_2_obj[x]:
               self.r2r_ss[rel1].add(rel2)
            if x in self.obj_rel_2_sub:
               for rel2 in self.obj_rel_2_sub[x]:
                  self.r2r_so[rel1].add(rel2)
      for x in self.obj_rel_2_sub:
         for rel1 in self.obj_rel_2_sub[x]:
            for rel2 in self.obj_rel_2_sub[x]:
               self.r2r_oo[rel1].add(rel2)
            if x in self.sub_rel_2_obj:
               for rel2 in self.sub_rel_2_obj[x]:
                  self.r2r_os[rel1].add(rel2)
      

   def __str__(self):
      return "triple set with " +  str(len(self.triples)) + " triples using " + str(len(self.rels)) + " relations loaded from " + self.path

   def index_triple(self, x_y_2_z, x_y_2_z_list, x, y, z):
      if x not in x_y_2_z:
         x_y_2_z[x] = {}
         x_y_2_z_list[x] = {}
      if y not in x_y_2_z[x]:
         x_y_2_z[x][y] = {z}
         x_y_2_z_list[x][y] = [z]
      else:
         x_y_2_z[x][y].add(z)
         x_y_2_z_list[x][y].append(z)

   def is_known(self, sub, rel, obj):
      if sub in self.sub_rel_2_obj and rel in self.sub_rel_2_obj[sub] and obj in self.sub_rel_2_obj[sub][rel]:
         return True
      return False
   
   def get_1to1_score(self, rel):
      num_obj = len(self.r2_obj[rel])
      num_sub = len(self.r2_sub[rel])
      sub_per_obj = num_sub / num_obj
      obj_per_sum = num_obj / num_sub
      rel_1to1 = min(sub_per_obj, obj_per_sum)
      return rel_1to1


class PredictionSet:
   """
   A set of triples that are predicted by rules. A prediction set is a a triple set whre each triple is explained
   by at least one rule.  However, a prediction set has differnt index strcuture than a triple set.
   """

   def __init__(self):
      self.sub_rel_2_obj_rules = {}
      self.obj_rel_2_sub_rules = {}
      self.prediction_counter = 0
      self.rules = []

   def add_prediction(self, sub, rel, obj, rule):
      self.index_predicted_triple(self.sub_rel_2_obj_rules, sub, rel, obj, rule, True)
      self.index_predicted_triple(self.obj_rel_2_sub_rules, obj, rel, sub, rule, False)

   def __str__(self):
      return "prediction set with " +  str(self.prediction_counter) + " predictions"

   def index_predicted_triple(self, x_y_2_z_rules, x, y, z, rule, count):
      if x not in x_y_2_z_rules:
         x_y_2_z_rules[x] = {}
      if y not in x_y_2_z_rules[x]:
         x_y_2_z_rules[x][y] = {}
      if z not in x_y_2_z_rules[x][y]:
         x_y_2_z_rules[x][y][z] = {rule}
         if count: self.prediction_counter += 1
      else:
         x_y_2_z_rules[x][y][z].add(rule)




   def sort_explaining_rules(self):
      # self.rules.sort(key=lambda r : r.aconfidence, reverse=True)
      self.rules = self.sort_rules(self.rules)
      #for r in self.rules:
      #   print(str(r.aconfidence) + "   " +  str(r)   
      for sub in self.sub_rel_2_obj_rules:
         for rel in self.sub_rel_2_obj_rules[sub]:
            for obj in self.sub_rel_2_obj_rules[sub][rel]:
               xrules = []
               xrules.extend(self.sub_rel_2_obj_rules[sub][rel][obj])
               xrules = self.sort_rules(xrules)
               self.sub_rel_2_obj_rules[sub][rel][obj] = xrules
               self.obj_rel_2_sub_rules[obj][rel][sub] = xrules

   def sort_rules(self, rules):
      # use standard sorting if its only few rules
      if (len(rules) < 100):
          rules.sort(key=lambda r : r.aconfidence, reverse=True)
          return rules
      # use count sort if its many rules
      srules = []
      bins = {}
      num_of_bins = 1000
      for b in range(num_of_bins-1,-1,-1):
         bins[b] = []
      for rule in rules:
         b = math.floor(rule.aconfidence * num_of_bins) 
         bins[b].append(rule)
      for b in range(num_of_bins-1,-1,-1):
         bins[b].sort(key=lambda r : r.aconfidence, reverse=True)
      for b in range(num_of_bins-1,-1,-1):
         srules.extend(bins[b])
      return srules


   def merge(self, other):
      """
      TAKE CARE: The merge is usually applied between two prediction sets that might not have the access to the interned
      strings. Thus you have to use '=='' instead of 'is', while for the remaining code it shozuld be okay to use 'is' which
      is expected to be faster.
      """

      # make a deep copy
      for sub in other.sub_rel_2_obj_rules:
         for rel in other.sub_rel_2_obj_rules[sub]:
            for obj in other.sub_rel_2_obj_rules[sub][rel]:
               for rule in other.sub_rel_2_obj_rules[sub][rel][obj]:
                  self.index_predicted_triple(self.sub_rel_2_obj_rules, sub, rel, obj, rule, True)
                  self.index_predicted_triple(self.obj_rel_2_sub_rules, obj, rel, sub, rule, False)

class CompletionSet:

   def __init__(self, tasks, predictions):
      self.tasks = tasks
      self.predictions = predictions
      self.head_predictions = {}
      self.tail_predictions = {}


   def collect(self):
      print(">>> collecting relevant predictions ...")
      for t in self.tasks.triples:
         self.tail_predictions[t] = []
         if t.sub in self.predictions.sub_rel_2_obj_rules:
            if t.rel in self.predictions.sub_rel_2_obj_rules[t.sub]:
               for obj in self.predictions.sub_rel_2_obj_rules[t.sub][t.rel]:
                  self.tail_predictions[t].append((obj, self.predictions.sub_rel_2_obj_rules[t.sub][t.rel][obj][0].aconfidence))
         self.head_predictions[t] = []
         if t.obj in self.predictions.obj_rel_2_sub_rules:
            if t.rel in self.predictions.obj_rel_2_sub_rules[t.obj]:
               for sub in self.predictions.obj_rel_2_sub_rules[t.obj][t.rel]:
                  self.head_predictions[t].append((sub, self.predictions.obj_rel_2_sub_rules[t.obj][t.rel][sub][0].aconfidence))
                  if self.predictions.obj_rel_2_sub_rules[t.obj][t.rel][sub][0].aconfidence > 0.99:
                     print("*** high rule: " + str(self.predictions.obj_rel_2_sub_rules[t.obj][t.rel][sub][0]))
                     print("*** creating: " + id2to[sub] + " " + id2to[t.rel] + " " + id2to[t.obj])
      print(">>> sorting the explaining rules of collected predictions ...")
      for t in self.tasks.triples:
         self.tail_predictions[t].sort(key=lambda x : x[1], reverse=True)
         self.head_predictions[t].sort(key=lambda x : x[1], reverse=True)

   def filter(self, train):
      # the train triples set will be used only if ist defined like that in the config file
      filter_sets = []
      for fset_path in config.paths["input"]["filter"]:
         loaded = False
         if fset_path == config.paths["input"]["train"]:
            print(">>> > > > add train set to filter sets")
            filter_sets.append(train)
            loaded = True
         if fset_path == config.paths["input"]["tasks"]:
            print(">>> > > > add test set to filter sets")
            filter_sets.append(self.tasks)
            loaded = True
         if not(loaded):
            fset = TripleSet(fset_path)
            filter_sets.append(fset)
         
      max_num_candidates = 100
      for t in self.tasks.triples:
         self.filter_candidate_list(t, self.tail_predictions[t], filter_sets, max_num_candidates, False)
         self.filter_candidate_list(t, self.head_predictions[t], filter_sets, max_num_candidates, True) 


   def filter_candidate_list(self, t, candidates, filter_sets, max_num_candidates, head_not_tail):
      i = 0
      while i < len(candidates) and i < max_num_candidates:
         candidate = candidates[i][0]
         ct = (candidate, t.rel, t.obj) if head_not_tail else (t.sub, t.rel, candidate)
         if head_not_tail:
            if candidate == t.sub:
               i += 1
               continue
         else:
            if candidate == t.obj:
               i += 1
               continue
         remove = False
         f = 0
         for fset in filter_sets:
            x = id2to[ct[0]] + " " + id2to[ct[1]] + " " + id2to[ct[2]]
            if fset.is_known(*ct):
               remove = True
               break
            f += 1
         if remove: candidates.pop(i)
         else: i += 1
      if len(candidates) > max_num_candidates:
         del candidates[100:] # works in place what is required here


   def write_ranking(self, path):
      print(">>> ... writing the ranking-file")
      f = open(path, "w")
      for t in self.tasks.triples:
         f.write(str(t) + "\n")
         f.write("Heads: ")
         for pair in self.head_predictions[t]:
            f.write(str(id2to[pair[0]]) + "\t" + str(pair[1]) + "\t")
         f.write("\n")
         f.write("Tails: ")
         for pair in self.tail_predictions[t]:
            f.write(str(id2to[pair[0]]) + "\t" + str(pair[1]) + "\t")
         f.write("\n")
      f.close()


if __name__ == '__main__':
   triples = TripleSet(config.paths["input"]["train"])
   count = 0
   for rel in triples.rels:
      num_obj = len(triples.r2_obj[rel])
      num_sub = len(triples.r2_sub[rel])
      sub_per_obj = num_sub / num_obj
      obj_per_sum = num_obj / num_sub
      rel_1to1 = min(sub_per_obj, obj_per_sum)
      if rel_1to1 < 0.1:
         count += 1
         print("(" + str(count) + ") " + id2to[rel] + ": " + str(num_sub) + " | " + str(num_obj) + " | " + str(triples.get_1to1_score(rel)))