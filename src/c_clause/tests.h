#include <iostream>
#include <string>
#include <memory>
#include <omp.h>
#include <chrono>
#include <numeric>

#include "core/Index.h"
#include "core/TripleStorage.h"
#include "core/Rule.h"
#include "core/RuleStorage.h"
#include "features/Application.h"
#include "core/QueryResults.h"
#include "core/RuleFactory.h"
#include "core/Types.h"



int main();
void test_compute_strings();
void tests_groundings();
void tests();
void testTripleScoring();
