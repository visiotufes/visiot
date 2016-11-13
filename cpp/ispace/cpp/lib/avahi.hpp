#ifndef __IS_AVAHI_DISCOVERY_HPP__
#define __IS_AVAHI_DISCOVERY_HPP__

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>

#include <cstdint>
#include <vector>
#include <assert.h>
#include <stdexcept>
#include <functional>

namespace is {

class Avahi {
  
  AvahiClient* client;
  AvahiServiceBrowser* browser;
  AvahiSimplePoll* simple_poll;
  
  bool outdated;
  unsigned int queued_resolvers;

public:

  struct ServiceInfo {
    std::string name;
    std::string type;
    std::string ip;
    uint16_t port;
  };

private:

  typedef std::function<bool (const ServiceInfo&)> filter_t;
  filter_t filter;
  std::vector<ServiceInfo> services;

public:

  Avahi(const char* type);
  ~Avahi();
  
  /* Discover the specified service type on the network,
     a filter can be specified.
   */
  const std::vector<ServiceInfo>& discover (
    filter_t filt = [] (const ServiceInfo&) { return true; }
  );

private:

  /* Called whenever the client or server state changes */
  static void client_callback (
    AvahiClient* client, 
    AvahiClientState state, 
    void* data
  );

  /* Called whenever a new services becomes available 
     on the LAN or is removed from the LAN */
  static void browse_callback (
    AvahiServiceBrowser* browser, 
    AvahiIfIndex interface, 
    AvahiProtocol protocol, 
    AvahiBrowserEvent event,
    const char* name, 
    const char* type, 
    const char* domain, 
    AvahiLookupResultFlags flags, 
    void* data
  );

  /* Called whenever a service has been resolved 
     successfully or timed out */
  static void resolve_callback (
    AvahiServiceResolver* resolver, 
    AvahiIfIndex interface,
    AvahiProtocol protocol, 
    AvahiResolverEvent event,
    const char* name, 
    const char* type, 
    const char* domain, 
    const char* host,
    const AvahiAddress* address, 
    uint16_t port, 
    AvahiStringList* txt, 
    AvahiLookupResultFlags flags, 
    void* data
  );

};

} // ::is

#endif // __IS_AVAHI_DISCOVERY_HPP__