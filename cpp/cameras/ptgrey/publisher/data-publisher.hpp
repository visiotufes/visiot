#ifndef IS_DATA_PUBLISHER_HPP
#define IS_DATA_PUBLISHER_HPP

#include <tuple>
#include "publisher.hpp"
#include "utils/image.hpp"

#include <iostream>

namespace is {

struct DataPublisher {

	is::Publisher publisher;
	static unsigned int ids;
	unsigned int id;

	DataPublisher(std::string broker, std::string entity) :
			publisher(is::connect(broker, 5672, { "ispace", "ispace", "/" }), entity) {
  		this->id = this->ids;
  		this->ids++;
	}

	void publish(cv::Mat& frame) {
		this->publisher.publish(to_image(frame), "frame");
	}

	void publish(is::Image& frame) {
		this->publisher.publish(frame, "frame");
	}

	void publish(const int64_t& timestamp, const float& fps) {
		this->publisher.publish(std::make_tuple(timestamp,fps), "timestamp");
	}

};

unsigned int DataPublisher::ids = 0;

} // ::is

#endif // IS_DATA_PUBLISHER_HPP