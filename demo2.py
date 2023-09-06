import time
import c_clause



start = time.time()

train = "/home/patrick/Desktop/PyClause/data/wnrr/train.txt"
filter = "/home/patrick/Desktop/PyClause/data/wnrr/valid.txt"
target = "/home/patrick/Desktop/PyClause/data/wnrr/test.txt"


rules = "/home/patrick/Desktop/PyClause/data/wnrr/anyburl-rules-c5-3600"
ranking_file = "/home/patrick/Desktop/PyClause/data/wnrr/rankingFile.txt"

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


