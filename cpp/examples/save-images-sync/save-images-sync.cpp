#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include "opencv2/imgproc.hpp"

#include "services/sync/sync.hpp"
#include "services/find-entities/find-entities.hpp"
#include "cameras/cameras.hpp"

using namespace is;

int main (int argc, char *argv[]) {

	if (argc != 4) {
		std::cout << ">> Usage: ./save-images-sync <path-folder> <number-images> <sub-sample>" << std::endl;
		exit(-1);
	}

	std::string folder = argv[1];
	int n_images = atoi(argv[2]);
	int sub_sample = atoi(argv[3]);

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
		
	std::vector<cv::Mat> images;
	int k = 0;

	cameras.start_consume();
	
	for (int n = 0; n < n_images*sub_sample; ++n) {
		auto frames = cameras.get_frame();

		for (auto& f : frames) {
			cv::resize(f, f, cv::Size(f.cols/2, f.rows/2));
		}
		cv::Mat image;
		cv::hconcat(frames, image);
		
		if (++k == sub_sample) {
			images.push_back(image.clone());
			std::cout << '[' << images.size() << '/' << n_images << ']' << std::endl;
			k = 0;
		}

		cv::imshow("cameras", image);
		cv::waitKey(1);		
	}

	std::cout << "Images captured: " << images.size() << std::endl;

	
	int w = n_images < 10    ? 1 :
			n_images < 100   ? 2 :
			n_images < 1000  ? 3 :
			n_images < 10000 ? 4 : 5;

	k = 0;
	for (auto& im : images)	{
		std::ostringstream name;
		name << folder << std::setw(w) << std::setfill('0') << k++ << ".jpg";
		cv::imwrite(name.str(), im);
	}

	return 0;
}