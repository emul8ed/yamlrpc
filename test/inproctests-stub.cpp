#include "inproctests.h"
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

void TestRpc::simpleCall1() { SimpleCall1 = true; }

void TestRpc::simpleCall2() { SimpleCall2 = true; }

auto TestRpc::scalarArgs(uint32_t Arg1, uint32_t Arg2) -> uint32_t {
  return Arg1 * Arg2;
}

auto TestRpc::pairArgs(std::pair<uint32_t, uint32_t> Pair)
    -> std::pair<uint32_t, uint32_t> {
  auto [Num, Denom] = Pair;

  return std::make_pair(Num / Denom, Num % Denom);
}

auto TestRpc::tupleArgs(std::tuple<uint32_t, uint32_t, uint32_t> Value)
    -> std::tuple<uint32_t, std::string> {
  auto [a, b, c] = Value;
  uint32_t Result = (a * b) + c;
  std::string Str = std::string("Result: ") + std::to_string(Result);
  return std::make_tuple(Result, Str);
}

auto TestRpc::nestedTupleArgs(
    std::tuple<std::tuple<uint32_t, uint32_t>, std::tuple<float, double>> Value)
    -> std::tuple<std::string, std::tuple<uint64_t, float>> {
      
  auto [Tuple1, Tuple2] = Value;
  auto [Int1, Int2] = Tuple1;
  auto [Float, Double] = Tuple2;

  std::string Str = "A string";
  
  auto InnerTuple = std::tuple<uint64_t, float>(Int1 * Int2, Double / Float);
  
  return std::make_tuple(std::move(Str), InnerTuple);
}

auto TestRpc::customType(Custom Arg) -> Custom {
  return {Arg.Field1 + 111, Arg.Field2 + "->out"};
}

auto makeServerObjects() -> std::unique_ptr<ServerObjStorage> {
  return std::make_unique<ServerObjStorage>();
}

void bindAll(ServerObjStorage &Storage) {
  Storage.RpcObj.simpleCall.bind(Storage.Stub, &TestRpc::simpleCall1);
  Storage.RpcObj.scalarArgs.bind(Storage.Stub, &TestRpc::scalarArgs);
  Storage.RpcObj.pairArgs.bind(Storage.Stub, &TestRpc::pairArgs);
  Storage.RpcObj.tupleArgs.bind(Storage.Stub, &TestRpc::tupleArgs);
  Storage.RpcObj.nestedTupleArgs.bind(Storage.Stub, &TestRpc::nestedTupleArgs);
  Storage.RpcObj.customType.bind(Storage.Stub, &TestRpc::customType);
}