#ifndef __IS_CHANNEL_HPP__
#define __IS_CHANNEL_HPP__

#include <cassert>
#include <string>
#include <cstdint>
#include <SimpleAmqpClient/SimpleAmqpClient.h>

#include "avahi.hpp"

namespace is {

using namespace std;

typedef AmqpClient::Channel::ptr_t Channel;

struct Credentials {
  string username;
  string password;
  string virtual_host;
};

/* Discover broker running on host with specified hostname using Avahi mDNS*/
Avahi::ServiceInfo discover(const string& hostname, const uint16_t& port = 5672);

/* Connect to broker using ip and port */
Channel connect(const string& ip, const uint16_t& port, const Credentials& credentials);

/* Connect to broker using the Avahi::ServiceInfo info */
Channel connect(const Avahi::ServiceInfo& service, const Credentials& credentials);

} // ::is

#endif // __IS_CHANNEL_HPP__