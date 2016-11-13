#ifndef PTGREY_HPP
#define PTGREY_HPP

#include "ptgrey-utils.hpp"
#include "../cameras-types.hpp"

#include "FlyCapture2.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <thread>
#include <algorithm>
#include <memory>
#include <tuple>
#include <stdexcept>

namespace is {
namespace cameras {

using namespace std;

class PointGrey {

public:
	PointGrey(FlyCapture2::PGRGuid* handle);
	~PointGrey();

	static vector<unique_ptr<PointGrey>> discovery();
	static unique_ptr<PointGrey> discovery(const std::string& ip_address);
	
	void run();
	void run(const Configuration& conf);
	void stop();
	void set_property(CameraProperty& prop);

	void wait_frame();
	bool get_frame(Image& image);
	bool get_frame(cv::Mat& img);
	bool get_frame(cv::Mat& img, int64_t& ts);
	
	float get_fps();

	void print_camera_info();
	unsigned int get_serial_number();

	static bool sort_by_serial(vector<unique_ptr<PointGrey>>& cameras,
				 			   const vector<unsigned int>& serials);

private:
	FlyCapture2::GigECamera	camera;
	FlyCapture2::PGRGuid* 	handle;
	ImageQueue		 		frames;
	float 					current_fps;
	float 					max_fps;
	float 					max_trigger_delay;
	unsigned int 			max_width;
	unsigned int 			max_height;
	unsigned int 			hoffset;
	unsigned int 			voffset;
	unsigned int 			hstepsize;
	unsigned int 			vstepsize;
	atomic<ImageType> 		imtype;
	bool 					running, first_run;
	unsigned int 			serial_number;
	
	bool 					connect();
	bool 					disconnect();
	static void 			on_image_grabbed(FlyCapture2::Image* pImage, const void* pCallbackData);
	bool 					push_frame(const Image& img);
	
	bool 					set_fps(float fps);
	float 					get_max_fps();
	bool 					query_max_fps();
	bool 					query_max_trigger_delay();
	
	bool 					set_trigger_delay(float delay);
	float 					get_max_trigger_delay();
	
	bool 					query_max_resolution();
	bool 					set_resolution(const Resolution& res);
	bool 					set_roi(int& x, int& y, int& width, int& height);
	bool 					validate_roi(int& x, int& y, int& width, int& height);
	void 					center_roi(int& x, int& y, int& width, int& height);
	
	ImageType 				get_image_type();
	void 					set_image_type(const ImageType& type);
	
	bool 					set_packet_delay(unsigned int delay);
	bool 					set_packet_size(unsigned int size);
	
	bool 					query_serial_number();
	void 					init_configuration(const Configuration& conf);

	const boost::any		default_fps          = 1.0;
	const boost::any		default_image_type   = ImageType::rgb;
	const boost::any		default_resolution   = Resolution(640,480);
	const boost::any		default_packet_delay = 6000;
	const boost::any		default_packet_size  = 1400;

};

} // ::cameras
} // ::is

#endif // PTGREY_HPP