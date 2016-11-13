#include <iostream>
#include "robots/robots.hpp"

using namespace is;

int main(int argc, char const *argv[]) {
  if (argc != 3) {
    std::cout << ">> Usage: ./display-single-odometry <broker-address> <robot-ID>" << std::endl;
    exit(-1);
  }

  std::string broker = argv[1];
  std::string id = argv[2];

  Robot robot(broker, id);

  robot.set_sample_rate(10.0);
  robot.set_pose(make_tuple(0, 0, 0));
  robot.set_velocity(make_tuple(100.0, 0.0));

  robot.start_consume();
  while (1) {
    auto odometry = robot.get_odometry();
    std::cout << "(" << odometry.x
              << "," << odometry.y
              << "," << odometry.th << ")\n" << std::flush;
  }

  return 0;
}