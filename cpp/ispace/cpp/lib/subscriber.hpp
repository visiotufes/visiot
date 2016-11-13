#ifndef __IS_SUBSCRIBRER_HPP__
#define __IS_SUBSCRIBRER_HPP__

#include <string>
#include <future>
#include <SimpleAmqpClient/SimpleAmqpClient.h>

#include "channel.hpp"
#include "serialize.hpp"

namespace is {

using namespace std;
using namespace AmqpClient;

class Subscriber {

  Channel channel;
  const string exchange;
  const string key;

  Envelope::ptr_t envelope;
  BasicMessage::ptr_t message; 

public:

  Subscriber() {}
  Subscriber(Channel channel, const string& key, const string& exchange = "data");
  
  unsigned int latency(); 
  std::string topic() const;

public:

  template <typename DataType>
  bool consume(DataType& data, int timeout = -1) {
    //Envelope::ptr_t envelope;
    
    do {
      bool newmsg = channel->BasicConsumeMessage(envelope, timeout);
      if (newmsg == false) {
        // timeout reached and nothing was received
        return false;
      }
      message = envelope->Message(); 
    } while (message->ContentType() != "application/msgpack" ||
             message->TimestampIsSet() == false);

    deserialize(data, message->Body());
    return true;
  }

}; // ::Subscriber

} // ::is

#endif // __IS_SUBSCRIBRER_HPP__
