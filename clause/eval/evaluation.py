import sys
import os

from clause.data.triples import TripleSet


class Hits():
    """
    Used to compute MRR and Hits@k metrics.
    """

    def __init__(self):
        self.ATKMAX = 500 # might not be required in the python version
        self.init()


    def init(self):
        self.tail_ranks = []
        self.hits_ad_n_tail = [0] * self.ATKMAX
        self.counter_tail = 0
        self.counter_tail_covered = 0
        self.head_ranks = []
        self.hits_ad_n_head = [0] * self.ATKMAX
        self.counter_head = 0
        self.counter_head_covered = 0   
    
    def compute_scores(self, ranking, triples, head_direction = True, tail_direction = False):
        self.init()
        for triple in triples:
            if head_direction:
                cands = ranking.get_head_candidates(str(triple))
                self.evaluate_head(cands, str(triple))
            if tail_direction:
                cands = ranking.get_tail_candidates(str(triple))
                self.evaluate_tail(cands, str(triple))
        

    def get_mrr(self):
        """
        Computes and returns the MRR.
        """
        mrr = 0.0
        hk = 0.0
        hk_prev = 0.0
        hk_diff = 0.0
        for k in range(self.ATKMAX):
            hk = (self.hits_ad_n_head[k] + self.hits_ad_n_tail[k]) / (self.counter_head + self.counter_tail)
            hk_diff = hk - hk_prev
            mrr += hk_diff * (1.0 / (k+1))
            hk_prev = hk
        return mrr
    
    def get_mr(self, num_of_entities):
        """
        Computes and returns the mean rank.
        miss_k determines the assumed position of a
        candidate that was not found in the ranking.
        """
        return (self.get_mr_head(num_of_entities) + self.get_mr_tail(num_of_entities)) / 2
    
    def get_mrr_head(self):
        """
        Computes and returns the head MRR.
        """
        mrr = 0.0
        hk = 0.0
        hk_prev = 0.0
        hk_diff = 0.0
        for k in range(self.ATKMAX):
            hk = self.hits_ad_n_head[k] / self.counter_head
            hk_diff = hk - hk_prev
            mrr += hk_diff * (1.0 / (k+1))
            hk_prev = hk
        return mrr
    
    def get_mr_head(self, num_of_entities):
        """
        Computes and returns the head mean rank.
        num_of_entities determines the assumed position of a
        candidate that was not found in the ranking.
        """
        mr = 0.0
        rank_sum = 0
        counter_found = 0
        previous = 0
        for k in range(self.ATKMAX):
            current = self.hits_ad_n_head[k] - previous
            rank_sum = rank_sum + current * (k+1)
            counter_found = counter_found + current
            previous = self.hits_ad_n_head[k]
        if counter_found != self.counter_head:
            rank_sum = rank_sum + (self.counter_head - counter_found) * num_of_entities
        return rank_sum / self.counter_head
    
    def get_mrr_tail(self):
        """
        Computes and returns the tail MRR.
        """
        mrr = 0.0
        hk = 0.0
        hk_prev = 0.0
        hk_diff = 0.0
        for k in range(self.ATKMAX):
            hk = self.hits_ad_n_tail[k] / self.counter_tail
            hk_diff = hk - hk_prev
            mrr += hk_diff * (1.0 / (k+1))
            hk_prev = hk
        return mrr
    

    def get_mr_tail(self, num_of_entities):
        """
        Computes and returns the tail mean rank.
        num_of_entities determines the assumed position of a
        candidate that was not found in the ranking.
        """
        mr = 0.0
        rank_sum = 0
        counter_found = 0
        previous = 0
        for k in range(self.ATKMAX):
            current = self.hits_ad_n_tail[k] - previous
            rank_sum = rank_sum + current * (k+1)
            counter_found = counter_found + current
            previous = self.hits_ad_n_tail[k]
        if counter_found != self.counter_tail:
            rank_sum = rank_sum + (self.counter_tail - counter_found) * num_of_entities
        return rank_sum / self.counter_tail

    def get_hits_at_k(self, k):
        k = k -1
        return (self.hits_ad_n_head[k] + self.hits_ad_n_tail[k]) / (self.counter_head + self.counter_tail)

    def evaluate_head(self, candidates, triple):
        (t_head, t_relation, t_tail) = triple.split()
        foundAt = -1
        self.counter_head += 1
        if len(candidates) > 0: self.counter_head_covered += 1
        for rank in range(len(candidates)):
            candidate = candidates[rank]
            if candidate == t_head:
                for index in range(rank, rank + self.ATKMAX):
                    # print("i=" + str(index))
                    if index < self.ATKMAX:
                        self.hits_ad_n_head[index] += 1
                foundAt = rank + 1
                break
        counter = 0
        ranked = False
        for candidate in candidates:
            counter += 1
            if candidate == t_head:
                self.head_ranks.append(counter)
                ranked = True
                break
        if not ranked:
            self.head_ranks.append(-1)
        return foundAt


    def evaluate_tail(self, candidates, triple):
        (t_head, t_relation, t_tail) = triple.split()
        foundAt = -1
        self.counter_tail += 1
        if len(candidates) > 0: self.counter_tail_covered += 1
        for rank in range(len(candidates)):
            candidate = candidates[rank]
            if candidate == t_tail:
                for index in range(rank, rank + self.ATKMAX):
                    if index < self.ATKMAX:
                        self.hits_ad_n_tail[index] += 1
                foundAt = rank + 1
                break
        counter = 0
        ranked = False
        for candidate in candidates:
            counter += 1
            if candidate == t_tail:
                self.tail_ranks.append(counter)
                ranked = True
                break
        if not ranked:
            self.tail_ranks.append(-1)
        return foundAt
    

