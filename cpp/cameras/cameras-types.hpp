#ifndef CAMERAS_TYPES_HPP
#define CAMERAS_TYPES_HPP

#include <map>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/any.hpp>

#include "utils/image.hpp"
#include "utils/sync-queue.hpp"

namespace is {

using namespace boost;
using namespace boost::lockfree;

enum class Property {
	fps,
	image_type,
	resolution,
	delay,
	packet_delay,
	packet_size,
	num_property
};

enum class ImageType {
	rgb,
	grayscale
};

struct Resolution {
	int width;
	int height;

	Resolution() {}
	Resolution(int width, int height) {
		this->width = width;
		this->height = height;
	}
};

struct CameraProperty {
	Property type;
	boost::any value;
	
	CameraProperty() {}
	CameraProperty(Property type, boost::any value) {
		this->type = type;
		this->value = value;
	}
};

typedef spsc_queue<CameraProperty, capacity<10>> PropertyQueue;

typedef std::map<Property,boost::any> Configuration;

typedef SyncQueue<Image,10> ImageQueue;

} // ::is


#endif // CAMERAS_TYPES_HPP