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

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(VCPKG_MANIFEST_DIR ${CMAKE_CURRENT_BINARY_DIR})
if (NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  set(ENV{VCPKG_FORCE_SYSTEM_BINARIES} ON)
endif()

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "")

project(test-cpp)

find_package(yaml-cpp REQUIRED)

add_library(yamlrpc INTERFACE)
target_link_libraries(yamlrpc INTERFACE yaml-cpp::yaml-cpp)

if (YAMLRPC_ENABLE_TESTING)
  add_subdirectory(test)
endif()

file(CREATE_LINK ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/compile_commands.json SYMBOLIC)