class CompletionResult():

    def __init__(self, triple_as_string):
        self.triple = triple_as_string
        self.head_results = []
        self.tail_results = []
        self.head_confidences = []
        self.tail_confidences = []

    def __str__(self):
        i = 0
        rep_head = "Heads: "
        for candidate in self.head_results:
            confidence = self.head_confidences[i]
            rep_head += candidate + "\t" + str(confidence) + "\t"
            i += 1
        i = 0
        rep_tail = "Tails: "
        for candidate in self.tail_results:
            confidence = self.tail_confidences[i]
            rep_tail += candidate + "\t" + str(confidence) + "\t"
            i += 1
        return rep_head + "\n" + rep_tail



    def add_head_results(self, heads, k = 0):
        if k > 0:
            self.add_results(heads, self.head_results, k)
        else:
            self.add_results(heads, self.head_results)

    def add_tail_results(self, tails, k = 0):
        if k > 0:
            self.add_results(tails, self.tail_results, k)
        else:
            self.add_results(tails, self.tail_results)

    def add_results(self, candidates, results, k=None):
        for c in candidates:
            if c != "":
                results.append(c)
                if k is not None:
                    k -= 1
                    if k == 0: return

    def add_head_confidences(self, confidences, k = 0):
        if k > 0: self.add_confidences(confidences, self.head_confidences, k)
        else: self.add_confidences(confidences, self.head_confidences)

    def add_tail_confidences(self, confidences, k = 0):
        if k > 0: self.add_confidences(confidences, self.tail_confidences, k)
        else: self.add_confidences(confidences, self.tail_confidences)

    def add_confidences(self,confs, confidences, k = None):
        for d in confs:
            confidences.append(d)
            if k is not None:
                k -= 1
                if k == 0: return

    def add_head_candidate_with_confidence(self, cc):
        self.head_results.append(cc[0])
        self.head_confidences.append(cc[1])

    def add_tail_candidate_with_confidence(self, cc):
        self.tail_results.append(cc[0])
        self.tail_confidences.append(cc[1])


