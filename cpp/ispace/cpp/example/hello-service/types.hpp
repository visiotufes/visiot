#ifndef __HELLO_SERVICE_TYPES_HPP__
#define __HELLO_SERVICE_TYPES_HPP__

#include <string>
#include <msgpack.hpp>

struct HelloRequestType {
  std::string name;
  MSGPACK_DEFINE(name);
};

struct HelloReplyType {
  std::string message;
  MSGPACK_DEFINE(message);
};

#endif