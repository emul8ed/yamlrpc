/**
MIT License

Copyright (c) 2024 Henry Morgan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <yaml-cpp/yaml.h>

namespace yamlrpc {

template <typename T> struct Serializer {
  using TStorage = std::decay_t<T>;

  static auto deserialize(YAML::Node Node) { return Node.as<TStorage>(); }

  static auto serialize(T const &Value) { return YAML::Node{Value}; }
};

template <> struct Serializer<void> {
  using TStorage = void;

  static auto deserialize(YAML::Node Node) {}

  static auto serialize() -> YAML::Node { return {}; }
};

namespace detail {

template <typename TArg>
auto unpackArg(YAML::iterator &YIt, [[maybe_unused]] std::tuple<TArg> *PH) {
  return std::make_tuple(Serializer<TArg>::deserialize(*YIt));
}

template <typename TArg1, typename... TArgs>
auto unpackArg(YAML::iterator &YIt,
               [[maybe_unused]] std::tuple<TArg1, TArgs...> *PH) {
  return std::tuple_cat(
      std::make_tuple(Serializer<TArg1>::deserialize(*YIt)),
      unpackArg<TArgs...>(++YIt, nullptr));
}

inline void packArg(YAML::Node &Node) {}

template <typename TArg> auto packArg(YAML::Node &Node, TArg LastArg) {
  Node.push_back(Serializer<TArg>::serialize(LastArg));
}

template <typename TArg1, typename... TArgs>
auto packArg(YAML::Node &Node, TArg1 Arg, TArgs... Args) {
  Node.push_back(Arg);
  packArg(Node, Args...);
}

} // namespace detail

class Skel {
public:
  void theMethod(std::vector<std::string> const &Vec);
  auto returningMethod(std::vector<std::string> const &Vec) -> std::string;
  auto sortVector(std::vector<std::string> const &Vec)
      -> std::vector<std::string>;
};

// -----------------------------------------------------------------------------
class CommandBase {
public:
  auto getCommandId() const -> uint32_t { return CommandId; }

private:
  friend class RpcTransport;

  void setCommandId(uint32_t CmdId) { CommandId = CmdId; }

  virtual auto invoke(YAML::Node InputNode) const -> YAML::Node = 0;

private:
  uint32_t CommandId;
};

// -----------------------------------------------------------------------------
class RpcTransport {
public:
  RpcTransport() = default;
  ~RpcTransport() = default;
  
  RpcTransport(RpcTransport &&) = delete;
  auto operator=(RpcTransport &&) = delete;

  RpcTransport(RpcTransport const &) = delete;
  auto operator=(RpcTransport const &) = delete;

public:
  void addCommand(CommandBase &Command) {
    Command.setCommandId(Commands.size());
    Commands.push_back(&Command);
  }

public:
  virtual auto isClient() const -> bool { return false; };
  virtual auto isServer() const -> bool { return false; };
  virtual auto invokeEndpoint(YAML::Node &InputNode) -> YAML::Node = 0;

protected:
  auto invokeCommand(YAML::Node InputNode) {
    // TODO: error handling
    uint32_t CmdIdx = InputNode[0].as<uint32_t>();
    auto *Command = Commands[CmdIdx];
    YAML::Node ArgsNode = InputNode[1];
    return Command->invoke(ArgsNode);
  }

protected:
  std::vector<CommandBase *> Commands;
};

// -----------------------------------------------------------------------------
class RpcObject {
public:
  RpcObject(RpcTransport &Transport) : Transport(Transport) {}

  RpcObject(RpcObject const &) = delete;
  auto operator=(RpcObject const &) = delete;

  RpcObject(RpcObject &&) = delete;
  auto operator=(RpcObject &&) = delete;
  
public:
  auto getTransport() const -> RpcTransport const & {
    return Transport;
  }
  auto getTransport() -> RpcTransport & {
    return Transport;
  }

protected:
  RpcTransport &Transport;
};

// -----------------------------------------------------------------------------
class RpcError : public std::runtime_error {
public:
  RpcError(char const* Msg) : std::runtime_error(Msg) {}
};

class InprocServer;

// -----------------------------------------------------------------------------
class InprocServer : public RpcTransport {
public:
  auto isServer() const -> bool override {
    return true;
  }

  auto invokeEndpoint(YAML::Node &InputNode) -> YAML::Node override {
    return invokeCommand(InputNode);
  }
};

// -----------------------------------------------------------------------------
class InprocClient : public RpcTransport {
public:
  InprocClient(InprocServer &Server) : Server(Server) {}

  auto isClient() const -> bool override {
    return true;
  }

  auto invokeEndpoint(YAML::Node &InputNode) -> YAML::Node override {
    return Server.invokeEndpoint(InputNode);
  }

private:
  InprocServer &Server;
};

// -----------------------------------------------------------------------------
template <typename TReturn, typename... TArgs>
class Command : public CommandBase {
public:
  using TCallback = std::function<auto(TArgs...)->TReturn>;

  // TODO: handle containers of string views?
  static_assert(!std::is_same_v<TReturn, std::string_view>,
                "string_view is not a valid return type (no storage)");

  Command(RpcObject *RpcInterface) : RpcIf(RpcInterface->getTransport()) {
    RpcIf.addCommand(*this);
  }

  auto operator()(TArgs... Args) -> TReturn {
    if (!RpcIf.isClient()) {
      throw RpcError {"Not a client object"};
    }
    
    YAML::Node RootNode{YAML::NodeType::Sequence};
    RootNode.push_back(getCommandId());

    YAML::Node ArgsNode;
    detail::packArg(ArgsNode, Args...);
    RootNode.push_back(ArgsNode);

    auto RetNode = RpcIf.invokeEndpoint(RootNode);

    return Serializer<TReturn>::deserialize(RetNode);
  }

  template <typename TThis>
  void bind(TThis &This, auto (TThis::*Function)(TArgs...)->TReturn) {
    if (!RpcIf.isServer()) {
      throw RpcError {"Not a server object"};
    }
    
    auto LambdaWrapper = [&This, Function](TArgs... Args) {
      return (This.*Function)(Args...);
    };
    Callback = LambdaWrapper;
  }

private:
  friend class RpcTransport;

  auto invoke(YAML::Node InputNode) const -> YAML::Node override {
    auto YIt = InputNode.begin();

    if constexpr (sizeof...(TArgs) == 0) {
      if constexpr (std::is_void_v<TReturn>) {
        Callback();
        return {};
      } else {
        return Serializer<TReturn>::serialize(Callback());
      }
    } else {
      auto Tuple = detail::unpackArg<TArgs...>(YIt, nullptr);

      if constexpr (std::is_void_v<TReturn>) {
        std::apply(Callback, std::move(Tuple));
        return {};
      } else {
        return Serializer<TReturn>::serialize(
            std::apply(Callback, std::move(Tuple)));
      }
    }
  }

private:
  RpcTransport &RpcIf;
  TCallback Callback;
};

} // namespace yamlrpc
