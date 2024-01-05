#include "../yamlrpc.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>

using namespace yamlrpc;

struct Custom {
    uint32_t Field1;
    std::string Field2;
};

struct TestRpc {
  void simpleCall1() {
    SimpleCall1 = true;
  }
  void simpleCall2() {
    SimpleCall2 = true;
  }

  Custom customType(Custom Arg) {
    return {Arg.Field1 + 111, Arg.Field2 + "->out"};
  }

  bool SimpleCall1 = false;
  bool SimpleCall2 = false;
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

struct TestRpcObject : public RpcObject {
  TestRpcObject(RpcTransport &Transport) : RpcObject(Transport) {}

  Command<void> simpleCall{this};
  Command<Custom, Custom> customType{this};
};

class Inproc : public testing::Test {
protected:
  InprocServer Server{};
  InprocClient Client{Server};

  TestRpcObject ServerObj {Server};
  TestRpcObject ClientObj {Client};
  TestRpc Impl;
};

TEST_F(Inproc, Simple) {
  ServerObj.simpleCall.bind(Impl, &TestRpc::simpleCall1);
  ASSERT_FALSE(Impl.SimpleCall1);
  ClientObj.simpleCall();
  ASSERT_TRUE(Impl.SimpleCall1);
}

TEST_F(Inproc, Rebind) {
  ASSERT_FALSE(Impl.SimpleCall1);
  ASSERT_FALSE(Impl.SimpleCall2);
  ServerObj.simpleCall.bind(Impl, &TestRpc::simpleCall1);

  ClientObj.simpleCall();
  ASSERT_TRUE(Impl.SimpleCall1);
  ASSERT_FALSE(Impl.SimpleCall2);

  Impl.SimpleCall1 = false;
  ServerObj.simpleCall.bind(Impl, &TestRpc::simpleCall2);

  ClientObj.simpleCall();
  ASSERT_FALSE(Impl.SimpleCall1);
  ASSERT_TRUE(Impl.SimpleCall2);
}

TEST_F(Inproc, ScalarArg) {
    // TODO
}

TEST_F(Inproc, VectorArg) {
    // TODO
}

TEST_F(Inproc, MultipleArgs) {
    // TODO
}

TEST_F(Inproc, CustomArg) {
    ServerObj.customType.bind(Impl, &TestRpc::customType);

    Custom Out = ClientObj.customType({100, "In"});
    ASSERT_EQ(Out.Field1, 211);
    ASSERT_EQ(Out.Field2, "In->out");
}

TEST_F(Inproc, Unbound) {
  ASSERT_FALSE(Impl.SimpleCall1);

  EXPECT_THROW(ClientObj.simpleCall(), std::bad_function_call);
}

TEST_F(Inproc, SimpleCallOnServerObject) {
  ServerObj.simpleCall.bind(Impl, &TestRpc::simpleCall1);
  EXPECT_THROW(ServerObj.simpleCall(), RpcError);
}

TEST_F(Inproc, BindOnClientObject) {
  EXPECT_THROW(ClientObj.simpleCall.bind(Impl, &TestRpc::simpleCall1), RpcError);
}

int main(int Argc, char **Argv) {
  testing::InitGoogleTest(&Argc, Argv);

  return RUN_ALL_TESTS();
}