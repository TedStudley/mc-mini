# Specify .cpp files
set(TARGET_NAME ${PROJECT_NAME})
set(SRC
  debug/backtrace.cpp

  geometry/geometry.cpp

  matrixForms/denseForms.cpp
  matrixForms/sparseForms.cpp

  output/output.cpp

  params.cpp
  params/paramParser.cpp
  params/paramTree.cpp

  problem/advection.cpp
  problem/diffusion.cpp
  problem/initialization.cpp
  problem/problem.cpp
  problem/solveRoutines.cpp)

# Build a library from all specified source files
# This is required for using Google Test
add_library(${PROJECT_NAME}-lib ${SRC})
target_link_libraries(${PROJECT_NAME}-lib ${LIBRARIES})
set_target_properties(${PROJECT_NAME}-lib PROPERTIES OUPTUT_NAME ${PROJECT_NAME})

# Build an executable from main.cpp and all the specified source files
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${TARGET_NAME} ${LIBRARIES} ${PROJECT_NAME}-lib)
