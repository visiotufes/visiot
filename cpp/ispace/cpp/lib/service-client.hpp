#ifndef __IS_CLIENT_HPP__
#define __IS_CLIENT_HPP__

#include <set>
#include <map>
#include <string>
#include <chrono>
#include <algorithm>
#include <SimpleAmqpClient/SimpleAmqpClient.h>

#include "channel.hpp"
#include "serialize.hpp"

namespace is {

using namespace std;
using namespace AmqpClient;

typedef unsigned int uint;

struct GroupRequest;

class ServiceClient {
  
  Channel channel;
  string exchange;
  string queue;

  uint correlation_id;
  unordered_multimap<uint, BasicMessage::ptr_t> replies;

public:

  ServiceClient(Channel channel, const string& exchange = "services");

  set<uint> receive(const uint& id, const uint& timeout);
  set<uint> receive(const set<uint>& ids, const uint& timeout);
  set<uint> receive(const GroupRequest& group, const uint& timeout);

  template <typename DataType>
  bool consume(const uint& id, DataType& data) {
    auto iterator = replies.find(id);
    if (iterator == replies.end()) {
      return false; // not found
    }

    deserialize(data, iterator->second->Body());
    replies.erase(iterator); // delete message
    return true;
  }

  template <typename DataType>
  uint request(const string& service, const DataType& data) {  
    uint id = correlation_id;
    ++correlation_id;

    auto request = BasicMessage::Create(serialize(data));
    request->ReplyTo(queue);
    request->CorrelationId(to_string(id)); 
    request->ContentType("application/msgpack");
    request->DeliveryMode(BasicMessage::dm_persistent);
    channel->BasicPublish(exchange, service, request);
    
    return id;
  }

};

struct GroupRequest {

  ServiceClient& client;
  set<uint> ids;

  GroupRequest(ServiceClient& client) : client(client) { }

  template <typename DataType>
  void request(const string& service, const DataType& data) {  
    ids.insert(client.request(service, data));
  }

};

} // ::is

#endif // __IS_CLIENT_HPP__