import time
import c_clause



start = time.time()

train = "/home/patrick/Desktop/PyClause/data/wnrr/train.txt"
filter = "/home/patrick/Desktop/PyClause/data/wnrr/valid.txt"
target = "/home/patrick/Desktop/PyClause/data/wnrr/test.txt"


rules = "/home/patrick/Desktop/PyClause/data/wnrr/anyburl-rules-c5-3600"
ranking_file = "/home/patrick/Desktop/PyClause/data/wnrr/rankingFile.txt"


#### Calculate exact rule statistics through materialization
# calcStats returns list[num_pred, num_correct_pred]

handler = c_clause.RuleHandler(train)
print(handler.calcStats("_has_part(X,Y) <= _has_part(X,A), _member_of_domain_region(A,B), _member_of_domain_region(Y,B)"))
print(handler.calcStats("_hypernym(X,06355894) <= _synset_domain_topic_of(X,A), _synset_domain_topic_of(06355894,A)"))

testHandler = c_clause.RuleHandler(target)
print(testHandler.calcStats("_hypernym(X,06355894) <= _synset_domain_topic_of(X,A), _synset_domain_topic_of(06355894,A)"))

exit()





#### Calculate a ranking and serialize / use in python

ranker = c_clause.RankingHandler()
options = {"option1":"value1", "option2":"value2"}
ranker.calculateRanking(target, train, filter, rules, ranking_file, options)


rankingtime = time.time()
headRanking = ranker.getRanking("head")
tailRanking = ranker.getRanking("tail")
serializeTime = time.time()



print(f"all time: {serializeTime-start}")
print(f"ranking time: {rankingtime-start}")
print(f"serialize time: {serializeTime-rankingtime}")


