#ifndef SYNC_SERVICE_UTILS_HPP
#define SYNC_SERVICE_UTILS_HPP

#include <tuple>
#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <limits>
#include <boost/circular_buffer.hpp>

#include "subscriber.hpp"
#include "service-client.hpp"
#include "../sync.hpp"

using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;

namespace is {

struct SubTimestamp {
    is::Subscriber sub;
    int64_t ts;
    float fps;

    SubTimestamp(auto channel, string entity) :
      sub(channel, entity + ".timestamp") { }

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

pair<vector<boost::circular_buffer<float>>, vector<float>>  get_samples(vector<SubTimestamp>& subscribers, int nsamples, bool& different_fps) {
	unsigned int ne = subscribers.size();
  
  vector<boost::circular_buffer<float>> ts_samples(ne);
  vector<float> delays(ne, 0.0);
  
  for (auto& d : ts_samples) { d.set_capacity(nsamples); }

  unsigned int reference = 0;
  unsigned int old_reference = 0;
  // Get samples
  for (unsigned int n = 0; n < nsamples; ++n) {
    // Consume data
    for (auto& s : subscribers) { s.consume(); } 
    // Verifies that all are the same fps
    if (n==0) {
      float first_fps = subscribers.front().fps;
      for (auto& s : subscribers) {
        auto rate = s.fps/first_fps;
        if ( rate > 1.01 || rate < 0.99) {
            different_fps = true;
            cout << "Exiting.. different fps" << endl;
            return make_pair(ts_samples,delays);
        }
      }
      different_fps = false;
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
      float dt = duration_cast<microseconds>(diff).count()/1000.0;
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
  } // End of sampling

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

  return make_pair(ts_samples, delays);
}

void set_delays(is::ServiceClient& clientDelay, vector<string> entities, vector<float> delays) {
  set<unsigned int> recvids;
  unsigned int ne = entities.size();
  for (unsigned int i = 0; i < ne; i++) {
    auto id = clientDelay.request(entities[i] + ".delay", delays[i]);
    recvids.insert(id);
  }
  auto ids = clientDelay.receive(recvids, 1000);
  if (ids.empty()) {
    cout << "Nothing was received :(" << endl;
  }
  for (auto& id : ids) {
    string reply;
    clientDelay.consume(id, reply);
    cout << "id: " << id << ", received: " << reply << endl;
  }
}

Sync sync_entities(string broker, vector<string> entities) {

  unsigned int ne = entities.size();
	// Create timestamp subscribers
  vector<SubTimestamp> subscribers;

	for (auto& e : entities) {
  	auto channel = is::connect(broker, 5672, { "ispace", "ispace", "/" }); 
  	subscribers.push_back(SubTimestamp(channel, e));
  }
   
  // Create trigger delay client
  auto channel = is::connect(broker, 5672, { "ispace", "ispace", "/" });
  is::ServiceClient clientDelay(channel);
  
  for (int t = 0; t < 3; ++t) {

    vector<float> delays(ne, 0.0);
    set_delays(clientDelay, entities, delays);

    // Discard 3 samples
    for (int i=0; i<3; i++) {
      for (auto& s : subscribers) { s.consume(); } 
    }
    
    bool different_fps;
    auto result = get_samples(subscribers, 10, different_fps);
    if (different_fps) {
      return Sync::different_fps;
    }
    auto ts_samples = result.first;
    delays = result.second;

    set_delays(clientDelay, entities, delays);
    
    // Verify sync
    
    // Discard 3 samples
    for (int i=0; i<3; i++) {
      for (auto& s : subscribers) { s.consume(); } 
    }

    result = get_samples(subscribers, 5, different_fps);
    ts_samples = result.first;
    delays = result.second;

    bool restart_sync = false;
    for (auto& d : delays) {
      if (d > 0.05*(1000.0/subscribers.front().fps) ) {
        cout << "Restart sync" << endl;
        restart_sync = true;
      }
    }
    if (!restart_sync) {
      return Sync::success;
    }
  }
  return Sync::fail;
}

} // ::is



#endif // SYNC_SERVICE_UTILS_HPP