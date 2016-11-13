#include <thread>
#include <functional>
#include "ptgrey.hpp"
#include "utils/entity.hpp"
#include "data-publisher.hpp"
#include "service-provider.hpp"

using namespace is;
using namespace is::cameras;

void run_camera(unique_ptr<PointGrey> camera,
			   std::string broker,
			   is::entity entity,
			   std::string fps,
			   PropertyQueue& queue) {

	auto name = entity.type + "." + entity.id;

	auto channel = is::connect(broker, 5672, { "ispace", "ispace", "/" });
	is::ServiceProvider server(channel, name);

	// Expose FPS services
	server.expose("fps", [&queue] (auto& service) {
		float fps;
		service.request(fps);
		//std::cout << "Setting FPS [" <<  i << "]: " << fps << std::endl;
		queue.push(CameraProperty(Property::fps, fps));
		std::string reply { "ok" };
		service.reply(reply);
	});

	// Expose DELAY services
	server.expose("delay", [&queue] (auto& service) {
		float delay;
		service.request(delay);
		//std::cout << "Setting DELAY [" <<  i << "]: " << delay << std::endl;
		queue.push(CameraProperty(Property::delay, delay));
		std::string reply { "ok" };
		service.reply(reply);
	});
	
	// Expose IMAGE_TYPE services
	server.expose("type", [&queue] (auto& service) {
		int typeInt;
		service.request(typeInt);
		//std::cout << "Setting TYPE [" <<  i << "]: " << typeInt << std::endl;
		queue.push(CameraProperty(Property::image_type, static_cast<ImageType>(typeInt)));
		std::string reply { "ok" };
		service.reply(reply);
	});
	
	// Expose RESOLUTION services
	server.expose("resolution", [&queue] (auto& service) {
		std::pair<int, int> res;
		service.request(res);
		//std::cout << "Setting RESOLUTION [" <<  i << "]: " << res.first << 'x' << res.second << std::endl;
		queue.push(CameraProperty(Property::resolution, Resolution(res.first, res.second)));
		std::string reply { "ok" };
		service.reply(reply);
	});
	
	// Expose individual INFO services
	server.expose("info", [&entity] (auto& service) {
		service.reply(entity);
		std::cout << "Info request[" <<  entity.id << "]" << std::endl;
	});

	server.listen();		


	is::DataPublisher publisher(broker, name);
	camera->run();
	while (1) {
		Image frame;
		camera->wait_frame();
		while(camera->get_frame(frame)) {
			// Publish frame and timestamp
			publisher.publish(frame);
			publisher.publish(frame.timestamp, camera->get_fps());
		}
		queue.consume_all([&] (auto& prop) { 
			camera->set_property(prop);
		});
	}
	camera->stop();
}

int main (int argc, char** argv) {

	if (argc < 2 || argc > 3) {
		std::cout << ">> Usage: ./publisher <broker-address> <optional-fps>" << std::endl;
		return 0;
	}
	
	std::string broker = argv[1];
	std::string fps = (argc==3) ? argv[2] : "";
	
	// Discovery cameras
	auto cameras = PointGrey::discovery();
	if(cameras.empty()) {
		std::cout << ">> No camera found." << std::endl;
		exit(-1);
	}

	// Sort cameras in specific order
	PointGrey::sort_by_serial(cameras, {15358823, 15358824, 15385990, 15328550});

	unsigned int ncam = (unsigned int) cameras.size();
	// Create propreaty queues
	std::vector<PropertyQueue> propsqueue(ncam);
	
	// Create threads for each camera
	std::vector<std::thread> threads;
	for (int i = 0; i < ncam; ++i) {

		is::entity entity {
  		"camera", 
			std::to_string(i),
			{ "fps", "delay", "type", "resolution", "info" },
			{ "frame", "timestamp" }
		};
		
		std::thread thread { 
			run_camera, 
			std::move(cameras.at(i)), 
			broker, 
			entity,
			fps, 
			std::ref(propsqueue.at(i)) 
		};
  		
		threads.push_back(std::move(thread));
	}

	std::cout << ">> Publishing...\n" << std::flush;
	threads.back().join();
	return 0;
}