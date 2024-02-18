import sys
import re


class Triple:
   """
   A triple from a knowledge graph which consist of subject, relation, object.
   The triple has also a reference to the index which it maps entities and relations to integer ids.
   """
   def __init__(self, sub, rel, obj, index):
      s = sys.intern(sub)
      r = sys.intern(rel)
      o = sys.intern(obj)
      self.index = index
      self.sub = self.__get_or_create_id(s)
      self.rel = self.__get_or_create_id(r)
      self.obj = self.__get_or_create_id(o)
      self.hashcode = None

   def __get_or_create_id(self, to):
      if to in self.index.to2id:
         id_ = self.index.to2id[to]
         return id_
      else:
         id_ = self.index.my_id
         self.index.to2id[to]  = id_
         self.index.id2to[id_] = to
         self.index.my_id += 1
         return id_

   def __str__(self):
      return f"{self.index.id2to[self.sub]} {self.index.id2to[self.rel]} {self.index.id2to[self.obj]}"
        
   def __eq__(self, other):
      return (self.sub == other.sub) and (self.rel == other.rel) and (self.obj == other.obj)

   def __hash__(self):
      if self.hashcode == None:
         self.hashcode = self.sub + self.rel + self.obj
      return self.hashcode
   


   def get_head_token(self):
      return self.index.id2to[self.sub]
   
   def get_tail_token(self):
      return self.index.id2to[self.obj]

   def get_relation_token(self):
      return self.index.id2to[self.rel]


