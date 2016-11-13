#ifndef __IS_PUBLISHER_HPP__
#define __IS_PUBLISHER_HPP__

#include <string>
#include <thread>
#include <SimpleAmqpClient/SimpleAmqpClient.h>

#include "channel.hpp"
#include "serialize.hpp"

namespace is {

using namespace AmqpClient;
using namespace std;

class Publisher {

  Channel channel;
  const string exchange;
  const string entity;
  
public:

  Publisher(Channel channel, const string& entity, const string& exchange = "data");
  
  template <typename DataType>
  void publish(const DataType& data, const string& topic) {
    auto timestamp = chrono::system_clock::now().time_since_epoch().count();
    auto payload = serialize(data);
    auto message = BasicMessage::Create(payload);
    message->ContentType("application/msgpack");
    message->Timestamp(timestamp); 
    channel->BasicPublish(exchange, entity + '.' + topic, message);
  }

}; // ::Publisher

} // ::is

#endif // __IS_PUBLISHER_HPP__