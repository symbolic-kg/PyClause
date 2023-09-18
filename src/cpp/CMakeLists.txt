cmake_minimum_required(VERSION 3.0.0)
project(rule_backend VERSION 0.1.0 LANGUAGES C CXX)
add_compile_options("-fopenmp")

include(CTest)
enable_testing()




set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)



add_library(
    rules_backend SHARED core/myClass.cpp core/Index.cpp core/Rule.cpp core/TripleStorage.cpp
    core/Util.hpp core/RuleStorage.cpp core/Globals.cpp features/Application.cpp Api.cpp core/QueryResults.cpp
    core/RuleFactory.cpp
)

add_executable(main main.cpp)
# link rules_backend library into the main executable using modern signature
target_link_libraries(main PRIVATE rules_backend)

# Add OpenMP support
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(rules_backend PUBLIC OpenMP::OpenMP_CXX)
    target_link_libraries(main PRIVATE OpenMP::OpenMP_CXX)
endif()


