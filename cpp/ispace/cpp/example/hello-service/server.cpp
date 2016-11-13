#include <iostream>

#include "service-provider.hpp"
#include "types.hpp"

void hello(is::ServiceProvider& service) {
  HelloRequestType request;
  service.request(request);

  HelloRequestType reply { "Hello " + request.name };
  service.reply(reply);
}

void usage() {
  std::cout << "server <broker-hostname>" << std::endl; 
  exit(1);
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    usage();
  }

  std::string broker = argv[1];

  auto info = is::discover(broker);
  auto channel = is::connect(info, { "ispace", "ispace", "/" });

  // Service Provider  
  is::ServiceProvider server(channel, "myservice");
  server.expose("hello", hello);
  server.listen_sync();

  return 0;
} 