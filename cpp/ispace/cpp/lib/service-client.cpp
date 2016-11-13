#include "service-client.hpp"

namespace is {

using namespace std;
using namespace AmqpClient;

ServiceClient::ServiceClient(Channel channel, const string& exchange)
  : channel(channel), exchange(exchange), correlation_id(0) {

  // passive durable auto_delete 
  auto xtype = AmqpClient::Channel::EXCHANGE_TYPE_TOPIC;
  this->channel->DeclareExchange(exchange, xtype, false, true, false);

  // Create queue to receive rpc responses
  queue = this->channel->DeclareQueue("");
  this->channel->BindQueue(queue, exchange, queue);
  this->channel->BasicConsume(queue);
}

set<uint> ServiceClient::receive(const uint& id, const uint& timeout) {
  set<uint> ids { id };
  return receive(ids, timeout);
}

set<uint> ServiceClient::receive(const set<uint>& ids, const uint& timeout) {
  using namespace chrono;
  auto end = high_resolution_clock::now() + milliseconds(timeout);

  set<uint> received;
  
  // check if we have not received the reply already
  for (auto& id : ids) {
    auto iterator = replies.find(id);
    if (iterator != replies.end()) {
      received.insert(id);    
    }
  }
  if (received.size() == ids.size()) {
    return received; // all packets arrived, return
  }

  while (1) {  
    Envelope::ptr_t envelope;
    BasicMessage::ptr_t message; 

    do {  
      auto now = high_resolution_clock::now();
      int delta = duration_cast<milliseconds>(end-now).count();
      if (delta <= 0) {
        return received; // we are out of time already, leave!
      }

      bool newmsg = channel->BasicConsumeMessage(envelope, delta);
      if (newmsg == false) {
        return received; // timeout reached and nothing was received
      }

      message = envelope->Message(); 

    } while (message->ContentType() != "application/msgpack" ||
             message->CorrelationIdIsSet() == false);
      
    uint id = stoi(message->CorrelationId());
    replies.emplace(id, message);
    
    auto iterator = ids.find(id);
    if (iterator != ids.end()) { 
      // we were waiting for this message
      received.insert(id);
      if (received.size() == ids.size()) {
        return received; // all packets arrived, return
      }
    }

  }
}

set<uint> ServiceClient::receive(const GroupRequest& group, const uint& timeout) {
  return receive(group.ids, timeout);
}

} // ::is