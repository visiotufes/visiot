#include <iostream>

#include "service-client.hpp"
#include "types.hpp"

void usage() {
  std::cout << "client <broker-hostname> <message>" << std::endl; 
  exit(1);
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    usage();
  }

  std::string broker = argv[1];
  std::string message = argv[2];

  auto info = is::discover(broker);
  auto channel = is::connect(info, { "ispace", "ispace", "/" });

  // Service Client  
  is::ServiceClient client(channel);
  is::GroupRequest group(client);
  
  HelloRequestType request { message };
  auto id1 = client.request("myservice.hello", request);
  auto id2 = client.request("myservice.hello", request);
  auto ids = client.receive({id1, id2}, 1000);

  if (ids.empty()) {
    std::cout << "Nothing was received :(" << std::endl;
  }

  for (auto& id : ids) {
    HelloReplyType reply;
    client.consume(id, reply);
    std::cout << "id: " << id << ", received: " << reply.message << std::endl;
  }

  return 0;
} 