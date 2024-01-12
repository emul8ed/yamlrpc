#include "zmqtransport.h"
#include "../yamlrpc.h"

#include <iostream>
#include <zmq.hpp>

using namespace yamlrpc;

// -----------------------------------------------------------------------------
void ZmqServer::start(char const *Addr) {
  Socket.bind(Addr);

  ServerThread = std::thread{[this]() {
    try {
      while (true) {
        zmq::message_t Request;

        auto Result = Socket.recv(Request, zmq::recv_flags::none);
        if (!Result) {
          // TODO: Handle this properly
          std::cerr << "Socket.recv returned error: " << Result.value() << "\n";
        }

        auto RequestStr = Request.to_string_view();

        // Request string is zero-terminated
        auto CmdYml = YAML::Load(RequestStr.data());

        auto Response = invokeCommand(CmdYml);

        YAML::Emitter Emitter;
        Emitter << Response;

        Socket.send(zmq::buffer(Emitter.c_str(), Emitter.size() + 1),
                    zmq::send_flags::none);
      }
    } catch (zmq::error_t &Err) {
      if (Err.num() != ETERM) {
        std::cerr << "Caught zmq::error_t: " << Err.what() << "\n";
      }
    }
  }};
}

// -----------------------------------------------------------------------------
void ZmqServer::stop() {
  Context.shutdown();
  ServerThread.join();
}

// -----------------------------------------------------------------------------
auto ZmqServer::invokeEndpoint(YAML::Node &InputNode) -> YAML::Node {
  return invokeCommand(InputNode);
}