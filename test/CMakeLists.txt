enable_testing()

add_executable(test-yamlrpc
  inproctests.cpp
  inproctests-stub.cpp
)

find_package(GTest REQUIRED)
find_package(cppzmq REQUIRED)

target_link_libraries(test-yamlrpc PRIVATE
  cppzmq-static
  GTest::gtest
  yamlrpc
)

add_test(NAME test-yamlrpc
  COMMAND $<TARGET_FILE:test-yamlrpc>
  )