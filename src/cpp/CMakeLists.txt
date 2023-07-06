cmake_minimum_required(VERSION 3.0.0)
project(rule_backend VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()




set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)



add_library(rules_backend SHARED myClass.cpp Index.cpp Rule.cpp TripleStorage.cpp Util.hpp)

add_executable(main main.cpp)
# link the 'my_program_code' library into the main executable
target_link_libraries(main rules_backend)


