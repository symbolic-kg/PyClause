from triples import Triple, TripleSet
import time



x = {}
x["b"] = {}
x["b"]["timeout"] = 11
start = time.time()
for i in range(0,10000):
    a = 0
    b = 0
    for j in range(0,10000):
        current = time.time()
        if current-start > x["b"]["timeout"] :
            b = 11
        # if b % 100 == 0:
        #     pass
        a += 1
stop = time.time()

print(str(stop-start))