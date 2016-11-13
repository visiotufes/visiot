#ifndef SYNC_HPP
#define SYNC_HPP

#include <string>
#include <utils/entity.hpp>
#include "service-client.hpp"

namespace is {

using namespace std;

enum class Sync{
	success = 0,
	fail = 1,
	different_fps = 2,
	timeout = 3
};
// List of is::entities
Sync sync(string broker, vector<entity> entities);
    
// List of entities names i.e. { "camera.0", "robot.1" }
Sync sync(string broker, vector<string> entities);

} // ::is

MSGPACK_ADD_ENUM(is::Sync);

#endif // SYNC_HPP