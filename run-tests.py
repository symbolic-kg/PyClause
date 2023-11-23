import time
import c_clause
from clause.tests.test import test_uc_b_zero_ranking, test_237_all_ranking, test_qa_handler,\
test_loader, test_triple_scoring, test_explanation_tracking

count_true = 0
num_tests = 0

try:
    test_loader()
    count_true += 1
    num_tests +=1
except Exception as e:
    print("Test loader failed")
    num_tests +=1
    print(e)
try:
    test_qa_handler()
    count_true +=1
    num_tests +=1
except Exception as e:
    num_tests +=1
    print(e)
try:
    test_uc_b_zero_ranking()
    count_true +=1
    num_tests +=1
except Exception as e:
    num_tests+=1
    print(e)

try:
    test_237_all_ranking()
    count_true += 1
    num_tests +=1
except Exception as e:
    num_tests +=1
    print(e)
try:
    test_triple_scoring()
    count_true += 1
    num_tests +=1
except Exception as e:
    print("Triple scoring test failed")
    num_tests +=1
    print(e)
try:
    test_explanation_tracking()
    count_true += 1
    num_tests +=1
except Exception as e:
    print("Explanation tracking failed")
    num_tests +=1
    print(e)



print(f"{count_true} of {num_tests} tests were successful.")



