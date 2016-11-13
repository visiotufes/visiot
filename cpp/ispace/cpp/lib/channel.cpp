#include "channel.hpp"

namespace is {

using namespace std;

/* Discover broker running on host with specified hostname using Avahi mDNS*/
Avahi::ServiceInfo discover (const string& hostname, const uint16_t& port) {
  Avahi avahi("_rabbitmq._tcp");

  auto services = avahi.discover([&] (const Avahi::ServiceInfo& service) {
    return (service.port == port && service.name == hostname);
  });

  assert(services.size());
  return services.front();
}

/* Connect to broker using the Avahi::ServiceInfo */
Channel connect(const string& ip, const uint16_t& port, const Credentials& credentials) {
  return AmqpClient::Channel::Create (
    ip, port, credentials.username, credentials.password, credentials.virtual_host
  );
}

/* Connect to broker using the Avahi::ServiceInfo */
Channel connect(const Avahi::ServiceInfo& service, const Credentials& credentials) {
  return connect(service.ip, service.port, credentials);
}

} // ::is