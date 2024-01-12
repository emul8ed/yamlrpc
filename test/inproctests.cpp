#include "../yamlrpc.h"

#include "inproctests.h"

#include <gtest/gtest.h>

#include <functional>
#include <memory>
#include <utility>

#include <unistd.h>

using namespace yamlrpc;

class Inproc : public testing::Test {
protected:
  Inproc() : ServerObjects(makeServerObjects()) {}

  void bindAll() { ::bindAll(*ServerObjects); }

  void SetUp() override;
  void TearDown() override;
  
  std::unique_ptr<ServerObjStorage> ServerObjects;
#if defined(ZMQ_TRANSPORT)
  ZmqClient Client;
#else // defined(ZMQ_TRANSPORT)
  InprocClient Client{ServerObjects->Server};
#endif // defined(ZMQ_TRANSPORT)

  TestRpcObject ClientObj{Client};
};

void Inproc::SetUp() {
#if defined(ZMQ_TRANSPORT)
  char Path[1024];
  sprintf(Path, "ipc:///tmp/test-yamlrpc-zmq-%u.sock", getpid());
  ServerObjects->Server.start(Path);
  Client.connect(Path);
#endif // defined(ZMQ_TRANSPORT)
}

void Inproc::TearDown() {
#if defined(ZMQ_TRANSPORT)
  ServerObjects->Server.stop();
#endif // defined(ZMQ_TRANSPORT)
}

#define BIND_TARGET(Command, TargetFn)                                         \
  ServerObjects->RpcObj.Command.bind(ServerObjects->Stub, &TestRpc::TargetFn)

#define BIND(Command) BIND_TARGET(Command, Command)

TEST_F(Inproc, Simple) {
  bindAll();

  ASSERT_FALSE(ServerObjects->Stub.SimpleCall1);
  ClientObj.simpleCall();
  ASSERT_TRUE(ServerObjects->Stub.SimpleCall1);
}

TEST_F(Inproc, SimpleConst) {
  bindAll();

  uint32_t Value = ClientObj.simpleConstCall();
  ASSERT_EQ(Value, 123456);
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
  bindAll();

  auto Result = ClientObj.scalarArgs(10U, 20U);
  ASSERT_EQ(Result, 10U * 20U);
}

TEST_F(Inproc, ContainerArgs) {
  // TODO
}

/// @test Verify serialize/deserialize of std::pair arg/return
TEST_F(Inproc, PairArgs) {
  bindAll();

  auto [Div, Rem] = ClientObj.pairArgs(std::make_pair(7U, 2U));

  ASSERT_EQ(Div, 3);
  ASSERT_EQ(Rem, 1);
}

TEST_F(Inproc, TupleArgs) {
  bindAll();

  auto [ResultInt, ResultStr] = ClientObj.tupleArgs(std::make_tuple(4U, 5U, 6U));

  ASSERT_EQ(ResultInt, (4U * 5U) + 6U);
  ASSERT_EQ(ResultStr, "Result: 26");
}

TEST_F(Inproc, NestedTupleArgs) {
  bindAll();

  auto [ResultStr, ResultTuple] = ClientObj.nestedTupleArgs(std::make_tuple(
      std::make_tuple(11U, 34U),
      std::make_tuple(
          1.11f,
          2.34))); // yr::Command<std::tuple<std::string, std::tuple<uint64_t,
                   // float>>, std::tuple<std::tuple<uint32_t, uint32_t>,
                   // std::tuple<float, double>>>

  ASSERT_EQ(ResultStr, "A string");
  
  auto [ResultInt, ResultFloat] = ResultTuple;
  ASSERT_EQ(ResultInt, 11U * 34U);
  ASSERT_EQ(ResultFloat, static_cast<float>(2.34 / 1.11f));
}

// TODO: nested tuple
// TODO: custom type with/without copy/move
// TODO: verify mismatched types in arg pair/tuple

TEST_F(Inproc, CustomArg) {
  bindAll();

  Custom Out = ClientObj.customType({100, "In"});
  ASSERT_EQ(Out.Field1, 211);
  ASSERT_EQ(Out.Field2, "In->out");
}

// TODO: handle this in zmq transport
#if !defined(ZMQ_TRANSPORT)
TEST_F(Inproc, Unbound) {
  ASSERT_FALSE(ServerObjects->Stub.SimpleCall1);

  EXPECT_THROW(ClientObj.simpleCall(), std::bad_function_call);
}
#endif // !defined(ZMQ_TRANSPORT)

TEST_F(Inproc, SimpleCallOnServerObject) {
  bindAll();

  EXPECT_THROW(ServerObjects->RpcObj.simpleCall(), RpcError);
}

TEST_F(Inproc, BindOnClientObject) {
  EXPECT_THROW(
      ClientObj.simpleCall.bind(ServerObjects->Stub, &TestRpc::simpleCall1),
      RpcError);
}

auto main(int Argc, char **Argv) -> int {
  testing::InitGoogleTest(&Argc, Argv);

  return RUN_ALL_TESTS();
}