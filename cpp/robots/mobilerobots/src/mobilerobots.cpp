#include "mobilerobots.hpp"

namespace is {
namespace robots {

using namespace std;
using namespace std::chrono;

MobileRobots::MobileRobots(int argc, char** argv) : parser(&argc, argv) {
    this->parser.loadDefaultArguments();
    this->apply_delay.store(false);
    Aria::init();
}

MobileRobots::~MobileRobots() {
    this->apply_delay.store(false);
    Aria::init();
}

bool MobileRobots::run( int64_t period,
                        double x,
                        double y,
                        double th ) {

    this->sample_rate = (period < 100) ? 100 : period;
    // Start robot thread    
    thread t( [&](){
        // ArRobotConnector connects to the robot, get some initial data from it such as type and name,
        // and then loads parameter files for this robot.
        ArRobotConnector robotConnector(&(this->parser), &(this->robot));
        if (!robotConnector.connectRobot()) {
            ArLog::log(ArLog::Terse, "Could not connect to the robot.");
            return false;
        }
        // Start the robot processing cycle running in the background.
        // True parameter means that if the connection is lost, then the
        // run loop ends.
        this->robot.runAsync(true);
        this->robot.lock();
        this->robot.enableMotors();
        this->robot.setVel(0);
        this->robot.setRotVel(0);
        // Set initial position
        this->robot.moveTo(ArPose(x,y,th)); //[mm],[mm],[°]
        this->robot.unlock();
        ArUtil::sleep(1000);

//TODO: creat delay variable -> sync function
        while(1) {    
            // Get timestamp
            auto ts = system_clock::now();
            // Get odometry
            Odometry odo { 
                this->robot.getX(),
                this->robot.getY(),
                this->robot.getTh(),
                ts.time_since_epoch().count()
            };
            this->push_odometry(odo);
            // Sleep
            if(!(this->apply_delay.load())) {
                this_thread::sleep_until(ts + milliseconds(this->sample_rate));
            } else {
                this->apply_delay.store(false);
                this_thread::sleep_until(ts + milliseconds(this->sample_rate+this->delay));
            }
        }
    });

    this->robot_thread = std::move(t);

    return true;
}

void MobileRobots::stop() {
    this->robot.lock();
    this->robot.disableMotors();
    this->robot.setVel(0);
    this->robot.setRotVel(0);
    this->robot.unlock();
}

bool MobileRobots::push_odometry(const Odometry& odo) {
    return this->odometry.push(odo);
}

bool MobileRobots::get_odometry(Odometry& odo) {
    bool ret_value = this->odometry.pop(odo);
    if (ret_value == false) {
        return false;
    }

    while (ret_value) {
        ret_value = this->odometry.pop(odo);
    }
    return true;
}

bool MobileRobots::wait_odometry() {
    this->odometry.wait();
}

void MobileRobots::set_property(RobotProperty& prop) {
    switch (prop.type) {
        case Property::lin_velocity:
            this->set_linvel(boost::any_cast<double>(prop.value));
            break;
        case Property::rot_velocity:
            this->set_rotvel(boost::any_cast<double>(prop.value));
            break;
        case Property::pose:
            this->set_pose(boost::any_cast<tuple<double,double,double>>(prop.value));
            break;            
        case Property::delay:
            this->set_delay(boost::any_cast<double>(prop.value));
            break;
        case Property::sample_rate:
            this->set_sample_rate(boost::any_cast<double>(prop.value));
            break;
        default:
            break;
    }
}

// Linear Velocity in [mm/s]
void MobileRobots::set_linvel(double vel) {
    this->robot.lock();
    this->robot.setVel(vel);
    this->robot.unlock();
}

// Rotational velocity in [°/s]
void MobileRobots::set_rotvel(double vel) {
    this->robot.lock();
    this->robot.setRotVel(vel);
    this->robot.unlock();
}

// Pose in <[mm],[mm],[°]>
void MobileRobots::set_pose(tuple<double, double, double> pose) {
    double x, y, th;
    x = get<0>(pose);
    y = get<1>(pose);
    th = get<2>(pose);
    this->robot.lock();
    this->robot.moveTo(ArPose(x,y,th));
    this->robot.unlock();
}

// Delay in [s]
void MobileRobots::set_delay(double delay) {
//TODO: add period verification
    int64_t d = static_cast<int64_t>(1000.0*delay);
    if (d <= this->sample_rate) {
        this->delay = d;
        this->apply_delay.store(true);
    }
}

float MobileRobots::get_sample_rate() {
    return 1000.0/static_cast<float>(this->sample_rate);
}

void MobileRobots::set_sample_rate(double sample_rate) {
    this->sample_rate = static_cast<int64_t>(1000.0/sample_rate);
}


} // ::robots
} // ::is