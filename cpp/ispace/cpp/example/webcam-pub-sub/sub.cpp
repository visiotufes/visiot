#include <string>
#include <chrono>
#include <iostream>

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"

#include "image.hpp"
#include "subscriber.hpp"

using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;

void usage() {
  cout << "sub <broker-hostname>" << endl; 
  exit(1);
}

int main(int argc, char const *argv[]) {
  if (argc != 2) {
    usage();
  } 
  
  string hostname = argv[1];

  auto info = is::discover(hostname);
  auto channel = is::connect(info, { "ispace", "ispace", "/" });  
  auto subscriber = is::Subscriber(channel, "webcam.raw");

  cout << "Listening... " << endl; 
  while (1) {
    Image image;
    subscriber.consume(image);   
    cout << "\r   \r" << subscriber.latency() << std::flush;
    cv::imshow("Webcam stream", to_mat(image));
    cv::waitKey(1); 
  }
}