class TripleSet:
   """
    A triple set is a list of triples with several index structures to support
    fast graph traversal of the knowledge graph defined by the triple set.
   """
   
   def __init__(self, path, index = None, encod = None):
      print(">>> loading triple set from path " + str(path) + " ...")
      
      # the path from which the triple set has been loaded
      self.path = path
      self.encod = encod
      
      # list of triples
      self.triples = []
      
      # use the index that has been passed as argument or dreate new empty index
      self.index = index
      if self.index == None:  self.index = TripleIndex()

      # the core index structure that points to sets
      self.sub_rel_2_obj = {}
      self.obj_rel_2_sub = {}

      # the core index structre that point so lists (list are required whenever you wwant to sample)
      self.sub_rel_2_obj_list = {}
      self.obj_rel_2_sub_list = {}

      self.r2r_ss = {} # informs if there is r1(x,y),r2(x,z) in the triple set
      self.r2r_so = {} # informs if there is r1(x,y),r2(z,x) in the triple set
      self.r2r_os = {} # informs if there is r1(x,y),r2(y,z) in the triple set
      self.r2r_oo = {} # informs if there is r1(x,y),r2(z,y) in the triple set

      # all object / subjects that have appear together with a relation in a triple of the this tripleset
      self.r2_obj = {}
      self.r2_sub = {}
      
      # all of the relations within the dataset
      self.rels = set()

      # maps a relation to the number of triples using that relation
      self.r2_count = {} 

      # counts for each relation how many triples of the form r(x,x) instantiate relation r 
      self.r2_count_reflexiv = {}
      f = None
      if self.encod == None: f = open(path, "r")
      else: f = open(path, "r", encoding=self.encod)
      for line in f:
         token = line.split()
         if len(token) == 3:
            t = Triple(token[0], token[1], token[2], self.index)
            self.triples.append(t)
            # two lines to build the core index structures 
            self.__index_triple(self.sub_rel_2_obj, self.sub_rel_2_obj_list, t.sub, t.rel, t.obj)
            self.__index_triple(self.obj_rel_2_sub, self.obj_rel_2_sub_list, t.obj, t.rel, t.sub)
            self.rels.add(t.rel)
            if not(t.rel in self.r2_count): self.r2_count[t.rel] = 1
            else: self.r2_count[t.rel] += 1
            if t.sub == t.obj:
               if not(t.rel in self.r2_count_reflexiv): self.r2_count_reflexiv[t.rel] = 1
               else: self.r2_count_reflexiv[t.rel] += 1

      f.close()
      self.__index_paths()
      self.__index_sub_obj()
      print(">>> ... read and indexed " +  str(len(self.triples)) + " triples.")

   def __index_sub_obj(self):
      """
      Collects per relation the entities that appear in subject object position. 
      """
      for rel in self.rels:
         self.r2_sub[rel] = set()
         self.r2_obj[rel] = set()
      for x in self.sub_rel_2_obj:
         for rel in self.sub_rel_2_obj[x]:
            self.r2_obj[rel].update(self.sub_rel_2_obj[x][rel])
      for x in self.obj_rel_2_sub:
         for rel in self.obj_rel_2_sub[x]:
            self.r2_sub[rel].update(self.obj_rel_2_sub[x][rel])

   def __index_paths(self):
      """
      Checks and stores whether two relations share the same entity in their subject/object positions.
      By precomputing this it is possible to speed up rule mining as impossible rules can be pruned. 
      """
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
      return "triple set with " +  str(len(self.triples)) + " triples and " + str(len(self.rels)) + " relations (from: " + self.path + ")"

   def __index_triple(self, x_y_2_z, x_y_2_z_list, x, y, z):
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
      '''
      Used to check in constant time if a triple (index format) exists within the dataset, i.e. if the triple is known.
      '''
      if sub in self.sub_rel_2_obj and rel in self.sub_rel_2_obj[sub] and obj in self.sub_rel_2_obj[sub][rel]:
         return True
      return False

   def is_true(self, sub_token, rel_token, obj_token):
      '''
      Used to check in constant time if a triple (token format) exists within the dataset, i.e. if the triple is known.
      '''
      if rel_token in self.index.to2id and obj_token in self.index.to2id and sub_token in self.index.to2id:
         return self.is_known(self.index.to2id[sub_token], self.index.to2id[rel_token], self.index.to2id[obj_token])
      else:
         # this can occur ...
         return False
   
   def get_1to1_score(self, rel):
      num_obj = len(self.r2_obj[rel])
      num_sub = len(self.r2_sub[rel])
      sub_per_obj = num_sub / num_obj
      obj_per_sum = num_obj / num_sub
      rel_1to1 = min(sub_per_obj, obj_per_sum)
      return rel_1to1
   
   def write_masked(self, outpath, replace_non_alpha_numeric = True):
      f = None
      if self.encod == None: f = open(outpath, "w")
      else: f = open(outpath, "w", encoding=self.encod)


      for triple in self.triples:
         sub = triple.index.id2to[triple.sub]
         rel = triple.index.id2to[triple.rel]
         obj = triple.index.id2to[triple.obj]
         if replace_non_alpha_numeric:
            sub = re.sub('[^0-9a-zA-Z\-\_]+', '*', sub)
            rel = re.sub('[^0-9a-zA-Z\-\_]+', '*', rel)
            obj = re.sub('[^0-9a-zA-Z\-\_]+', '*', obj)
         else:
            sub = sub.replace(',','~')
            rel = rel.replace(',','~')
            obj = obj.replace(',','~')

            sub = sub.replace("(", "[")
            sub = sub.replace(")", "]")
            obj = obj.replace("(", "[")
            obj = obj.replace(")", "]")

         ts = "e" + sub + "\tr" + rel + "\te" + obj
         f.write(ts + "\n")
      f.close()

   def to_list(self):
      
      """Outputs a list of string striples.

       A string triple is a list or tuple with three string elements ["head", "relation", "tail"].

      """
      return [str(trip).split(" ") for trip in self.triples]


class TripleIndex:
   """
   A simple index thta maps the raw strings (called tokens, short 'to' here) to integer ids and vice versa.
   The counter my_id is increased whenever a new integer id is used. 
   """

   def __init__(self):
      self.to2id = {}
      self.id2to = {}
      self.my_id = 1 

   def __str__(self):
      return "triple index with " + str(self.my_id - 1) + " entries"

