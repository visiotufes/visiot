#include <iostream>
#include <tuple>
#include <thread>
#include <functional>

#include "publisher.hpp"
#include "service-provider.hpp"

#include "mobilerobots.hpp"
#include "utils/entity.hpp"

using namespace std;
using namespace std::chrono;
using namespace is;
using namespace is::robots;

int main (int argc, char** argv) {

	if (argc < 5 || argc > 6) {
		std::cout << ">> Usage: ./publisher <broker-address> <entity-ID> -rp <serial-port> <optional-period>"
		          << "\n\tor ./publisher <broker-hostname> <entity-ID> -rh <ip>:<port> <optional-period>" << std::endl;
		return 0;
	}
	
	std::string broker = argv[1];
	std::string entity_id = argv[2];
	std::string period = argc == 6 ? argv[5] : "";

	// Create Robot Entity
	is::entity entity {
		"robot", 
		entity_id,
		{ "velocity", "delay", "info" },
		{ "odometry", "timestamp" }
	};
	
	// Create and Run Robot
	MobileRobots robot(argc, argv);

	// Create properties queue
	PropertyQueue queue;

	// Expose Robot Services
	auto channel = is::connect(broker, 5672, { "ispace", "ispace", "/" });
	auto name = entity.type + "." + entity.id;
	is::ServiceProvider server(channel, name);

	// Expose VELOCITY service
	server.expose("velocity", [&queue] (auto& service) {
		tuple<double,double> velocities;
		service.request(velocities);
		auto v = std::get<0>(velocities);
		auto w = std::get<1>(velocities);
		std::cout << "Setting VELOCITIES [" << v << "," << w << "]" << std::endl;
		
		queue.push(RobotProperty(Property::lin_velocity, v));
		queue.push(RobotProperty(Property::rot_velocity, w));
		
		std::string reply { "ok" };
		service.reply(reply);
	});

	// Expose POSE service
	server.expose("pose", [&queue] (auto& service) {
		tuple<double,double,double> pose;
		service.request(pose);
		auto x  = std::get<0>(pose);
		auto y  = std::get<1>(pose);
		auto th = std::get<2>(pose);
		std::cout << "Setting POSE [" << x << "," << y << "," << th << "]" << std::endl;
		
		queue.push(RobotProperty(Property::pose, pose));
		std::string reply { "ok" };
		service.reply(reply);
	});

	// Expose DELAY services
	server.expose("delay", [&queue] (auto& service) {
		double delay;
		service.request(delay);
		std::cout << "Setting DELAY " << delay << std::endl;
		
		queue.push(RobotProperty(Property::delay, delay));
		
		std::string reply { "ok" };
		service.reply(reply);
	});

	// Expose SAMPLE_RATE services
	server.expose("sample_rate", [&queue] (auto& service) {
		double sample_rate;
		service.request(sample_rate);
		std::cout << "Setting SAMPLE_RATE " << sample_rate << std::endl;
		
		queue.push(RobotProperty(Property::sample_rate, sample_rate));
		
		std::string reply { "ok" };
		service.reply(reply);
	});
	
	// Expose individual INFO services
	server.expose("info", [&entity] (auto& service) {
		service.reply(entity);
		std::cout << "Info request" << std::endl;
	});

	server.listen();

	// Publish odometry and timestamp
	is::Publisher publisher(is::connect(broker, 5672, { "ispace", "ispace", "/" }), name);
	std::cout << ">> Publishing " << name << std::endl;
	
	if (period.empty()) {
		robot.run();
	} else {
		robot.run(stoi(period));
	}

	while(1) {
		robot.wait_odometry();
		Odometry odo;
		while(robot.get_odometry(odo)) {
			// Publish odometry
			publisher.publish(odo, "odometry");
			// Publish timestamp
			publisher.publish(make_tuple(odo.timestamp, robot.get_sample_rate()), "timestamp");
		}
		queue.consume_all([&] (auto& prop) { 
			robot.set_property(prop);
		});
	}
	robot.stop();

	return 0;
}