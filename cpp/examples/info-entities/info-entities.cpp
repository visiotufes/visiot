#include <iostream>
#include "service-client.hpp"
#include "services/find-entities/find-entities.hpp"

void usage() {
  std::cout << ">> Usage: ./client-info <broker-address> <entity-name>" << std::endl;
  exit(1);
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    usage();
  }

  std::string broker = argv[1];
  std::string entity = argv[2];

  auto entities = is::find_entities(broker, entity);

  for(auto& e : entities) {
      std::cout << ">> Entity:" << std::endl;
      std::cout << '\t' << e.type << "." << e.id << std::endl;
      std::cout << ">> Services:" << std::endl;
      for (auto& s : e.services) {
        std::cout << '\t' << s << std::endl;
      }
      std::cout << ">> Resources:" << std::endl;
      for (auto& r : e.resources) {
        std::cout << '\t' << r << std::endl;
      }
      std::cout << "__________________" << std::endl;
  }

  return 0;
} 
