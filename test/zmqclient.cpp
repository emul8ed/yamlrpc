#include "zmqtransport.h"

auto ZmqClient::invokeEndpoint(YAML::Node &InputNode) -> YAML::Node {
  YAML::Emitter Emitter;
  Emitter << InputNode;

  Socket.send(zmq::buffer(Emitter.c_str(), Emitter.size() + 1),
              zmq::send_flags::none);

  zmq::message_t Reply{};

  if (!Socket.recv(Reply, zmq::recv_flags::none)) {
      throw yamlrpc::RpcError("No reply received");
  }

  return YAML::Load(reinterpret_cast<char const *>(Reply.data()));
}
