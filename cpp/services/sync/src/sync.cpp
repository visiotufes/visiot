#include "sync.hpp"

namespace is {

using namespace std;

Sync sync(string broker, vector<entity> entities) {
	vector<string> str_entities;
	for (auto& e : entities) {
		str_entities.push_back(e.type + "." + e.id);
	}
	return sync(broker, str_entities);
}

Sync sync(string broker, vector<string> entities) {
	is::ServiceClient client(connect(broker, 5672, { "ispace", "ispace", "/" }));
	auto reqid = client.request("is.sync", entities);
	auto ids = client.receive(reqid, 60000);
	if (ids.empty()) { return Sync::timeout; }
	for (auto id : ids) {
		Sync reply;
		client.consume(id, reply);
		return reply;
	}
}

} //::is