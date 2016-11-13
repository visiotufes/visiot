#include "find-entities.hpp"

namespace is {

using namespace std;

vector<entity> find_entities(string broker, string filter) {
	vector<entity> entities;
	auto channel = is::connect(broker, 5672, { "ispace", "ispace", "/" });

 	is::ServiceClient client(channel);
  	
  	set<unsigned int> recvids;
  	auto id = client.request("is.find_entities", filter);
  	recvids.insert(id);
  	auto ids = client.receive(recvids, 2000);
	
	if (ids.empty()) {
 		return entities;
 	}

    for (auto& id : ids) {
    	vector<entity> cur_entities;
    	client.consume(id, cur_entities);
    	entities.insert(entities.end(), cur_entities.begin(), cur_entities.end());
    }
	return entities;
}

} //::is