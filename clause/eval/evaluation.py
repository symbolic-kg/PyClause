import sys
import os
sys.path.append(os.getcwd())

from clause.data.triples import TripleSet

class Hits():
    """
    Used to compute MRR and Hits@k metrics.
    """

    def __init__(self):
        self.filter_sets = [] # triples sets used for filtering
        self.ATKMAX = 100 # might not be required in the python version
        self.init()


    def init(self):
        self.tail_ranks = []
        self.hits_ad_n_tail = [0] * self.ATKMAX
        self.hits_ad_n_tail_filtered = [0] * self.ATKMAX
        self.counter_tail = 0
        self.counter_tail_covered = 0
        self.head_ranks = []
        self.hits_ad_n_head = [0] * self.ATKMAX
        self.hits_ad_n_head_filtered = [0] * self.ATKMAX
        self.counter_head = 0
        self.counter_head_covered = 0
    
    def add_filter_set(self, tripleset):
        self.filter_sets.append(tripleset)

   
    
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
        Computes and returns the filtered MRR.
        """
        mrr = 0.0
        hk = 0.0
        hk_prev = 0.0
        hk_diff = 0.0
        for k in range(self.ATKMAX):
            hk = (self.hits_ad_n_head_filtered[k] + self.hits_ad_n_tail_filtered[k]) / (self.counter_head + self.counter_tail)
            hk_diff = hk - hk_prev
            mrr += hk_diff * (1.0 / (k+1))
            hk_prev = hk
        return mrr
    
    def get_mrr_head(self):
        """
        Computes and returns the filtered head MRR.
        """
        mrr = 0.0
        hk = 0.0
        hk_prev = 0.0
        hk_diff = 0.0
        for k in range(self.ATKMAX):
            hk = self.hits_ad_n_head_filtered[k] / self.counter_head
            hk_diff = hk - hk_prev
            mrr += hk_diff * (1.0 / (k+1))
            hk_prev = hk
        return mrr
    
    def get_mrr_head(self):
        """
        Computes and returns the filtered tail MRR.
        """
        mrr = 0.0
        hk = 0.0
        hk_prev = 0.0
        hk_diff = 0.0
        for k in range(self.ATKMAX):
            hk = self.hits_ad_n_tail_filtered[k] / self.counter_tail
            hk_diff = hk - hk_prev
            mrr += hk_diff * (1.0 / (k+1))
            hk_prev = hk
        return mrr

    def get_hits_at_k(self, k):
        k = k -1
        return (self.hits_ad_n_head_filtered[k] + self.hits_ad_n_tail_filtered[k]) / (self.counter_head + self.counter_tail)

    def evaluate_head(self, candidates, triple):
        (t_head, t_relation, t_tail) = triple.split()
        foundAt = -1
        self.counter_head += 1
        if len(candidates) > 0: self.counter_head_covered += 1
        filterCount = 0
        for rank in range(len(candidates)):
            if rank < self.ATKMAX:
                candidate = candidates[rank]
                if candidate == t_head:
                    for index in range(rank, rank - filterCount + self.ATKMAX):
                        # print("i=" + str(index))
                        if index < self.ATKMAX:
                            self.hits_ad_n_head[index] += 1
                            self.hits_ad_n_head_filtered[index - filterCount] += 1
                    foundAt = rank + 1
                    break
                else:
                    for filterSet in self.filter_sets:
                        if filterSet.is_true(candidate, t_relation, t_tail):
                            filterCount += 1
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
        filterCount = 0
        for rank in range(len(candidates)):
            if rank < self.ATKMAX:
                candidate = candidates[rank]
                if candidate == t_tail:
                    for index in range(rank, rank - filterCount + self.ATKMAX):
                        if index < self.ATKMAX:
                            self.hits_ad_n_tail[index] += 1
                            self.hits_ad_n_tail_filtered[index - filterCount] += 1
                    foundAt = rank + 1
                    break
                else:
                    for filterSet in self.filter_sets:
                        if filterSet.is_true(t_head, t_relation, candidate):
                            filterCount += 1
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
        that can be evaluted using the standard evaluatiobn protooll in the filtered version.
        The interla rankings are usually filtered against valid and train. The additioal
        filtering against the testset is done in this method.

        This function takes as arguments a head and a tail ranking and a testset.
        It is possible to additionally set a list of traget_triples. If this parameter is not set,
        then the triples from the testset are used as target_triples. The traget_triples are used as
        evaluation queries for which the ranking is generated. They are usually a subset or all of the triples of the testset.
        
        The head or the tail ranking can be set to None. In this case empty lists of predictions for the
        head/tail cases are created within the ranking.
        """
        if target_triples == None: target_triples = testset.triples
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
                        for cc in head_ranking[r][o]:
                            cr.add_head_candidate_with_confidence(cc)
            if not tail_ranking == None:
                if r in tail_ranking:
                    if s in tail_ranking[r]:
                        for cc in tail_ranking[r][s]:
                            cr.add_tail_candidate_with_confidence(cc)

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








if __name__ == '__main__':

    ranking = Ranking("local/anyburl-rules-c5-3600-analysis/pyclause-fix6/preds-ALL")


    testset = TripleSet("data/wnrr/test.txt")
    hits = Hits()


    hits.compute_scores(ranking, testset.triples, True, True)

    print("MRR: " + str(hits.get_mrr()))

    # next steps: 
    # 1) check and/or integrate filtering
    # 2) check correctness of numbers via an anyburl comparison
    # 3) think about the hits.reset vs always generate and return a new object
    # 4) write nice piece of code that allows you to compute tail and head specific mrr for each relation of fb237 for the top 10 relations
    # 5) talk to patrick w.r.t integration of his stuff (forgot what he wanted ...)
        # rabking constrcutor fpr patrick ranking
        # add testset to read from file ranking constructor








