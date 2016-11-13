#include <iostream>
#include <string>
#include "opencv2/highgui.hpp"
#include "cameras/cameras.hpp"

using namespace is;

int main (int argc, char *argv[]) {
	if (argc != 3) {
		std::cout << ">> Usage: ./display-single-camera <broker-address> <camera-ID>" << std::endl;
		exit(-1);
	}

	std::string broker = argv[1];
	std::string id = argv[2];

	Camera camera(broker, id);

	camera.set_fps(10.0);
	camera.set_type(ImageType::rgb);
	camera.set_resolution(720,480);

	camera.start_consume();
	while (1) {
		cv::Mat frame = camera.get_frame();
		cv::imshow("camera." + id, frame);
		cv::waitKey(1);
	}
	return 0;
}