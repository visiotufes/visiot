#ifndef __IS_SERVICE_PROVIDER_HPP__
#define __IS_SERVICE_PROVIDER_HPP__

#include <string>
#include <thread>
#include <cassert>
#include <atomic>
#include <unordered_map>

#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include "channel.hpp"
#include "serialize.hpp"

namespace is {

using namespace std;
using namespace AmqpClient;

class ServiceProvider {

  Channel channel;
  string service;
  string exchange;

  typedef function<void (ServiceProvider&)> handle_t;
  unordered_map<string, handle_t> map;

  BasicMessage::ptr_t message;
  Envelope::ptr_t envelope;

  atomic_bool running;
  thread listener;

public:

  ServiceProvider() {}
  ServiceProvider(Channel channel, const string& service, const string& exchange = "services");
  ~ServiceProvider();

  void expose(const string& key, handle_t handle);
  void process();

  void stop();
  void listen();
  void listen_sync();

public:

  template <typename DataType>
  void request(DataType& data) {
    deserialize(data, message->Body());
  }

  template <typename DataType>
  void reply(const DataType& data) const {
    auto payload = serialize(data);
    auto response = BasicMessage::Create(payload);
    response->ContentType("application/msgpack");
    response->CorrelationId(message->CorrelationId());
    channel->BasicPublish(exchange, message->ReplyTo(), response);
    channel->BasicAck(envelope);
  }
  
}; // ::ServiceProvider

} // ::is

#endif // __IS_SERVICE_PROVIDER_HPP__