#include "yamlrpc.h"

#include <iostream>
#include <string>
#include <vector>

using namespace yamlrpc;

Skel Skeleton;

class ZmqInterface : public RpcTransport {
public:
  virtual auto invokeEndpoint(YAML::Node &InputNode) -> YAML::Node { return {}; }
};

class InprocInterface : public RpcTransport {
public:
  virtual auto invokeEndpoint(YAML::Node &InputNode) -> YAML::Node { return {}; }
};

class MyRpcInterface : RpcObject {
public:
  MyRpcInterface(RpcTransport &Transport) : RpcObject(Transport) {}

  Command<void, std::vector<std::string> const &> theMethod{this};
  Command<void> voidArgsMethod{this};
  Command<std::string, std::vector<std::string> const &> returningMethod{
      this};
  Command<std::vector<std::string>, std::vector<std::string> const &>
      sortVector{this};
};

int main(int, char**) {
    std::vector<std::string> VIn = {"c", "b", "a"};
    std::cout << "Address of Badger: " << std::hex << &VIn << "\n";
    std::cout << "Address of string: " << std::hex << &VIn[0] << "\n";
    InprocServer Server;
    InprocClient Client {Server};

    MyRpcInterface ServerObj {Server};
    ServerObj.theMethod.bind(Skeleton, &Skel::theMethod);
    ServerObj.returningMethod.bind(Skeleton, &Skel::returningMethod);
    ServerObj.sortVector.bind(Skeleton, &Skel::sortVector);

    MyRpcInterface ClientObj {Client};
    ClientObj.theMethod(VIn);
    std::string retVal = ClientObj.returningMethod(VIn);
    std::cout << "returningMethod returned '" << retVal << "'\n";
    auto sortedVec = ClientObj.sortVector(VIn);
    for (auto const &Str : sortedVec) {
      std::cout << Str << "\n";
    }

    return 0;
}