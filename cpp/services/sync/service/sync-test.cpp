#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <limits>
#include <boost/circular_buffer.hpp>

#include "subscriber.hpp"
#include "service-client.hpp"

using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;

struct SubTimestamp {
    is::Subscriber sub;
    int64_t ts;
    float fps;

    SubTimestamp(auto channel, string entity) :
            sub(channel, entity + ".timestamp") {
    }

    void consume() {
        std::tuple<int64_t, float> payload;
        this->sub.consume(payload);
        this->ts = std::get<0>(payload);
        this->fps = std::get<1>(payload);
    }
};

float mean(const boost::circular_buffer<float>& b) {
  return std::accumulate(b.begin(), b.end(), 0)/b.size();
}

void usage() {
  cout << ">> Usage: ./sync-cameras <broker-address> <entity-0> ... <entity-n>" << endl; 
  exit(1);
}

int main(int argc, char const *argv[]) {
  if (argc < 3) { usage(); }
  
  string broker = argv[1];
  vector<string> entities;
  for (int i = 2; i < argc; ++i) {
    entities.push_back(argv[i]);  
  }
  unsigned int ne = entities.size();

  // Create timestamp subscribers
  std::vector<SubTimestamp> subscribers;
//TODO: testar com 1 só canal, n lembro se da certo :(
  for (auto& e : entities) {
    auto channel = is::connect(broker, 5672, { "ispace", "ispace", "/" }); 
    subscribers.push_back(SubTimestamp(channel, e));
  }
  cout << "Listening... " << endl; 
  
  // Create trigger delay client
  auto channel = is::connect(broker, 5672, { "ispace", "ispace", "/" }); 
  is::ServiceClient clientDelay(channel);

  bool first = true;
  std::vector<float> delays(ne, 0.0);
  std::vector<boost::circular_buffer<float>> ts_samples(ne);
  for (auto& d : ts_samples) { d.set_capacity(10); }
  bool fpschange = false;
  unsigned int nsamples = 10;

  // Init delays with 0.0
  std::set<unsigned int> recvids;
  for (auto& e : entities) {
    auto id = clientDelay.request(e + ".delay", 0.0);
    recvids.insert(id);
  }

  auto ids = clientDelay.receive(recvids, 1000);

  if (ids.empty()) {
    std::cout << "Nothing was received :(" << std::endl;
  }

  for (auto& id : ids) {
    std::string reply;
    clientDelay.consume(id, reply);
    std::cout << "id: " << id << ", received: " << reply << std::endl;
  }

  // Discard 3 samples
  for (int i=0; i<3; i++) {
    for (auto& s : subscribers) {
        s.consume();
    } 
  }

  unsigned int reference = 0;
  unsigned int old_reference = 0;
  
  for (unsigned int n = 0; n < nsamples; ++n) {
    // Consume data
    for (auto& s : subscribers) {
      s.consume();
    } 
    // Verifies that all are the same fps
    if (n==0) {
      float first_fps = subscribers.front().fps;
      for (auto& s : subscribers) {
        if (s.fps != first_fps) {
// TODO: trocar para código de erro fps diferente
          cout << "Exiting.. different fps" << endl;
          return -1;
        }
      }
    }

    old_reference = reference;
    // Get the latest timestamp
    int64_t max = 0;
    for (unsigned int i = 0; i < ne; i++) {
        auto s = subscribers[i];
        if (s.ts > max) {
          max = s.ts;
          reference = i;
        }
    }

    // Calculate delays between entities
    cout << "ref: " << reference << "\told_ref: " << old_reference << '\t' << flush;

    for (unsigned int i = 0; i < ne; i++) {
      auto s = subscribers[i];
      auto diff = nanoseconds(max - s.ts);
      float dt = duration_cast<milliseconds>(diff).count();
      while (dt > 1000/s.fps){
        dt -= 1000/s.fps;
      }
      if (dt > 950/s.fps) {
        dt = 1000/s.fps - dt;
      }
      
      if (reference != old_reference) {
        ts_samples[i].clear();
        n = 0;
      } 
      ts_samples[i].push_back(dt);
      
      cout << std::fixed << std::setw(6) << std::setprecision(4) << dt << '\t' << flush;
      //cout << dt << '\t' << flush;
    }
    cout << '\n' << flush;
  }

  // Calculate average delay
  for (unsigned int i = 0; i < ne; i++) {
    if (i != reference) {
      delays[i] = mean(ts_samples[i])/1000.0;
    } else {
      delays[i] = 1.0/subscribers[i].fps;
    }
    cout << std::fixed << std::setw(6) << std::setprecision(4) << delays[i] << '\t' << flush;
    //cout << delays[i] << '\t' << flush;
  }
  cout << '\n' << flush;

  // Apply delays

  recvids.clear();
  for (unsigned int i = 0; i < ne; i++) {
    auto id = clientDelay.request(entities[i] + ".delay", delays[i]);
    recvids.insert(id);
  }

  ids.clear();
  ids = clientDelay.receive(recvids, 1000);

  if (ids.empty()) {
    std::cout << "Nothing was received :(" << std::endl;
  }

  for (auto& id : ids) {
    std::string reply;
    clientDelay.consume(id, reply);
    std::cout << "id: " << id << ", received: " << reply << std::endl;
  }

/*************************************************************************************/
  // verify!

  for (unsigned int n = 0; n < 100; ++n) {
    // Consume data
    for (auto& s : subscribers) {
      s.consume();
    }

    // Get the latest timestamp
    int64_t max = 0;
    for (unsigned int i = 0; i < ne; i++) {
        auto s = subscribers[i];
        if (s.ts > max) {
          max = s.ts;
          reference = i;
        }
    }
 
    // Calculate delays between entities
    for (unsigned int i = 0; i < ne; i++) {
      auto s = subscribers[i];
      auto diff = nanoseconds(max - s.ts);
      float dt = duration_cast<milliseconds>(diff).count();
      while (dt > 1000/s.fps){
        dt -= 1000/s.fps;
      }
      if (dt > 950/s.fps) {
        dt = 1000/s.fps - dt;
      }
        ts_samples[i].push_back(dt);
        cout << dt << '\t' << flush;
    }
    cout << '\n' << flush;
  
  }

  return 0;
}

