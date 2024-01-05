enable_testing()

find_package(GTest REQUIRED)
find_package(cppzmq REQUIRED)

add_executable(test-yamlrpc
  inproctests.cpp
  inproctests-stub.cpp
)

set_target_properties(test-yamlrpc PROPERTIES CXX_STANDARD 17)

target_link_libraries(test-yamlrpc PRIVATE
  cppzmq-static
  GTest::gtest
  yamlrpc
)

add_test(NAME test-yamlrpc
  COMMAND $<TARGET_FILE:test-yamlrpc>
  )