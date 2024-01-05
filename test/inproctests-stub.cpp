#include "inproctests.h"

void TestRpc::simpleCall1() {
  SimpleCall1 = true;
}

void TestRpc::simpleCall2() {
  SimpleCall2 = true;
}

auto TestRpc::scalarArgs(uint32_t Arg1, uint32_t Arg2) -> uint32_t {
  return Arg1 * Arg2;
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
  Storage.RpcObj.customType.bind(Storage.Stub, &TestRpc::customType);
}