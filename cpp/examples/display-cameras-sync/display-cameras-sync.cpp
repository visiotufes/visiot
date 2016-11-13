#include <iostream>
#include "opencv2/imgproc.hpp"

#include "services/sync/sync.hpp"
#include "services/find-entities/find-entities.hpp"
#include "cameras/cameras.hpp"

using namespace is;

int main (int argc, char *argv[]) {
	
	auto entities = find_entities("localhost", "camera");

	GroupCamera cameras("localhost", entities);

	cameras.set_fps(1.0);
	cameras.set_type(ImageType::rgb);
	cameras.set_resolution(640, 480);
	
	std::cout << "Waiting for sync.." << std::endl;

	auto sync_code = sync("localhost", entities);

	if (sync_code != Sync::success) {
		std::cout << "Sync failed, exiting.." << std::endl;
		exit(-1);
	}

	cameras.start_consume();
		
	while (1) {
		auto frames = cameras.get_frame();

		for (auto& f : frames) {
			cv::resize(f, f, cv::Size(f.cols/2, f.rows/2));
		}
		cv::Mat image;
		cv::hconcat(frames, image);
		
		cv::imshow("cameras", image);
		cv::waitKey(1);		
	}

	return 0;
}