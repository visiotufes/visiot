#ifndef ODOMETRY_HPP
#define ODOMETRY_HPP

#include <msgpack.hpp>

namespace is {

struct Odometry {
	double x, y, th;
  	int64_t timestamp;

	Odometry() {}
	Odometry(const double x,  const double y, 
			 const double th, const int64_t timestamp) {
		this->x  = x;
		this->y  = y;
		this->th = th;
		this->timestamp = timestamp;
	}

	MSGPACK_DEFINE(x, y, th, timestamp);
};

} // ::is

#endif // ODOMETRY_HPP