/*
    // Verify fps change
    if (first) {
      for (unsigned int i = 0; i < ne; i++) {
        oldfps[i] = subscribers[i].fps;  
      }
      first = false;
    } else {
      for (unsigned int i = 0; i < ne; i++) {
        oldfps[i] = fps[i];
        fps[i] = subscribers[i].fps;
        if (oldfps[i] != fps[i]) {
          fpschange = true;
        }
      }
    }
    //
    if (!fpschange) {
      int64_t max = 0;
      unsigned int reference = 0;
      
      for (unsigned int i = 0; i < ne; i++) {
          auto s = subscribers[i];
          if (s.ts > max) {
            max = s.ts;
            reference = i;
          }
      }
      
      for (unsigned int i = 0; i < ne; i++) {
          auto diff = nanoseconds(max - subscribers[i].ts);
          float dt = duration_cast<milliseconds>(diff).count();
          while (dt > 1000/fps[i]){
            dt -= 1000/fps[i];
          }
          if (dt > 950/fps[i]) {
            dt = 1000/fps[i] - dt;
          }
          averageDelays[i].push_back(dt);

          //std::cout << dt << '\t' << flush;
      }
      //std::cout << '\n';

      nsamples++;
      if(nsamples>=10) {
          // Print average deltaT's                  
          std::cout << "\r                        \r";
          for (auto& d : averageDelays) {
            std::cout << std::fixed << std::setw(6) << std::setprecision(3) << mean(d) << " ";
          }
          std::cout << std::flush;
                    
//  MELHORAR ESSA BOSTA DE IMPLEMENTAÇÃO NOJENTA
        if(nsamples==10) {
          // Verify if exist dt > T/2
          int indexMoreHalf = -1;
          int indexRef = -1;
          float minDiff = std::numeric_limits<float>::max();
          float mindt = std::numeric_limits<float>::max();

          for (unsigned int i = 0; i < ne; i++) {
            auto d = averageDelays[i];
            auto meand = mean(d);
            if (meand < mindt) {
              mindt = meand;
              indexRef = i;
            }
            auto diff = meand - (1000.0/fps[i])/2.0;
            if (diff > 0.0) {
              if (diff < minDiff){
                minDiff = diff;
                indexMoreHalf = i;
              }
            }
          }
          std::cout << indexMoreHalf << '\t' << minDiff << '\n' << std::flush;
          // Calc delays
          if (indexMoreHalf >= 0) {
            for (unsigned int i = 0; i < ne; i++) {
              if (i != indexMoreHalf) {
                if (i == indexRef) {
                  // reference
                  delays[i] = 1.0/fps[i]-mean(averageDelays[indexMoreHalf])/1000.0;
                } else {
                  // not reference
                  delays[i] = mean(averageDelays[i])/1000.0 + 1.0/fps[i]-mean(averageDelays[indexMoreHalf])/1000.0;
                  while (delays[i] > 1.0/fps[i]) {
                    delays[i] -= 1.0/fps[i];
                  }
                }
              } else {
                //delays[i] = 0.0; // testar esperar um frame ao inves de fazer isso
                delays[i] = 1.0/fps[i];
              }
            }
            
          } else {
            for (unsigned int i = 0; i < ne; i++) {
              if (i != indexRef) {
                delays[i] = mean(averageDelays[i])/1000.0;
              } else {
                //delays[i] = 0.0; // testar esperar um frame ao inves de fazer isso
                delays[i] = 1.0/fps[i];
              }
            }   
          }
          // Show delays
          std::cout << "delays: ";
          for (auto& d : delays) {
            std::cout << std::fixed << std::setw(6) << std::setprecision(3) << d << " ";
          }
          std::cout << '\n' <<  std::flush;
          // Send delays

          if(true) {
            std::set<unsigned int> recvids;
            for (unsigned int i = 0; i < ne; ++i) {
              auto id = clientDelay.request(entity + ".delay." + std::to_string(i), delays[i]);
              recvids.insert(id);
            }

            auto ids = clientDelay.receive(recvids, 1000);

            if (ids.empty()) {
              std::cout << "Nothing was received :(" << std::endl;
            }

            for (auto& id : ids) {
              std::string reply;
              clientDelay.consume(id, reply);
              std::cout << "id: " << id << ", received: " << reply << std::endl;
            }
          }
          
        }
      }

    } else {
      // fps changed
      cout << "FPS changed" << endl;
      subscribers.clear();
      for (unsigned int id = 0; id < ne; ++id) {
        auto channel = is::connect(info, { "ispace", "ispace", "/" }); 
        subscribers.push_back(SubTimestamp(channel, entity, id));
      }
      for (auto& d : delays) {
        d = 0.0;
      }
      fpschange = false;
      nsamples = 0;
    }
*/