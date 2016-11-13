#ifndef ROBOTS_TYPES_HPP
#define ROBOTS_TYPES_HPP

#include <boost/lockfree/spsc_queue.hpp>
#include <boost/any.hpp>

#include "utils/sync-queue.hpp"
#include "utils/odometry.hpp"

namespace is {

using namespace boost;
using namespace boost::lockfree;

enum class Property {
	lin_velocity,
	rot_velocity,
	pose,
	delay,
	sample_rate
};

struct RobotProperty {
	Property type;
	boost::any value;

	RobotProperty() {}
	RobotProperty(Property type, boost::any value) {
		this->type = type;
		this->value = value;
	}
};

typedef spsc_queue<RobotProperty, capacity<10>> PropertyQueue;

typedef SyncQueue<Odometry, 10> OdometryQueue;

} // ::is

#endif // ROBOTS_TYPES_HPP