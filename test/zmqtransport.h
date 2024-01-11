#include "../yamlrpc.h"

#include <atomic>
#include <thread>

#include <yaml-cpp/yaml.h>
#include <zmq.hpp>

namespace yr = yamlrpc;

// -----------------------------------------------------------------------------
class ZmqServer : public yr::RpcServer {
public:
  void start(char const *Addr);

  void stop();

  auto invokeEndpoint(YAML::Node &InputNode) -> YAML::Node override;

private:
  std::thread ServerThread;
  zmq::context_t Context{1};
  zmq::socket_t Socket{Context, zmq::socket_type::rep};
};

// -----------------------------------------------------------------------------
class ZmqClient : public yr::RpcClient {
public:
  void connect(char const *Addr) { Socket.connect(Addr); }

  auto invokeEndpoint(YAML::Node &InputNode) -> YAML::Node override;

private:
  zmq::context_t Context{1};
  zmq::socket_t Socket{Context, zmq::socket_type::req};
};