class Ranking():

    def __init__(self, filepath = None, contains_confidences = True, k = 100):
        self.hits = Hits()
        self.filepath = filepath
        self.contains_confidences = contains_confidences
        self.k = k
        self.results = {}
        if filepath == None:
            return
        counter = 0
        stepsize = 100000 
        f = open(self.filepath, "r")
        lines = f.readlines()
        while len(lines) > 1: # there might be a line break after the last line
            line = lines.pop(0)
            # print(line)
            counter += 1
            if counter % stepsize == 0: print("* parsed " + str(counter) + " lines of results file")
            if len(line) < 3: next
            cr = CompletionResult(line)
            head_line = lines.pop(0)
            tail_line = lines.pop(0)
            if head_line.startswith("Tails:"):
                temp_line = head_line
                head_line = tail_line
                tail_line = temp_line
            # print(self.get_results_from_line(head_line[7:]))
            cr.add_head_results(self.get_results_from_line(head_line[7:]), k)
            cr.add_head_confidences(self.get_confidences_from_line(head_line[7:]), k)
            cr.add_tail_results(self.get_results_from_line(tail_line[7:]), k)
            cr.add_tail_confidences(self.get_confidences_from_line(tail_line[7:]), k)
            line = line.replace('\t', ' ')
            self.results[line.strip()] = cr
        f.close()

    def convert_handler_ranking(self, head_ranking, tail_ranking, testset, target_triples = None):
        """
        This function converts the internal rankings computed by PyClause into a ranking
        that can be evaluted using the standard evaluation protocol.
        It is assumed that the rankings of the ranking handler are already filtered against valid and train.
        The additional filtering against the testset is done in this method.

        This function takes as arguments a head and a tail ranking and a testset.
        It is possible to additionally set a list of traget_triples. If this parameter is not set,
        then the triples from the testset are used as target_triples. The traget_triples are used as
        evaluation queries for which the ranking is generated. They are usually a subset or all of the triples of the testset.
        
        The head or the tail ranking can be set to None. In this case empty lists of predictions for the
        head/tail cases are created within the ranking.
        """
        if not target_triples: target_triples = testset.triples
        for triple in target_triples:
            s = triple.get_head_token()
            r = triple.get_relation_token()
            o = triple.get_tail_token()
            # print(str(triple) + " r=" + r)
            cr = CompletionResult(str(triple))
            self.results[str(triple)] = cr
            if not head_ranking == None:
                if r in head_ranking:
                    if o in head_ranking[r]:
                        count = 0
                        for cc in head_ranking[r][o]:
                            if cc[0] == s or not testset.is_true(cc[0],r,o):
                                cr.add_head_candidate_with_confidence(cc)
                                count += 1
                                if count == self.k: break
            if not tail_ranking == None:
                if r in tail_ranking:
                    if s in tail_ranking[r]:
                        count = 0
                        for cc in tail_ranking[r][s]:
                            if cc[0] == o or not testset.is_true(s,r,cc[0]):
                                cr.add_tail_candidate_with_confidence(cc)
                                count += 1
                                if count == self.k: break


    def write(self, filepath):
        f = open(filepath, "w")
        for triple in self.results:
            f.write(triple)
            f.write("\n")
            cr = self.results[triple]
            f.write(str(cr))
            f.write("\n")
        f.close()

    def add_filter_set(self, tripleset):
            self.hits.add_filter_set(tripleset)

    def compute_scores(self, triples, head_direction = True, tail_direction = True):
        self.hits.compute_scores(self, triples, head_direction, tail_direction)

    def get_results_from_line(self, rline):
        if not self.contains_confidences:
            return rline.split("\t")
        else:
            token = rline.split("\t")
            tokenx = [token[i*2] for i in range(len(token) // 2)]
            return tokenx

    def get_confidences_from_line(self, rline):
        if not self.contains_confidences:
            print("there are no confidences, you cannot retrieve them (line: " + rline + ")")
            return None
        else:
            token = rline.split("\t")
            tokenx = [float(token[i*2 + 1]) for i in range(len(token) // 2)]
            return tokenx
    
    def get_head_candidates(self, triple):
        if triple in self.results:
            return self.results[triple].head_results
        else:
            return []

    def get_tail_candidates(self, triple):
        if triple in self.results:
            return self.results[triple].tail_results
        else:
            return []
    
    def diff(self, other):
        """
        This diff has been written for checking the correctness
        of the conversion from internal to python ranking.
        It point to the first difference between self and
        other (another ranking).

        This method might be extended later.
        """
        for triple in self.results:
            cr_self = self.results[triple]
            if not triple in other.results:
                print(">>> found " + triple + " self, that is not in other")
                return
            cr_other = other.results[triple]
            if len(cr_self.head_results) != len(cr_other.head_results):
                print(">>> (" + triple + ") head candidates: self=" + str(len(cr_self.head_results)) + " other=" +  str(len(cr_other.head_results)))
            if len(cr_self.tail_results) != len(cr_other.tail_results):
                print(">>> (" + triple + ") tail candidates: self=" + str(len(cr_self.tail_results)) + " other=" +  str(len(cr_other.tail_results)))
        for triple in other.results:
            if not triple in self.results:
                print(">>> found " + triple + " other, that is not in self")
                return