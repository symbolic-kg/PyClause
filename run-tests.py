import time
import c_clause
from clause.tests.test import test_uc_b_zero_ranking, test_237_all_ranking



try:
    test_uc_b_zero_ranking()
except Exception as e:
    print(e)

try:
    test_237_all_ranking()
except Exception as e:
    print(e)


