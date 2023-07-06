import python_example
import timeit
import numpy as np
import rulebackend

def add(a,b):
    return a+b

def sumRange(a,b):
    sum_ = 0
    for i in range(a,b+1):
        sum_+=i
    return sum_
        


print(add(1,2))

n1 = 10000000000
n2 = 200000000000

#print(timeit.timeit(lambda: add(n1,n2)))
#print(timeit.timeit(lambda: python_example.add(1,2)))
print(python_example.add(2,3))
obj = python_example.myClass("peter")
obj.addOne()
print(obj.getNumber())

print(sumRange(1,100000))
print(obj.sumRange(1,100000))

start = 1
end = 100000
print(timeit.timeit(lambda: sumRange(start,end), number=100))
print(timeit.timeit(lambda: obj.sumRange(start,end),number=100))
print(timeit.timeit(lambda: np.sum(np.arange(start,end)),number=100))

data = rulebackend.TripleStorage({"a":2})

handler = handlerObject("peter")
triples = handler.trainset
## handler HOLDS DATASET
prediction = handler.materialize_rule(rule_i) 



