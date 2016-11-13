#include <thread>
#include <functional>
#include "ptgrey.hpp"
#include "utils/entity.hpp"
#include "data-publisher.hpp"
#include "service-provider.hpp"

using namespace is;
using namespace is::cameras;

int main (int argc, char** argv) {

	if (argc < 4 || argc > 5) {
		std::cout << ">> Usage: ./publisher <broker-address> <camera-address> <entity-ID> <optional-fps>" << std::endl;
		exit(-1);
	}
	
	std::string broker = argv[1];
	std::string camera_ip = argv[2];
	std::string entity_id = argv[3];
	std::string fps = (argc==5) ? argv[4] : "";

	// Discovery camera
	auto camera = PointGrey::discovery(camera_ip);
	if(camera == nullptr) {
		std::cout << ">> No camera found with ip " << camera_ip << std::endl;
		exit(-1);
	}

	// Create Camera Entity
	is::entity entity {
  		"camera", 
		entity_id,
		{ "fps", "delay", "type", "resolution", "info" },
		{ "frame", "timestamp" }
	};

	// Create properties queue
	PropertyQueue queue;

	// Expose Camera Services
	auto channel = is::connect(broker, 5672, { "ispace", "ispace", "/" });
	auto name = entity.type + "." + entity.id;
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
		
		std::cout << "Setting DELAY: " << delay << std::endl;
		
		queue.push(CameraProperty(Property::delay, delay));
		std::string reply { "ok" };
		service.reply(reply);
	});
	
	// Expose IMAGE_TYPE services
	server.expose("type", [&queue] (auto& service) {
		int typeInt;
		service.request(typeInt);
		std::cout << "Setting TYPE: " << typeInt << std::endl;
		queue.push(CameraProperty(Property::image_type, static_cast<ImageType>(typeInt)));
		std::string reply { "ok" };
		service.reply(reply);
	});
	
	// Expose RESOLUTION services
	server.expose("resolution", [&queue] (auto& service) {
		std::pair<int, int> res;
		service.request(res);
		std::cout << "Setting RESOLUTION: " << res.first << 'x' << res.second << std::endl;
		queue.push(CameraProperty(Property::resolution, Resolution(res.first, res.second)));
		std::string reply { "ok" };
		service.reply(reply);
	});
	
	// Expose INFO service
	server.expose("info", [&entity] (auto& service) {
		service.reply(entity);
		std::cout << "Info request[" <<  entity.id << "]" << std::endl;
	});

	server.listen();

	// Publish frame and timestamp
	is::DataPublisher publisher(broker, name);
	std::cout << ">> Publishing " << name << std::endl;

	if (fps.empty()) {
		camera->run({
			{Property::resolution, Resolution(720,480)}
		});
	} else {
		camera->run({
			{Property::fps, stod(fps)},
			{Property::resolution, Resolution(720,480)}
		});	
	}
	
	while (1) {
		Image frame;
		camera->wait_frame();
		while(camera->get_frame(frame)) {
			// Publish frame
			publisher.publish(frame);
			// Publish timestamp
			publisher.publish(frame.timestamp, camera->get_fps());
		}
		queue.consume_all([&] (auto& prop) { 
			camera->set_property(prop);
		});
	}
	camera->stop();

	return 0;
}