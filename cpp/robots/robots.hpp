#ifndef ROBOTS_HPP
#define ROBOTS_HPP

#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include "subscriber.hpp"
#include "service-client.hpp"
#include "utils/entity.hpp"
#include "robots-types.hpp"

namespace is {

using namespace std;

class Robot {

public:
	Robot(const string& broker, const string& id);
	Robot(const string& broker, const is::entity& entity);
	void start_consume();
	Odometry get_odometry();
	bool set_velocity(tuple<double,double> velocitites); // <[mm/s],[°/s]>
	bool set_pose(tuple<double,double,double> pose); //[mm],[mm],[°]
	bool set_sample_rate(double sample_rate); // [Hz]
private:
	string broker;
	string id;
	std::unique_ptr<Subscriber> subscriber;
	ServiceClient client;
};

Robot::Robot(const string& broker, const string& id) :
	client(connect(broker, 5672, {"ispace", "ispace", "/"})) { 
	this->id = id;
	this->broker = broker;
}

Robot::Robot(const string& broker, const is::entity& entity) :
	Robot(broker, entity.id) { }

void Robot::start_consume() {
	this->subscriber = std::make_unique<Subscriber>(connect(this->broker, 5672, {"ispace", "ispace", "/"}), "robot." + this->id + ".odometry");
}

Odometry Robot::get_odometry() {
	Odometry odo;
	(this->subscriber)->consume(odo);
	return odo;
}

bool Robot::set_velocity(tuple<double,double> velocitites) {
	auto reqid = this->client.request("robot." + this->id + ".velocity", velocitites);
	auto ids = this->client.receive(reqid, 1000);
	if (ids.empty()) { return false;}
 	//TODO: add return code erros
 	for (auto& id : ids) {
    	std::string reply;
    	client.consume(id, reply);
  	}
  	return true;
}

bool Robot::set_pose(tuple<double,double,double> pose) {
	auto reqid = this->client.request("robot." + this->id + ".pose", pose);
	auto ids = this->client.receive(reqid, 1000);
	if (ids.empty()) { return false;}
 	//TODO: add return code erros
 	for (auto& id : ids) {
    	std::string reply;
    	client.consume(id, reply);
  	}
	return true;
}

bool Robot::set_sample_rate(double sample_rate) {
	auto reqid = this->client.request("robot." + this->id + ".sample_rate", sample_rate);
	auto ids = this->client.receive(reqid, 1000);
	if (ids.empty()) { return false;}
 	//TODO: add return code erros
 	for (auto& id : ids) {
    	std::string reply;
    	client.consume(id, reply);
  	}
  	return true;	
}

class GroupRobot {

public:
	GroupRobot(const string& broker, const vector<string>& ids);
	GroupRobot(const string& broker, const vector<entity>& entities);
	void start_consume();
	vector<Odometry> get_odometry();
	vector<bool> set_velocity(vector<tuple<double,double>> velocitites);
	vector<bool> set_pose(vector<tuple<double,double,double>> poses);
	vector<bool> set_sample_rate(double sample_rate);
private:
	vector<Robot> robots;
};

GroupRobot::GroupRobot(const string& broker, const vector<string>& ids) {
	for (auto& id : ids) {
		this->robots.emplace_back(Robot(broker, id));
	}
}

GroupRobot::GroupRobot(const string& broker, const vector<entity>& entities) {
	for (auto& e : entities) {
		this->robots.emplace_back(Robot(broker, e));
	}
}

void GroupRobot::start_consume() {
	for (auto& robot : robots) {
		robot.start_consume();
	}
}

vector<Odometry> GroupRobot::get_odometry() {
	vector<Odometry> odometry;
	for (auto& robot : this->robots) {
		odometry.push_back(robot.get_odometry());
	}
	return odometry;
}

vector<bool> GroupRobot::set_velocity(vector<tuple<double,double>> velocitites) {
	vector<bool> result;
	if (this->robots.size() == velocitites.size()) {
		unsigned int nr = this->robots.size();
		for (int i = 0; i < nr; ++i) {
			result.push_back(this->robots[i].set_velocity(velocitites[i]));
		}
	} else {
		result.resize(this->robots.size(), false);
	}	
	return result;
}

vector<bool> GroupRobot::set_pose(vector<tuple<double,double,double>> poses) {
	vector<bool> result;
	if (this->robots.size() == poses.size()) {
		unsigned int nr = this->robots.size();
		for (int i = 0; i < nr; ++i) {
			result.push_back(this->robots[i].set_pose(poses[i]));
		}
	} else {
		result.resize(this->robots.size(), false);
	}	
	return result;
}

vector<bool> GroupRobot::set_sample_rate(double sample_rate) {
	vector<bool> result;
	for (auto& robot : robots) {
		result.push_back(robot.set_sample_rate(sample_rate));
	}
	return result;	
}

} // ::is

#endif // ROBOTS_HPP