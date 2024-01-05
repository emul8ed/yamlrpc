cmake_minimum_required(VERSION 3.22)

option(YAMLRPC_ENABLE_TESTING "Enable tests" ON)
option(YAMLRPC_ENABLE_ZMQ_TESTS "Enable zeromq-based tests" ON)

if (YAMLRPC_ENABLE_TESTING AND YAMLRPC_ENABLE_ZMQ_TESTS)
  set(YAMLRPC_VCPKG_JSON_CPPZMQ [["cppzmq",]])
endif()

if (YAMLRPC_ENABLE_TESTING)
  set(YAMLRPC_VCPKG_JSON_GTEST [["gtest",]])
endif()

configure_file(vcpkg.json.in vcpkg.json @ONLY)

set(VCPKG_MANIFEST_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# TODO: make this non-x64 only
set(ENV{VCPKG_FORCE_SYSTEM_BINARIES} ON)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "")

project(test-cpp)

find_package(yaml-cpp REQUIRED)

add_library(yamlrpc INTERFACE)
target_link_libraries(yamlrpc INTERFACE yaml-cpp::yaml-cpp)

add_executable(test-cpp
  stub.cpp
  skel.cpp
  )

set_target_properties(test-cpp PROPERTIES CXX_STANDARD 20)
target_link_libraries(test-cpp PRIVATE yamlrpc)

if (YAMLRPC_ENABLE_TESTING)
  add_subdirectory(test)
endif()

file(CREATE_LINK ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/compile_commands.json SYMBOLIC)
