import timeit
import numpy as np
import c_clause


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

print(timeit.timeit(lambda: add(n1,n2)))
print(timeit.timeit(lambda: rules_c.add(1,2)))
print(c_clause.add(2,3))
obj = c_clause.myClass("peter")
obj.addOne()
print(obj.getNumber())

print(sumRange(1,1000))
print(obj.sumRange(1,1000))

start = 1
end = 100
print("Python:")
print(timeit.timeit(lambda: sumRange(start,end), number=100))
print("c++:")
print(timeit.timeit(lambda: obj.sumRange(start,end),number=100))
print("Numpy:")
print(timeit.timeit(lambda: np.sum(np.arange(start,end)),number=100))



