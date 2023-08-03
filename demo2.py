import time
import rules_c



start = time.time()

train = "/home/patrick/Desktop/PyClause/data/fb15k-237/train.txt"
filter = "/home/patrick/Desktop/PyClause/data/fb15k-237/valid.txt"
target = "/home/patrick/Desktop/PyClause/data/fb15k-237/train.txt"


rules = "/home/patrick/Desktop/PyClause/data/fb15k-237/safran-rules-1000"
ranking_file = "/home/patrick/Desktop/PyClause/data/fb15k-237/firstRankingChristian.txt"

ranker = rules_c.RuleHandler()
ranker.calculateRanking(target, train, filter, rules, ranking_file)


rankingtime = time.time()
headRanking = ranker.getRanking("head")
tailRanking = ranker.getRanking("tail")
serializeTime = time.time()



print(f"all time: {serializeTime-start}")
print(f"ranking time: {rankingtime-start}")
print(f"serialize time: {serializeTime-rankingtime}")


