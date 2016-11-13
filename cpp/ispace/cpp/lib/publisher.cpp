#include "publisher.hpp"

namespace is {

using namespace std;

Publisher::Publisher(Channel channel, const string& entity, const string& exchange)
  : channel(channel), exchange(exchange), entity(entity) {

  // passive durable auto_delete 
  auto xtype = AmqpClient::Channel::EXCHANGE_TYPE_TOPIC;
  channel->DeclareExchange(exchange, xtype, false, true, false);
}

} // ::is