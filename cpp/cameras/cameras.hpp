#ifndef CAMERAS_HPP
#define CAMERAS_HPP

#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "subscriber.hpp"
#include "service-client.hpp"
#include "utils/entity.hpp"
#include "utils/image.hpp"
#include "cameras-types.hpp"

namespace is {

using namespace std;

class Camera {

public:
	Camera(const string& broker, const string& id);
	Camera(const string& broker, const is::entity& entity);
	void start_consume();
	cv::Mat get_frame();
	bool set_fps(float fps);
	bool set_type(ImageType type);
	bool set_resolution(int width, int height);

private:
	string broker;
	string id;
	std::unique_ptr<Subscriber> subscriber;
	ServiceClient client;
};

Camera::Camera(const string& broker, const string& id) :
	client(connect(broker, 5672, {"ispace", "ispace", "/"})) { 
	this->id = id;
	this->broker = broker;
}

Camera::Camera(const string& broker, const is::entity& entity) : 
	Camera(broker, entity.id) { }

void Camera::start_consume() {
	this->subscriber = std::make_unique<Subscriber>(connect(this->broker, 5672, {"ispace", "ispace", "/"}), "camera." + this->id + ".frame");
}

cv::Mat Camera::get_frame() {
	is::Image i;
	(this->subscriber)->consume(i);
	return to_mat(i);
}

bool Camera::set_fps(float fps) {
	auto reqid = this->client.request("camera." + this->id + ".fps", fps);
	auto ids = this->client.receive(reqid, 1000);
	if (ids.empty()) { return false;}
 	//TODO: add return code erros
 	for (auto& id : ids) {
    	std::string reply;
    	client.consume(id, reply);
  	}
  	return true;
}

bool Camera::set_type(ImageType type) {
	auto reqid = this->client.request("camera." + this->id + ".type", static_cast<int>(type));
	auto ids = this->client.receive(reqid, 1000);
	if (ids.empty()) { return false;}
 	//TODO: add return code erros
 	for (auto& id : ids) {
    	std::string reply;
    	client.consume(id, reply);
  	}
  	return true;
}

bool Camera::set_resolution(int width, int height) {
	auto reqid = this->client.request("camera." + this->id + ".resolution", std::make_pair(width, height));
	auto ids = this->client.receive(reqid, 1000);
	if (ids.empty()) { return false;}
 	//TODO: add return code erros
 	for (auto& id : ids) {
    	std::string reply;
    	client.consume(id, reply);
  	}
  	return true;	
}

class GroupCamera {

public:
	GroupCamera(const string& broker, const vector<string>& ids);
	GroupCamera(const string& broker, const vector<entity>& entities);
	void start_consume();
	vector<cv::Mat> get_frame();
	vector<bool> set_fps(float fps);
	vector<bool> set_type(ImageType type);
	vector<bool> set_resolution(int width, int height);
private:
	vector<Camera> cameras;
};

GroupCamera::GroupCamera(const string& broker, const vector<string>& ids) {
	for (auto& id : ids) {
		this->cameras.emplace_back(Camera(broker, id));
	}
}

GroupCamera::GroupCamera(const string& broker, const vector<entity>& entities) {
	for (auto& e : entities) {
		this->cameras.emplace_back(Camera(broker, e));
	}
}

void GroupCamera::start_consume() {
	for (auto& camera : this->cameras) {
		camera.start_consume();
	}
}

vector<cv::Mat> GroupCamera::get_frame() {
	vector<cv::Mat> frames;
	for (auto& camera : this->cameras) {
		frames.push_back(camera.get_frame());
	}
	return frames;
}
vector<bool> GroupCamera::set_fps(float fps) {
	vector<bool> result;
	for (auto& camera : cameras) {
		result.push_back(camera.set_fps(fps));
	}
	return result;
}

vector<bool> GroupCamera::set_type(ImageType type) {
	vector<bool> result;
	for (auto& camera : cameras) {
		result.push_back(camera.set_type(type));
	}
	return result;
}

vector<bool> GroupCamera::set_resolution(int width, int height) {
	vector<bool> result;
	for (auto& camera : cameras) {
		result.push_back(camera.set_resolution(width, height));
	}
	return result;	
}

} // ::is

#endif // CAMERAS_HPP