#ifndef __ENTITY_HPP__
#define __ENTITY_HPP__

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <msgpack.hpp>
#include "service-client.hpp"

namespace is {

using namespace std;

struct entity {
	string type;
	string id;
	vector<string> services;
	vector<string> resources;

	entity() {}
	entity(	const string& type, 
			const string& id,
			const vector<string>& services,
			const vector<string>& resources) {
		this->type = type;
		this->id = id;
		this->services = services;
		this->resources = resources;
	}

	MSGPACK_DEFINE(type, id, services, resources);
};

} // ::is

#endif // __ENTITY_HPP__