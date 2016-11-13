#ifndef MOBILEROBOTS_HPP
#define MOBILEROBOTS_HPP

#include "../robots-types.hpp"

#include <iostream>
#include <thread>
#include <atomic>

#include "Aria.h"

namespace is {
namespace robots {

using namespace std;

class MobileRobots {

public:
	MobileRobots(int argc, char** argv);
	~MobileRobots();
	bool run(int64_t period = 100,
             double x  = 0.0,
			 double y  = 0.0, 
			 double th = 0.0);
	void stop();

	bool get_odometry(Odometry& odo);
	bool wait_odometry();

	void set_property(RobotProperty& prop);

    float get_sample_rate();
private:
    ArArgumentParser parser;
    ArRobot robot;

    bool push_odometry(const Odometry& odo);
    OdometryQueue odometry;

    void set_linvel(double vel); // [mm/s]
    void set_rotvel(double vel); // [°/s]
    void set_pose(tuple<double, double, double> pose); //[mm],[mm],[°]

    void set_delay(double delay);
    atomic<bool> apply_delay;
    int64_t delay;

    void set_sample_rate(double sample_rate);
    int64_t sample_rate;

    thread robot_thread;
};

} // ::robots
} // ::is

#endif // MOBILEROBOTS_HPP