#include <iostream>
#include <string>
#include "service-provider.hpp"
#include "utils/entity.hpp"
#include "sync-service-utils.hpp"

using namespace std;
using namespace is;

int main (int argc, char *argv[]) {

  if (argc != 2) {
    cout << ">> Usage: ./sync-service <broker-address>" << endl;
    exit(-1);
  }

  string broker = argv[1];
  string entity = "is.sync";

  auto channel = is::connect(broker, 5672, { "ispace", "ispace", "/" });
  is::ServiceProvider server(channel, entity);

  cout << ">> Waiting for request..." << endl;
  
  server.expose("", [broker](auto& service) {
    
    vector<string> entities;
    service.request(entities);
    cout << ">> Sync request received!\n\t>> Entity list:\n" << flush;
    for (auto& e : entities) { cout << '\t' << e << endl; }
 
    // Sync!
    auto exit_code = sync_entities(broker, entities);

    service.reply(exit_code);

    cout << ">> Sync done!" << endl;
  });

  server.listen_sync();   

  return 0;
}