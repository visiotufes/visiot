#ifndef __IS_SERIALIZER_HPP__
#define __IS_SERIALIZER_HPP__

#include <string>
#include <sstream>
#include <msgpack.hpp>

namespace is {

template <typename DataType>
std::string serialize(const DataType& data) {
  std::stringstream body;
  msgpack::pack(body, data);
  return body.str();
}

template <typename DataType>
bool deserialize(DataType& data, const std::string& body) {
  msgpack::object_handle handle = msgpack::unpack(body.data(), body.size());
  // object is valid during the object_handle instance is alive.
  msgpack::object object = handle.get();
  object.convert(data);
  return true;
}

}

#endif // __IS_SERIALIZER_HPP__