enable_testing()

find_package(GTest REQUIRED)
find_package(cppzmq REQUIRED)

add_library(test-yamlrpc INTERFACE)

target_sources(test-yamlrpc INTERFACE
  inproctests-stub.cpp
)

target_compile_options(test-yamlrpc INTERFACE
  -Wall
  -Wextra
  -Werror
)

target_link_libraries(test-yamlrpc INTERFACE
  GTest::gtest
  yamlrpc
)

add_executable(test-yamlrpc-inproc
  inproctests.cpp
)

set_target_properties(test-yamlrpc-inproc PROPERTIES CXX_STANDARD 17)

target_link_libraries(test-yamlrpc-inproc PRIVATE
  cppzmq-static
  test-yamlrpc
)

add_test(NAME test-yamlrpc-inproc
  COMMAND $<TARGET_FILE:test-yamlrpc-inproc>
)

add_executable(test-yamlrpc-zmq
  inproctests.cpp
  zmqclient.cpp
  zmqserver.cpp
)

target_compile_definitions(test-yamlrpc-zmq PRIVATE
  ZMQ_TRANSPORT=1
)

set_target_properties(test-yamlrpc-zmq PROPERTIES
  CXX_STANDARD 17
)

target_link_libraries(test-yamlrpc-zmq PRIVATE
  test-yamlrpc
  cppzmq-static
)

add_test(NAME test-yamlrpc-zmq
  COMMAND $<TARGET_FILE:test-yamlrpc-zmq>
  )