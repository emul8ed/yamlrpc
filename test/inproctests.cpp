#include "../yamlrpc.h"
#include "inproctests.h"

#include <gtest/gtest.h>

#include <string>

using namespace yamlrpc;

class Inproc : public testing::Test {
protected:
  Inproc() : ServerObjects(makeServerObjects()) {
  }

  void bindAll() {
    ::bindAll(*ServerObjects);
  }

  std::unique_ptr<ServerObjStorage> ServerObjects;
  InprocClient Client{ServerObjects->Server};

  TestRpcObject ClientObj {Client};
};

#define BIND_TARGET(Command, TargetFn) \
  ServerObjects->RpcObj.Command.bind(ServerObjects->Stub, &TestRpc::TargetFn)

#define BIND(Command) \
  BIND_TARGET(Command, Command)

TEST_F(Inproc, Simple) {
  bindAll();

  ASSERT_FALSE(ServerObjects->Stub.SimpleCall1);
  ClientObj.simpleCall();
  ASSERT_TRUE(ServerObjects->Stub.SimpleCall1);
}

TEST_F(Inproc, Rebind) {
  ASSERT_FALSE(ServerObjects->Stub.SimpleCall1);
  ASSERT_FALSE(ServerObjects->Stub.SimpleCall2);
  BIND_TARGET(simpleCall, simpleCall1);

  ClientObj.simpleCall();
  ASSERT_TRUE(ServerObjects->Stub.SimpleCall1);
  ASSERT_FALSE(ServerObjects->Stub.SimpleCall2);

  ServerObjects->Stub.SimpleCall1 = false;
  BIND_TARGET(simpleCall, simpleCall2);

  ClientObj.simpleCall();
  ASSERT_FALSE(ServerObjects->Stub.SimpleCall1);
  ASSERT_TRUE(ServerObjects->Stub.SimpleCall2);
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
  bindAll();

  Custom Out = ClientObj.customType({100, "In"});
  ASSERT_EQ(Out.Field1, 211);
  ASSERT_EQ(Out.Field2, "In->out");
}

TEST_F(Inproc, Unbound) {
  ASSERT_FALSE(ServerObjects->Stub.SimpleCall1);

  EXPECT_THROW(ClientObj.simpleCall(), std::bad_function_call);
}

TEST_F(Inproc, SimpleCallOnServerObject) {
  bindAll();

  EXPECT_THROW(ServerObjects->RpcObj.simpleCall(), RpcError);
}

TEST_F(Inproc, BindOnClientObject) {
  EXPECT_THROW(
      ClientObj.simpleCall.bind(ServerObjects->Stub, &TestRpc::simpleCall1),
      RpcError);
}

int main(int Argc, char **Argv) {
  testing::InitGoogleTest(&Argc, Argv);

  return RUN_ALL_TESTS();
}