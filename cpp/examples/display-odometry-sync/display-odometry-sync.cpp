#include <iostream>

#include "services/sync/sync.hpp"
#include "services/find-entities/find-entities.hpp"
#include "robots/robots.hpp"

using namespace is;
using namespace std;

int main(int argc, char const *argv[]) {

  auto entities = find_entities("localhost", "robot");

  GroupRobot robots("localhost", entities);

  robots.set_sample_rate(1.0);

  std::cout << "Waiting for sync.." << std::endl;

  auto sync_code = sync("localhost", entities);
  
  robots.set_pose({ make_tuple(   0,    0,  0),
                    make_tuple(1000, 1000, 45) });

  robots.set_velocity({ make_tuple( 100, 5.0),
                        make_tuple(-100,-5.0) });

  robots.start_consume();

  while (1) {
    auto odometry = robots.get_odometry();
    for (int i=0; i < odometry.size(); ++i) {
      cout << "Robot_" << i 
           << "(" << odometry[i].x
           << "," << odometry[i].y
           << "," << odometry[i].th << ")\t" << flush;
    }
    cout << endl;
  }
  return 0;
}