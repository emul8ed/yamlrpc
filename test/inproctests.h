#pragma once

#include "../yamlrpc.h"

#include <cstdint>
#include <memory>
#include <string>

struct Custom {
    uint32_t Field1;
    std::string Field2;
};

namespace yamlrpc {

template <> struct Serializer<Custom> {
  using TStorage = Custom;

  static auto deserialize(YAML::Node Node) {
    Custom Result;
    Result.Field1 = Node["Field1"].as<typeof(Result.Field1)>();
    Result.Field2 = Node["Field2"].as<typeof(Result.Field2)>();
    return Result;
  }

  static auto serialize(Custom const &Value) {
    YAML::Node Node{YAML::NodeType::Map};
    Node["Field1"] = Value.Field1;
    Node["Field2"] = Value.Field2;
    return Node;
  }
};

} // namespace yamlrpc

struct TestRpcObject : public yamlrpc::RpcObject {
  TestRpcObject(yamlrpc::RpcTransport &Transport) : yamlrpc::RpcObject(Transport) {}

  yamlrpc::Command<void> simpleCall{this};
  yamlrpc::Command<uint32_t, uint32_t, uint32_t> scalarArgs{this};
  yamlrpc::Command<Custom, Custom> customType{this};
};

struct TestRpc {
  void simpleCall1();

  void simpleCall2();

  auto customType(Custom Arg) -> Custom;
  
  auto scalarArgs(uint32_t Arg1, uint32_t Arg2) -> uint32_t;

  bool SimpleCall1 = false;
  bool SimpleCall2 = false;
};

struct ServerObjStorage {
  yamlrpc::InprocServer Server;
  TestRpcObject RpcObj {Server};
  TestRpc Stub;
};

auto makeServerObjects() -> std::unique_ptr<ServerObjStorage>;
void bindAll(ServerObjStorage &Storage);