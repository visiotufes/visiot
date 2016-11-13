
#include "avahi.hpp"

namespace is {

Avahi::Avahi(const char* type) : outdated(true), queued_resolvers(0) {
  
  simple_poll = avahi_simple_poll_new();
  
  if (!simple_poll) {
    throw(std::runtime_error("Failed to allocate avahi simple poll"));
  }

  client = avahi_client_new (
    avahi_simple_poll_get(simple_poll), (AvahiClientFlags) 0, 
    client_callback, this, NULL
  );

  if (!client) {
    throw(std::runtime_error("Failed to allocate avahi client"));
  }

  browser = avahi_service_browser_new (
    client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, type, NULL, 
    (AvahiLookupFlags) 0, browse_callback, this
  );

  if (!browser) {
    throw(std::runtime_error("Failed to allocate avahi browser service"));
  }

}

Avahi::~Avahi() {
  avahi_service_browser_free(browser);
  avahi_client_free(client);
  avahi_simple_poll_free(simple_poll);
}

const std::vector<Avahi::ServiceInfo>& Avahi::discover(filter_t filt) {
  services.clear();
  filter = filt;
  avahi_simple_poll_loop(simple_poll);
  return services;
}

/* Called whenever the client or server state changes */

void Avahi::client_callback (
    AvahiClient* client, 
    AvahiClientState state, 
    void* data
  ) {

  assert(client);
  Avahi* self = (Avahi*) data;
  AvahiSimplePoll* simple_poll = self->simple_poll;

  if (state == AVAHI_CLIENT_FAILURE) {
    avahi_simple_poll_quit(simple_poll);
  }
} 


/* Called whenever a new services becomes available 
   on the LAN or is removed from the LAN */

void Avahi::browse_callback ( 
    AvahiServiceBrowser* browser, 
    AvahiIfIndex interface, 
    AvahiProtocol protocol, 
    AvahiBrowserEvent event,
    const char* name, 
    const char* type, 
    const char* domain, 
    AvahiLookupResultFlags flags, 
    void* data
  ) {

  assert(browser);

  Avahi* self = (Avahi*) data;
  AvahiClient* client = self->client;
  AvahiSimplePoll* simple_poll = self->simple_poll;
  
  self->outdated = true;

  switch (event) {
    case AVAHI_BROWSER_NEW:
      avahi_service_resolver_new (
        client, interface, protocol, name, type, domain, 
        AVAHI_PROTO_UNSPEC, (AvahiLookupFlags) 0, resolve_callback, data
      );
      self->queued_resolvers += 1;
      break;

    case AVAHI_BROWSER_REMOVE: 
      break;
      
    case AVAHI_BROWSER_ALL_FOR_NOW:
    case AVAHI_BROWSER_CACHE_EXHAUSTED:
      self->outdated = false;
      break;

    case AVAHI_BROWSER_FAILURE: 
      throw(std::runtime_error("Avahi browser failure"));
      break;
      
    default:
      break;
  }
}

/* Called whenever a service has been resolved 
   successfully or timed out */

void Avahi::resolve_callback (
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
  ) {

  assert(resolver);

  Avahi* self = (Avahi*) data;
  AvahiSimplePoll* simple_poll = self->simple_poll;
  
  switch (event) {
    case AVAHI_RESOLVER_FOUND: { 
      if (address->proto == AVAHI_PROTO_INET6) break;

      char ip[AVAHI_ADDRESS_STR_MAX];
      avahi_address_snprint(ip, sizeof(ip), address);

      Avahi::ServiceInfo service { name, type, ip, port };
      if (self->filter(service)) {
        self->services.emplace_back(service);
      }
    }

    case AVAHI_RESOLVER_FAILURE: 
    default: 
      break;
  }

  avahi_service_resolver_free(resolver);
  self->queued_resolvers -= 1;

  if (self->outdated == false && self->queued_resolvers == 0) {
    avahi_simple_poll_quit(simple_poll);
  }
}

} // ::is
