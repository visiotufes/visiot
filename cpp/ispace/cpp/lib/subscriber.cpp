#include "subscriber.hpp"

namespace is {

using namespace std;
using namespace AmqpClient;

Subscriber::Subscriber(Channel channel, const string& key, const string& exchange) 
  : channel(channel), exchange(exchange), key(key), message(nullptr) {
  
  // passive durable auto_delete 
  auto xtype = AmqpClient::Channel::EXCHANGE_TYPE_TOPIC;
  channel->DeclareExchange(exchange, xtype, false, true, false);
  
  Table arguments {
    { TableKey("x-max-length"), TableValue(1) }
  };

  // passive durable exclusive auto_delete arguments
  string queue = channel->DeclareQueue("", false, false, true, true, arguments);
  channel->BindQueue(queue, exchange, key);
  channel->BasicConsume(queue);
}

unsigned int Subscriber::latency() {
  using namespace std::chrono;

  auto now = system_clock::now().time_since_epoch().count();
  auto diff = nanoseconds(now - message->Timestamp());
  return duration_cast<milliseconds>(diff).count();
}

std::string Subscriber::topic() const {
  return envelope->RoutingKey();  
}

} // ::is
