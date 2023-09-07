import time
import rules_c



start = time.time()

train = "../eclipse-workspace/AnyBURL/data/WN18RR/train.txt"
filter = "../eclipse-workspace/AnyBURL/data/WN18RR/valid.txt"
target = "../eclipse-workspace/AnyBURL/data/WN18RR/test.txt"


rules = "../eclipse-workspace/AnyBURL-23-3/exp/torm/rules-torm-wn18rr-b5-s2"
ranking_file = "../eclipse-workspace/AnyBURL-23-3/exp/torm/ranking-torm-pyclause"

ranker = rules_c.RuleHandler()
options = {"option1":"value1", "option2":"value2"}
ranker.calculateRanking(target, train, filter, rules, ranking_file, options)


rankingtime = time.time()
headRanking = ranker.getRanking("head")
tailRanking = ranker.getRanking("tail")
serializeTime = time.time()



print(f"all time: {serializeTime-start}")
print(f"ranking time: {rankingtime-start}")
print(f"serialize time: {serializeTime-rankingtime}")


