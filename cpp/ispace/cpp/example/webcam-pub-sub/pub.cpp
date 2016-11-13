#include <string>
#include <chrono>
#include <iostream>

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"

#include "image.hpp"
#include "publisher.hpp"

using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;

void usage() {
  cout << "pub <broker-hostname>" << endl; 
  exit(1);
}

int main(int argc, char const *argv[]) {
  if (argc != 2) {
    usage();
  }

  string hostname = argv[1];

  auto info = is::discover(hostname);
  auto channel = is::connect(info, { "ispace", "ispace", "/" });
  auto publisher = is::Publisher(channel, "webcam");

  auto capture = cv::VideoCapture(0); // open the default camera
  assert(capture.isOpened());

  cout << "Publishing... " << endl; 
  while (1) {
    auto now = system_clock::now();
    cv::Mat frame;
    capture >> frame;
    publisher.publish(to_image(frame), "raw"); 
    this_thread::sleep_until(now + 33ms);
  }
}